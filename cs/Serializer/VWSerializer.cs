using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Serializer
{
    using System.Collections;
    using System.Diagnostics;
    using System.Globalization;
    using System.Linq.Expressions;
    using System.Reflection.Emit;
    using System.Security;
    using System.Security.Permissions;
    using VowpalWabbit.Serializer.Attributes;
    using VowpalWabbit.Serializer.Interfaces;
    using VowpalWabbit.Serializer.Intermediate;
    using VowpalWabbit.Serializer.Reflection;
    using VwHandle = IntPtr;

    public static class VWSerializer
    {
        public static Action<TContext, TVisitor> CreateSerializer<TContext, TVisitor>()
            where TVisitor : IVowpalWabbitVisitor
        {
            // Create dynamic assembly
            var asmName = new AssemblyName("VowpalWabbitSerializer." + typeof(TContext).Name);
            var dynAsm = AppDomain.CurrentDomain.DefineDynamicAssembly(asmName, AssemblyBuilderAccess.RunAndSave);

            // Create a dynamic module and type
            var dynMod = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule");

            return CreateSerializer<TContext, TVisitor>(dynMod);
        }

        private static Action<TContext, TVisitor> CreateSerializer<TContext, TVisitor>(ModuleBuilder moduleBuilder)
            where TVisitor : IVowpalWabbitVisitor
        {            
            var valueType = typeof(TContext);
            var valueParameter = Expression.Parameter(valueType, "value");
            var visitorParameter = Expression.Parameter(typeof(TVisitor), "visitor");

            // find all features and group by namespace
            var featuresByNamespace = ExtractFeaturesCompiled(valueParameter, null, null)
                    .GroupBy(f => new { f.Namespace, f.FeatureGroup, f.IsDense }, f => f);

            var visitation = new List<Expression>();
            var variables = new List<ParameterExpression>();
            var namespaceVariables = new List<ParameterExpression>();
            var visitationNamespace = new List<Expression>();

            foreach (var ns in featuresByNamespace)
	        {
                var features = ns.ToList();

                var baseNamespaceType = typeof(Namespace);
                var baseNamespaceInits = new [] {
                    Expression.Bind(baseNamespaceType.GetProperty("Name"), Expression.Constant(ns.Key.Namespace)),
                    Expression.Bind(baseNamespaceType.GetProperty("FeatureGroup"), Expression.Constant(ns.Key.FeatureGroup)),
                };

		        if (ns.Key.IsDense)
                {
                    // Dense namespace
                    if (features.Count != 1)
                    {
                        throw new NotSupportedException("Only a single dense vector is supported per namespace");
                    }

                    var feature = features[0];
                    var namespaceType = typeof(NamespaceDense<>).MakeGenericType(feature.DenseFeatureValueElementType);

                    var namespaceDense = Expression.MemberInit(
                        Expression.New(namespaceType),
                        baseNamespaceInits.Union(new[] { 
                            Expression.Bind(namespaceType.GetProperty("DenseFeature"), feature.NewFeatureExpression)
                        }));

                    var namespaceVariable = Expression.Variable(namespaceType);
                    namespaceVariables.Add(namespaceVariable);

                    // CODE namespace = new Namespace<float> { ... };
                    visitation.Add(Expression.Assign(namespaceVariable, namespaceDense));

                    // CODE visitor.Visit(namespace);
                    visitationNamespace.Add(Expression.Call(
                        visitorParameter,
                        ReflectionHelper.FindMethod(typeof(TVisitor), "Visit", namespaceType),
                        namespaceVariable));
                }
                else 
                {
                    // Sparse namespace
                    var featureVariables = new List<ParameterExpression>();

                    foreach (var feature in ns)
	                {
                        var featureVariable = Expression.Parameter(feature.FeatureType, feature.Name);

		                variables.Add(featureVariable);
                        featureVariables.Add(featureVariable);

                        // CODE feature = new Feature<float> { ... };
                        visitation.Add(Expression.Assign(featureVariable, feature.NewFeatureExpression));
	                }

                    // CODE new NamespaceSparse { Features = new[] { feature1, feature2, ... } }
                    var namespaceSparse = Expression.MemberInit(
                        Expression.New(typeof(NamespaceSparse)),
                        baseNamespaceInits.Union(new[] { 
                            Expression.Bind(
                                typeof(NamespaceSparse).GetProperty("Features"),
                                Expression.NewArrayInit(typeof(Feature), featureVariables))
                        }));

                    var namespaceVariable = Expression.Variable(typeof(NamespaceSparse));
                    namespaceVariables.Add(namespaceVariable);

                    visitation.Add(Expression.Assign(namespaceVariable, namespaceSparse));

                    // loop unrolling to have dispatch onto the correct Visit<T>
                    var featureVisitation = new List<Expression>();
                    foreach (var feature in featureVariables)
                    {
                        // CODE: visitor.Visit(feature1); 
                        Expression visitFeatureCall = Expression.Call(
                                    visitorParameter,
                                    ReflectionHelper.FindMethod(typeof(TVisitor), "Visit", feature.Type),
                                    feature);
                        
                        var featureValue = Expression.Property(feature, "Value");
                        if (!featureValue.Type.IsValueType || (featureValue.Type.IsGenericType && featureValue.Type.GetGenericTypeDefinition() == typeof(Nullable<>)))
                        {
                            // CODE: if(feature1.Value != null) visitor.Visit(feature1);
                            visitFeatureCall = Expression.IfThen(
                                Expression.NotEqual(featureValue, Expression.Constant(null)),
                                visitFeatureCall);
                        }

                        featureVisitation.Add(visitFeatureCall);
                    }

                    // CODE visitor.Visit(namespaceSparse, () => { visitor.Visit(feature1); visitor.Visit(feature2); ... });
                    visitationNamespace.Add(Expression.Call(
                        visitorParameter,
                        typeof(IVowpalWabbitVisitor).GetMethod("Visit", new[] { typeof(NamespaceSparse), typeof(Action) }),
                        namespaceVariable,
                        Expression.Lambda<Action>(Expression.Block(featureVisitation))    
                        ));
                }
	        }

            // Extract comment value
            var sharedComment = ExtractComment(valueParameter);

            // Find action dependent features
            var perAction = (from p in valueType.GetProperties(BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public)
                             let attr = p.GetCustomAttributes(typeof(ActionDependentFeaturesAttribute), true).FirstOrDefault()
                             where attr != null
                             select new { Property = p, Attribute = attr }
                ).FirstOrDefault();

            var perActionVisitation = new List<Expression>();
            if (perAction != null)
            {
                sharedComment = Expression.Constant("shared");

                // Get Serializer Action Method
                // construct loop to go through PerActionSamples
                var perActionType = perAction.Property.PropertyType;

                if (!typeof(IEnumerable<object>).IsAssignableFrom(perActionType))
                {
                    throw new NotSupportedException("PerAction types must implement from IEnumerable<>");
                }

                // find IEnumerable
                var perActionItemType = perActionType.GetInterfaces().Union(new[] { perActionType })
                    .Where(i => i.IsGenericType && i.GetGenericTypeDefinition() == typeof(IEnumerable<>))
                    .First()
                    .GetGenericArguments()[0];

                // Build serializer for PerAction feature
                var createSerializer = typeof(VWSerializer)
                    .GetMethod("CreateSerializer", BindingFlags.Static | BindingFlags.NonPublic, null, new[] { typeof(ModuleBuilder) }, null)
                    .MakeGenericMethod(perActionItemType, typeof(TVisitor));

                var serializer = (Delegate)createSerializer.Invoke(null, new [] { moduleBuilder });
                
                var enumerator = Expression.Variable(typeof(IEnumerator<>).MakeGenericType(perActionItemType));
                variables.Add(enumerator);

                // CODE enumerator = value.ActionDependentFeatures.GetEnumerator()
                perActionVisitation.Add(Expression.Assign(
                    enumerator, 
                    Expression.Call(Expression.Property(valueParameter, perAction.Property), perActionType.GetMethod("GetEnumerator"))));

                // exit: while(true) { if (enumerator.MoveNext()) { ... } else { goto exit; } }
                var loopBreak = Expression.Label();
                perActionVisitation.Add(Expression.Loop(Expression.Block(
                        Expression.IfThenElse(
                            test: Expression.Call(enumerator, typeof(IEnumerator).GetMethod("MoveNext")),
                            // serializer(visitor, enumerator.Current);
                            ifTrue:Expression.Call(
                                serializer.Method,
                                Expression.Property(enumerator, "Current"),
                                visitorParameter),
                            ifFalse:Expression.Break(loopBreak))
                    ),
                    loopBreak));
            }
            
            // visitor.Visit(comment, new[] { ns1, ns2, ... }, () => { visitor.Visit(ns1); visitor.Visit(ns2); ... });
            visitation.Add(Expression.Call(
                        visitorParameter,
                        typeof(TVisitor).GetMethod("Visit", new[] { typeof(string), typeof(Namespace[]), typeof(Action) }),
                        // comment:
                        sharedComment,
                        // namespaces: 
                        Expression.NewArrayInit(typeof(Namespace), namespaceVariables.ToArray()),
                        // visitNamespaces:
                        Expression.Lambda<Action>(Expression.Block(visitationNamespace))));

            var visit = Expression.Lambda<Action<TContext, TVisitor>>(
                Expression.Block(variables.Union(namespaceVariables), visitation.Union(perActionVisitation)),
                valueParameter,
                visitorParameter);

            var typeBuilder = moduleBuilder.DefineType("VowpalWabbitSerializer" + Guid.NewGuid().ToString().Replace('-', '_'));

            // Create our method builder for this type builder
            var methodBuilder = typeBuilder.DefineMethod("Serialize", 
                MethodAttributes.Public | MethodAttributes.Static,
                typeof(void), 
                new[] { typeof(TContext), typeof(TVisitor) });

            // compared to Compile this looks rather ugly, but there is a feature-bug 
            // that adds a security check to every call of the Serialize method
            visit.CompileToMethod(methodBuilder);

            var dynType = typeBuilder.CreateType();

            return (Action<TContext, TVisitor>)Delegate.CreateDelegate(typeof(Action<TContext, TVisitor>), dynType.GetMethod("Serialize"));
        }

        internal static bool IsValidDenseFeatureValueElementType(Type elemType)
        {
            return elemType == typeof(double)
                    || elemType == typeof(float)
                    || elemType == typeof(Int16)
                    || elemType == typeof(Int32)
                    || elemType == typeof(Int64);
        }

        internal static Type GetDenseFeatureValueElementType(Type type)
        {
            if (type.IsArray)
            {
                // TODO: also support IList/List
                var elemType = type.GetElementType();

                // numeric types
                if (IsValidDenseFeatureValueElementType(elemType))
                {
                    return elemType;
                }
            }

            if (typeof(IEnumerable<object>).IsAssignableFrom(type))
            {
                // let's get T of IEnumerable<T>
                return type.GetInterfaces()
                    .First(it => it.IsGenericType && it.GetGenericTypeDefinition() == typeof(IEnumerable<>))
                    .GetGenericArguments()[0];
            }

            return null;
        }

        private static IList<FeatureExpression> ExtractFeaturesCompiled(Expression valueExpression, string parentNamespace, char? parentFeatureGroup)
        {
            var props = valueExpression.Type.GetProperties(BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public);

            var localFeatures = from p in props
                                 let attr = (FeatureAttribute)p.GetCustomAttributes(typeof(FeatureAttribute), true).FirstOrDefault()
                                 where attr != null
                                 let featureValueType = p.PropertyType
                                 let featureType = typeof(Feature<>).MakeGenericType(featureValueType)
                                 let namespaceValue = attr.Namespace ?? parentNamespace
                                 let featureGroup = attr.InternalFeatureGroup ?? parentFeatureGroup
                                 let propertyExpression = Expression.Property(valueExpression, p)
                                 let name = p.Name
                                 select new FeatureExpression
                                 {
                                     Name = name,
                                     Namespace = namespaceValue,
                                     FeatureGroup = featureGroup,
                                     FeatureType = featureType,
                                     FeatureValueType = featureValueType,
                                     DenseFeatureValueElementType = GetDenseFeatureValueElementType(featureValueType),
                                     PropertyExpression = propertyExpression,
                                     // new Feature<T> { Namespace = ..., ... } 
                                     NewFeatureExpression = Expression.MemberInit(
                                        Expression.New(featureType),
                                        Expression.Bind(featureType.GetProperty("Name"), Expression.Constant(name)),
                                        Expression.Bind(featureType.GetProperty("Enumerize"), Expression.Constant(attr.Enumerize)),
                                        Expression.Bind(featureType.GetProperty("Value"), propertyExpression),
                                        Expression.Bind(featureType.GetProperty("Namespace"), Expression.Constant(namespaceValue)),
                                        Expression.Bind(featureType.GetProperty("FeatureGroup"), Expression.Constant(featureGroup)))
                                 };

            return localFeatures
                .Select(f => 
                {
                    var subFeatures = ExtractFeaturesCompiled(f.PropertyExpression, f.Namespace, f.FeatureGroup);
                    return subFeatures.Count == 0 ? new[] { f } : subFeatures;
                })
                .SelectMany(f => f)
                .ToList();
        }

        private static Expression ExtractComment(Expression expression)
        {
            var props = expression.Type.GetProperties(BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public);

            var comment = (from p in props
                           let attr = p.GetCustomAttributes(typeof(CommentAttribute), true).FirstOrDefault()
                           where attr != null
                           select p)
                .FirstOrDefault();

            if (comment == null)
            {
                return Expression.Constant(string.Empty);
            }

            return Expression.Property(expression, comment);
        }
    }
}
