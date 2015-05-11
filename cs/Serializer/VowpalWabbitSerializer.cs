using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer
{
    using System.Collections;
    using System.Diagnostics;
    using System.Globalization;
    using System.Linq.Expressions;
    using System.Reflection.Emit;
    using System.Security;
    using System.Security.Permissions;
    using Microsoft.Research.MachineLearning.Serializer.Attributes;
    using Microsoft.Research.MachineLearning.Serializer.Interfaces;
    using Microsoft.Research.MachineLearning.Serializer.Intermediate;
    using Microsoft.Research.MachineLearning.Serializer.Reflection;
    using VwHandle = IntPtr;

    public static class VowpalWabbitSerializer
    {
        private static readonly Dictionary<Tuple<Type, Type>, object> SerializerCache = new Dictionary<Tuple<Type, Type>, object>();

        public static Func<TContext, TVisitor, IList<TExampleResult>> CreateSerializer<TContext, TVisitor, TExampleResult, TNamespaceResult, TFeatureResult>()
            where TVisitor : IVowpalWabbitVisitor<TExampleResult, TNamespaceResult, TFeatureResult>
        {
            var cacheKey = Tuple.Create(typeof(TContext), typeof(TVisitor));
            object serializer;

            if (SerializerCache.TryGetValue(cacheKey, out serializer))
            {
                return (Func<TContext, TVisitor, IList<TExampleResult>>)serializer;
            }

            // Create dynamic assembly
            var asmName = new AssemblyName("VowpalWabbitSerializer." + typeof(TContext).Name + "." + typeof(TVisitor));
            var dynAsm = AppDomain.CurrentDomain.DefineDynamicAssembly(asmName, AssemblyBuilderAccess.RunAndSave);

            // Create a dynamic module and type
            var dynMod = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule");

            var newSerializer = CreateSerializer<TContext, TVisitor, TExampleResult, TNamespaceResult, TFeatureResult>(dynMod);

            SerializerCache[cacheKey] = newSerializer;

            return newSerializer;
        }

        private static Func<TContext, TVisitor, IList<TExampleResult>> CreateSerializer<TContext, TVisitor, TExampleResult, TNamespaceResult, TFeatureResult>(ModuleBuilder moduleBuilder)
            where TVisitor : IVowpalWabbitVisitor<TExampleResult, TNamespaceResult, TFeatureResult>
        {            
            var valueType = typeof(TContext);

            // define functions input parameter
            var valueParameter = Expression.Parameter(valueType, "value");
            var visitorParameter = Expression.Parameter(typeof(TVisitor), "visitor");

            // find all features and group by namespace
            var featuresByNamespace = ExtractFeaturesCompiled<TFeatureResult>(valueParameter, null, null)
                    .GroupBy(f => new { f.Namespace, f.FeatureGroup, f.IsDense }, f => f);

            var body = new List<Expression>();
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
                    var namespaceType = typeof(NamespaceDense<,>).MakeGenericType(feature.DenseFeatureValueElementType, typeof(TNamespaceResult));

                    var namespaceDense = Expression.MemberInit(
                        Expression.New(namespaceType),
                        baseNamespaceInits.Union(new[] { 
                            Expression.Bind(namespaceType.GetProperty("DenseFeature"), feature.NewFeatureExpression)
                        }));

                    var namespaceVariable = Expression.Variable(namespaceType);
                    namespaceVariables.Add(namespaceVariable);

                    // CODE namespace = new Namespace<float> { ... };
                    body.Add(Expression.Assign(namespaceVariable, namespaceDense));

                    // CODE namespace.Visit = () => visitor.Visit(namespace)
                    body.Add(Expression.Assign(
                            Expression.Property(namespaceVariable, namespaceType.GetProperty("Visit")),
                            Expression.Lambda<Func<TNamespaceResult>>(
                                Expression.Call(
                                    visitorParameter,
                                    ReflectionHelper.FindMethod(typeof(TVisitor), "Visit", namespaceType),
                                    namespaceVariable))));
                }
                else 
                {
                    // Sparse namespace
                    var featureVariables = new List<ParameterExpression>();

                    foreach (var feature in features)
	                {
                        var featureVariable = Expression.Parameter(feature.FeatureType, feature.Name);

		                variables.Add(featureVariable);
                        featureVariables.Add(featureVariable);

                        // CODE feature = new Feature<float> { ... };
                        body.Add(Expression.Assign(featureVariable, feature.NewFeatureExpression));
	                }

                    // CODE new NamespaceSparse { Features = new[] { feature1, feature2, ... } }
                    var namespaceSparse = Expression.MemberInit(
                        Expression.New(typeof(NamespaceSparse<TNamespaceResult, TFeatureResult>)),
                        baseNamespaceInits.Union(new[] { 
                            Expression.Bind(
                                typeof(NamespaceSparse<TNamespaceResult, TFeatureResult>).GetProperty("Features"),
                                Expression.NewArrayInit(typeof(IVisitableFeature<TFeatureResult>), featureVariables))
                        }));

                    var namespaceVariable = Expression.Variable(typeof(NamespaceSparse<TNamespaceResult, TFeatureResult>), "namespaceSparse");
                    namespaceVariables.Add(namespaceVariable);

                    // CODE namespace = new NamespaceSparse { ... }
                    body.Add(Expression.Assign(namespaceVariable, namespaceSparse));

                    // loop unrolling to have dispatch onto the correct Visit<T>
                    for (int i = 0; i < features.Count; i++)
                    {
                        var feature = features[i];
                        var featureVariable = featureVariables[i];
                     
                        // CODE: visitor.Visit(feature1); 
                        Expression visitFeatureCall = Expression.Call(
                                    visitorParameter,
                                    ReflectionHelper.FindMethod(typeof(TVisitor), feature.Enumerize ? "Visit" : "VisitEnumerize", featureVariable.Type),
                                    featureVariable);

                        var featureValue = Expression.Property(featureVariable, "Value");
                        if (!featureValue.Type.IsValueType || (featureValue.Type.IsGenericType && featureValue.Type.GetGenericTypeDefinition() == typeof(Nullable<>)))
                        {
                            // CODE: feature1.Value != null ? visitor.Visit(feature1) : default(TFeatureResult);
                            visitFeatureCall = Expression.Condition(
                                test: Expression.NotEqual(featureValue, Expression.Constant(null)),
                                ifTrue: visitFeatureCall,
                                ifFalse: Expression.Constant(default(TFeatureResult), typeof(TFeatureResult)));
                        }

                        // CODE feature.Visit = () => visitor.Visit( *visitFeatureCall* );
                        body.Add(
                            Expression.Assign(
                                Expression.Property(featureVariable, featureVariable.Type.GetProperty("Visit")),
                                Expression.Lambda<Func<TFeatureResult>>(visitFeatureCall)));
                    }

                    // CODE namespace.Visit = () => { visitor.Visit(namespace); });
                    body.Add(
                        Expression.Assign(
                            Expression.Property(namespaceVariable, namespaceVariable.Type.GetProperty("Visit")),
                            Expression.Lambda<Func<TNamespaceResult>>(
                                Expression.Call(
                                    visitorParameter,
                                    visitorParameter.Type.GetMethod("Visit", new[] { typeof(NamespaceSparse<TNamespaceResult, TFeatureResult>) }),
                                    namespaceVariable))));
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

            var exampleResults = Expression.Variable(typeof(List<TExampleResult>), "exampleResults");
            
            variables.Add(exampleResults);
            // CODE exampleResults = new List<TExampleResult>(); 
            body.Add(Expression.Assign(exampleResults, Expression.New(exampleResults.Type)));

            var listAddMethod = exampleResults.Type.GetMethod("Add", new[] { typeof(TExampleResult) });
            var visitNamespaceMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(string), typeof(IVisitableNamespace<TNamespaceResult>[]) });

            // CODE exampleResults.Add(visitor.Visit(comment, new[] { ns1, ns2, ... }));
            body.Add(
                Expression.Call(
                    exampleResults,
                    listAddMethod,
                    Expression.Call(
                        visitorParameter,
                        visitNamespaceMethod,
                        // comment:
                        sharedComment,
                        // namespaces: 
                        Expression.NewArrayInit(
                            typeof(IVisitableNamespace<TNamespaceResult>),
                            namespaceVariables.ToArray()))));

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
                var createSerializer = typeof(VowpalWabbitSerializer)
                    .GetMethod("CreateSerializer", BindingFlags.Static | BindingFlags.NonPublic, null, new[] { typeof(ModuleBuilder) }, null)
                    .MakeGenericMethod(perActionItemType, typeof(TVisitor), typeof(TExampleResult), typeof(TNamespaceResult), typeof(TFeatureResult));

                var serializer = (Delegate)createSerializer.Invoke(null, new [] { moduleBuilder });
                
                var enumerator = Expression.Variable(typeof(IEnumerator<>).MakeGenericType(perActionItemType));
                variables.Add(enumerator);

                // CODE enumerator = value.ActionDependentFeatures.GetEnumerator()
                body.Add(Expression.Assign(
                    enumerator, 
                    Expression.Call(Expression.Property(valueParameter, perAction.Property), perActionType.GetMethod("GetEnumerator"))));

                // CODE exit: while(true) { if (enumerator.MoveNext()) { ... } else { goto exit; } }
                var loopBreak = Expression.Label();
                body.Add(Expression.Loop(Expression.Block(
                        Expression.IfThenElse(
                            test: Expression.Call(enumerator, typeof(IEnumerator).GetMethod("MoveNext")),
                            // CODE exampleResults.AddRange(serializer(visitor, enumerator.Current));
                            ifTrue: Expression.Call(
                                exampleResults,
                                exampleResults.Type.GetMethod("AddRange"),
                                Expression.Call(
                                    serializer.Method,
                                    Expression.Property(enumerator, "Current"),
                                    visitorParameter)),
                            ifFalse:Expression.Break(loopBreak))),
                    loopBreak));
            }

            // CODE: return exampleResults;
            body.Add(exampleResults);

            var visit = Expression.Lambda<Func<TContext, TVisitor, IList<TExampleResult>>>(
                Expression.Block(variables.Union(namespaceVariables), body),
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

            return (Func<TContext, TVisitor, IList<TExampleResult>>)Delegate.CreateDelegate(typeof(Func<TContext, TVisitor, IList<TExampleResult>>), dynType.GetMethod("Serialize"));
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

        private static IList<FeatureExpression> ExtractFeaturesCompiled<TFeatureResult>(Expression valueExpression, string parentNamespace, char? parentFeatureGroup)
        {
            var props = valueExpression.Type.GetProperties(BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public);

            var localFeatures = from p in props
                                 let attr = (FeatureAttribute)p.GetCustomAttributes(typeof(FeatureAttribute), true).FirstOrDefault()
                                 where attr != null
                                 let featureValueType = p.PropertyType
                                 let featureType = typeof(Feature<,>).MakeGenericType(featureValueType, typeof(TFeatureResult))
                                 let namespaceValue = attr.Namespace ?? parentNamespace
                                 let featureGroup = attr.InternalFeatureGroup ?? parentFeatureGroup
                                 let propertyExpression = Expression.Property(valueExpression, p)
                                 let name = p.Name
                                 select new FeatureExpression
                                 {
                                     Name = name,
                                     Namespace = namespaceValue,
                                     Enumerize = attr.Enumerize,
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
                    var subFeatures = ExtractFeaturesCompiled<TFeatureResult>(f.PropertyExpression, f.Namespace, f.FeatureGroup);
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
