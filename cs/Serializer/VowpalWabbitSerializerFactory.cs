using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Research.MachineLearning.Serializer.Attributes;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;
using Microsoft.Research.MachineLearning.Serializer.Intermediate;
using Microsoft.Research.MachineLearning.Serializer.Reflection;
using Microsoft.Research.MachineLearning.Serializer.Visitors;

namespace Microsoft.Research.MachineLearning.Serializer
{
    /// <summary>
    /// Factory to ease creation of serializers
    /// </summary>
    public static class VowpalWabbitSerializerFactory
    {
        /// <summary>
        /// Example and Example Result type based serializer cache.
        /// </summary>
        private static readonly Dictionary<Tuple<Type, Type>, object> SerializerCache = new Dictionary<Tuple<Type, Type>, object>();

        public static VowpalWabbitSerializer<TExample, VowpalWabbitExample> CreateSerializer<TExample>(VowpalWabbitInterfaceVisitor visitor)
        {
            var serializerFunc = CreateSerializer<TExample, VowpalWabbitInterfaceVisitor, VowpalWabbitExample, FEATURE[], IEnumerable<FEATURE>>();
            if (serializerFunc == null)
            {
                return null;
            }

            return new VowpalWabbitSerializer<TExample, VowpalWabbitExample>(ex => serializerFunc(ex, visitor));
        }

        public static VowpalWabbitSerializer<TExample, string> CreateSerializer<TExample>(VowpalWabbitStringVisitor visitor)
        {
            var serializerFunc = CreateSerializer<TExample, VowpalWabbitStringVisitor, string, string, string>();
            if (serializerFunc == null)
            {
                return null;
            }

            return new VowpalWabbitSerializer<TExample, string>(ex => serializerFunc(ex, visitor));
        }

        public static Func<TExample, TVisitor, TExampleResult> CreateSerializer<TExample, TVisitor, TExampleResult, TNamespaceResult, TFeatureResult>()
            where TVisitor : IVowpalWabbitVisitor<TExampleResult, TNamespaceResult, TFeatureResult>
        {
            var cacheKey = Tuple.Create(typeof(TExample), typeof(TVisitor));
            object serializer;

            if (SerializerCache.TryGetValue(cacheKey, out serializer))
            {
                return (Func<TExample, TVisitor, TExampleResult>)serializer;
            }

            // Create dynamic assembly
            var asmName = new AssemblyName("VowpalWabbitSerializer." + typeof(TExample).Name + "." + typeof(TVisitor));
            var dynAsm = AppDomain.CurrentDomain.DefineDynamicAssembly(asmName, AssemblyBuilderAccess.RunAndSave);

            // Create a dynamic module and type
            var dynMod = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule");

            var newSerializer = CreateSerializer<TExample, TVisitor, TExampleResult, TNamespaceResult, TFeatureResult>(dynMod);

            SerializerCache[cacheKey] = newSerializer;

            return newSerializer;
        }

        private static Func<TExample, TVisitor, TExampleResult> CreateSerializer<TExample, TVisitor, TExampleResult, TNamespaceResult, TFeatureResult>(ModuleBuilder moduleBuilder)
            where TVisitor : IVowpalWabbitVisitor<TExampleResult, TNamespaceResult, TFeatureResult>
        {
            var valueType = typeof(TExample);

            // define functions input parameter
            var valueParameter = Expression.Parameter(valueType, "value");
            var visitorParameter = Expression.Parameter(typeof(TVisitor), "visitor");

            // find all features and group by namespace
            var allFeatures = ExtractFeaturesCompiled<TFeatureResult>(valueParameter, null, null).ToList();
            if (allFeatures.Count == 0)
            {
                return null;
            }

            var featuresByNamespace = allFeatures
                    .GroupBy(f => new { f.Namespace, f.FeatureGroup, f.IsDense }, f => f);

            var body = new List<Expression>();
            var variables = new List<ParameterExpression>();
            var namespaceVariables = new List<ParameterExpression>();
            var visitationNamespace = new List<Expression>();

            foreach (var ns in featuresByNamespace)
            {
                var features = ns.ToList();

                var baseNamespaceType = typeof(Namespace);
                var baseNamespaceInits = new[] {
                    Expression.Bind(baseNamespaceType.GetProperty("Name"), Expression.Constant(ns.Key.Namespace, typeof(string))),
                    Expression.Bind(baseNamespaceType.GetProperty("FeatureGroup"), Expression.Constant(ns.Key.FeatureGroup, typeof(char?))),
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
                                    ReflectionHelper.FindMethod(typeof(TVisitor), feature.Enumerize ? "VisitEnumerize" : "Visit", featureVariable.Type),
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

            var visitNamespaceMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(IVisitableNamespace<TNamespaceResult>[]) });

            // CODE return visitor.Visit(comment, new[] { ns1, ns2, ... })
            body.Add(
                Expression.Call(
                    visitorParameter,
                    visitNamespaceMethod,
                    Expression.NewArrayInit(
                        typeof(IVisitableNamespace<TNamespaceResult>),
                        namespaceVariables.ToArray())));


            var visit = Expression.Lambda<Func<TExample, TVisitor, TExampleResult>>(
                Expression.Block(variables.Union(namespaceVariables), body),
                valueParameter,
                visitorParameter);

            var typeBuilder = moduleBuilder.DefineType("VowpalWabbitSerializer" + Guid.NewGuid().ToString().Replace('-', '_'));

            // Create our method builder for this type builder
            var methodBuilder = typeBuilder.DefineMethod("Serialize",
                MethodAttributes.Public | MethodAttributes.Static,
                typeof(void),
                new[] { typeof(TExample), typeof(TVisitor) });

            // compared to Compile this looks rather ugly, but there is a feature-bug 
            // that adds a security check to every call of the Serialize method
            visit.CompileToMethod(methodBuilder);

            var dynType = typeBuilder.CreateType();

            return (Func<TExample, TVisitor, TExampleResult>)Delegate.CreateDelegate(typeof(Func<TExample, TVisitor, TExampleResult>), dynType.GetMethod("Serialize"));
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
                var elemType = type.GetInterfaces().Union(new[] { type })
                    .First(it => it.IsGenericType && it.GetGenericTypeDefinition() == typeof(IEnumerable<>))
                    .GetGenericArguments()[0];

                if (IsValidDenseFeatureValueElementType(elemType))
                {
                    return elemType;
                }
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
                                    // CODE new Feature<T> { Namespace = ..., ... } 
                                    NewFeatureExpression = Expression.MemberInit(
                                       Expression.New(featureType),
                                       Expression.Bind(featureType.GetProperty("Name"), Expression.Constant(name)),
                                       Expression.Bind(featureType.GetProperty("Enumerize"), Expression.Constant(attr.Enumerize)),
                                       Expression.Bind(featureType.GetProperty("Value"), propertyExpression),
                                       Expression.Bind(featureType.GetProperty("Namespace"), Expression.Constant(namespaceValue, typeof(string))),
                                       Expression.Bind(featureType.GetProperty("FeatureGroup"), Expression.Constant(featureGroup, typeof(char?))))
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
    }
}
