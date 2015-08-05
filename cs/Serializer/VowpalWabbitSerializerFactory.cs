// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializerFactory.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using VW.Serializer.Attributes;
using VW.Serializer.Interfaces;
using VW.Serializer.Intermediate;
using VW.Serializer.Reflection;
using VW.Serializer.Visitors;
using VW.Interfaces;
using System.IO;
using System.Runtime.CompilerServices;

namespace VW.Serializer
{
    /// <summary>
    /// Factory to ease creation of serializers.
    /// 
    /// Visit(ILabel label, IVisitableNamespace[] namespaces) Invoked for each example. Implementors must dispatch by calling <see cref="IVisitableNamespace.Visit()"/>.
    /// Visit<T>(INamespaceDense<T> namespaceDense); Invoked for each namespace.
    /// void Visit(INamespaceSparse namespaceSparse); Invoked for each namespace.
    /// void Visit<T>(IFeature<T> feature); Invoked for each feature.
    /// void VisitEnumerize<T>(IFeature<T> feature); Invoked for each feature which is additionally flagged by the enumerize option.
    /// </summary>
    public static class VowpalWabbitSerializerFactory
    {
        /// <summary>
        /// Example and example result type based serializer cache.
        /// </summary>
        private static readonly Dictionary<Tuple<Type, Type>, object> SerializerCache = new Dictionary<Tuple<Type, Type>, object>();

        private static readonly string SerializeMethodName = "Serialize";

        private static readonly ConstructorInfo ArgumentNullExceptionConstructorInfo = (ConstructorInfo)ReflectionHelper.GetInfo((ArgumentNullException t) => new ArgumentNullException(""));

        /// <summary>
        /// Compiles a serializers for the given example user type.
        /// </summary>
        /// <typeparam name="TExample">The example user type.</typeparam>
        /// <param name="visitor">The visitor to be used for serialization.</param>
        /// <param name="settings">The serializer settings.</param>
        /// <returns>A serializer for the given user example type.</returns>
        public static VowpalWabbitSerializer<TExample> CreateSerializer<TExample>(VowpalWabbitSettings settings)
        {
            var serializerFunc = CreateSerializer<TExample, VowpalWabbitInterfaceVisitor, VowpalWabbitExample>();
            if (serializerFunc == null)
            {
                // if no features are found, no serializer is generated
                serializerFunc = (_,__,___) => null;
            }

#if DEBUG
            var stringSerializerFunc = CreateSerializer<TExample, VowpalWabbitStringVisitor, string>();

            return new VowpalWabbitSerializer<TExample>(
                (vw, example, label) => new VowpalWabbitDebugExample(serializerFunc(vw, example, label), stringSerializerFunc(vw, example, label)),
                settings);
#else
            return new VowpalWabbitSerializer<TExample>(serializerFunc, settings);
#endif
        }

        /// <summary>
        /// Compiles a serializers for the given example user type.
        /// </summary>
        /// <typeparam name="TExample">The example user type.</typeparam>
        /// <typeparam name="TVisitor">The visitor to be used for serialization.</typeparam>
        /// <typeparam name="TExampleResult">The resulting serialization type.</typeparam>
        /// <returns>A serializer for the given user example type.</returns>
        public static Func<VowpalWabbit, TExample, ILabel, TExampleResult> CreateSerializer<TExample, TVisitor, TExampleResult>()
            // where TVisitor : IVowpalWabbitVisitor<TExampleResult>
        {
            var cacheKey = Tuple.Create(typeof(TExample), typeof(TVisitor));
            object serializer;

            if (SerializerCache.TryGetValue(cacheKey, out serializer))
            {
                return (Func<VowpalWabbit, TExample, ILabel, TExampleResult>)serializer;
            }

            // Create dynamic assembly
            var asmName = new AssemblyName("VowpalWabbitSerializer." + typeof(TExample).Name + "." + typeof(TVisitor));
            var dynAsm = AppDomain.CurrentDomain.DefineDynamicAssembly(asmName, AssemblyBuilderAccess.RunAndSave);

            // Create a dynamic module and type
//#if !DEBUG
            //var dynMod = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule", asmName.Name + ".dll", true);
//#else
            var dynMod = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule");
//#endif       
            var newSerializer = CreateSerializer<TVisitor, TExample, TExampleResult>(dynMod);

            SerializerCache[cacheKey] = newSerializer;

            return newSerializer;
        }

        //public static Expression Log([CallerFilePath] string filePath = "", [CallerLineNumber] int lineNumber = 0, Expression expression = null)
        //{
        //    var file = Expression.Constant(@"c:\vowpal_wabbit\test.log");
        //    var logMethod = (MethodInfo)ReflectionHelper.GetInfo((string a) => File.AppendAllText(a, a));
        //    var toString = (MethodInfo)ReflectionHelper.GetInfo((object a) => a.ToString());

        //    if (expression == null)
        //        return Expression.Call(logMethod, file, Expression.Constant(filePath + ":" + lineNumber + "\n"));

        //    return Expression.Block(
        //        Expression.Call(logMethod, file, Expression.Constant(filePath + ":" + lineNumber + "\n")),
        //        Expression.Call(logMethod, file, Expression.Call(expression, toString)));
        //}

        private static Func<VowpalWabbit, TExample, ILabel, TExampleResult> CreateSerializer<TVisitor, TExample, TExampleResult>(ModuleBuilder moduleBuilder)
            // where TVisitor : IVowpalWabbitVisitor<TExampleResult>
        {
            var valueType = typeof(TExample);

            // define functions input parameter
            var valueParameter = Expression.Parameter(valueType, "value");
            var labelParameter = Expression.Parameter(typeof(ILabel), "label");
            var vwParameter = Expression.Parameter(typeof(VowpalWabbit), "vw");
            var visitorParameter = Expression.Variable(typeof(TVisitor), "visitor");

            // find all features and group by namespace
            var allFeatures = ExtractFeaturesCompiled(valueParameter, null, null).ToList();
            if (allFeatures.Count == 0)
            {
                return null;
            }

            var featuresByNamespace = allFeatures.GroupBy(f => new { f.Namespace, f.FeatureGroup, f.IsDense }, f => f);

            var body = new List<Expression>();

            //// CODE if (value == null) throw new ArgumentNullException("value");
            //body.Add(Log());
            //body.Add(Expression.IfThen(
            //        Expression.Equal(valueParameter, Expression.Constant(null)),
            //        Expression.Throw(Expression.New(ArgumentNullExceptionConstructorInfo, Expression.Constant("value")))));

            //// CODE if (value == null) throw new ArgumentNullException("visitor");
            //body.Add(Log());
            //body.Add(Expression.IfThen(
            //        Expression.Equal(visitorParameter, Expression.Constant(null)),
            //        Expression.Throw(Expression.New(ArgumentNullExceptionConstructorInfo, Expression.Constant("visitor")))));
            
            var visitorCtor = typeof(TVisitor).GetConstructor(new[] { typeof(VowpalWabbit) });
            if (visitorCtor != null)
            {
                // visitor = new TVisitor(vw)
                body.Add(Expression.Assign(visitorParameter,
                    Expression.New(visitorCtor, vwParameter)));
            }
            else
            {
                // visitor = new TVisitor()
                body.Add(Expression.Assign(visitorParameter, Expression.New(typeof(TVisitor))));
            }

            var variables = new List<ParameterExpression>() { visitorParameter };
            var namespaceVariables = new List<ParameterExpression>();

            foreach (var ns in featuresByNamespace)
            {
                var features = ns.OrderBy(f => f.Order).ToList();

                var baseNamespaceInits = new List<MemberAssignment> 
                {
                    Expression.Bind(
                        ReflectionHelper.GetInfo((Namespace n) => n.Name),
                        Expression.Constant(ns.Key.Namespace, typeof(string)))
                };

                if (ns.Key.FeatureGroup != null)
                {
                    baseNamespaceInits.Add(
                        Expression.Bind(
                            ReflectionHelper.GetInfo((Namespace n) => n.FeatureGroup), 
                            Expression.Convert(Expression.Constant((char)ns.Key.FeatureGroup), typeof(char?))));
                }

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
                    //body.Add(Log());
                    body.Add(Expression.Assign(namespaceVariable, namespaceDense));

                    // CODE namespace.Visit = () => visitor.Visit(namespace)
                    //body.Add(Log());
                    body.Add(Expression.Assign(
                            Expression.Property(namespaceVariable, namespaceType.GetProperty("Visit")),
                            Expression.Lambda<Action>(
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
                        var featureVariable = Expression.Parameter(feature.FeatureType, feature.PropertyName);

                        variables.Add(featureVariable);
                        featureVariables.Add(featureVariable);

                        // CODE feature = new Feature<float> { ... };
                        //body.Add(Log());
                        body.Add(Expression.Assign(featureVariable, feature.NewFeatureExpression));
                    }

                    // CODE new NamespaceSparse { Features = new[] { feature1, feature2, ... } }
                    var namespaceSparse = Expression.MemberInit(
                        Expression.New(typeof(NamespaceSparse)),
                        baseNamespaceInits.Union(new[] { 
                            Expression.Bind(
                                ReflectionHelper.GetInfo((NamespaceSparse n) => n.Features),
                                Expression.NewArrayInit(typeof(IVisitableFeature), featureVariables))
                        }));

                    var namespaceVariable = Expression.Variable(typeof(NamespaceSparse), "namespaceSparse");
                    namespaceVariables.Add(namespaceVariable);

                    // CODE namespace = new NamespaceSparse { ... }
                    //body.Add(Log());
                    body.Add(Expression.Assign(namespaceVariable, namespaceSparse));

                    // loop unrolling to have dispatch onto the correct Visit<T>
                    for (var i = 0; i < features.Count; i++)
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
                            // CODE feature1.Value != null ? visitor.Visit(feature1) : default(TFeatureResult);
                            visitFeatureCall = Expression.IfThen(
                                    test: Expression.NotEqual(featureValue, Expression.Constant(null)),
                                    ifTrue: visitFeatureCall);
                        }

                        // CODE feature.Visit = visitor.Visit;
                        //body.Add(Log());
                        body.Add(
                            Expression.Assign(
                                Expression.Property(featureVariable, featureVariable.Type.GetProperty("Visit")),
                                Expression.Lambda<Action>(visitFeatureCall)));
                    }

                    // CODE namespace.Visit = () => { visitor.Visit(namespace); });
                    //body.Add(Log());
                    // return (MethodInfo)ReflectionHelper.GetInfo((IVowpalWabbitVisitor<TExampleResult> e) => e.Visit((NamespaceSparse)null));
                    var visitMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(INamespaceSparse) });
                    body.Add(
                        Expression.Assign(
                            Expression.Property(
                                namespaceVariable,
                                (PropertyInfo)ReflectionHelper.GetInfo((NamespaceSparse n) => n.Visit)),
                            Expression.Lambda<Action>(
                                Expression.Call(
                                    visitorParameter,
                                    visitMethod,
                                    namespaceVariable))));
                }
            }


            // CODE return visitor.Visit(label, new[] { ns1, ns2, ... })
            //body.Add(Log());
            var visitWithLabelMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(ILabel), typeof(IVisitableNamespace[]) });
            body.Add(
                Expression.Call(
                    visitorParameter,
                    visitWithLabelMethod,
                    labelParameter,
                    Expression.NewArrayInit(
                        typeof(IVisitableNamespace),
                        namespaceVariables.ToArray<Expression>())));


            // CODE (example, visitor) => { ... }
            var visit = Expression.Lambda<Func<VowpalWabbit, TExample, ILabel, TExampleResult>>(
                Expression.Block(variables.Union(namespaceVariables), body),
                vwParameter,
                valueParameter,
                labelParameter);
            
            var typeBuilder = moduleBuilder.DefineType("VowpalWabbitSerializer" + Guid.NewGuid().ToString().Replace('-', '_'));

            // Create our method builder for this type builder
            var methodBuilder = typeBuilder.DefineMethod(
                SerializeMethodName,
                MethodAttributes.Public | MethodAttributes.Static,
                typeof(void),
                new[] { typeof(VowpalWabbit), typeof(TExample), typeof(ILabel) });

            // compared to Compile this looks rather ugly, but there is a feature-bug 
            // that adds a security check to every call of the Serialize method
//#if !DEBUG
            //var debugInfoGenerator = DebugInfoGenerator.CreatePdbGenerator();
            //visit.CompileToMethod(methodBuilder, debugInfoGenerator);
//#else
            visit.CompileToMethod(methodBuilder);
//#endif
            var dynType = typeBuilder.CreateType();

            return (Func<VowpalWabbit, TExample, ILabel, TExampleResult>)Delegate.CreateDelegate(
                typeof(Func<VowpalWabbit, TExample, ILabel, TExampleResult>),
                dynType.GetMethod(SerializeMethodName));                                        
        }

        internal static bool IsValidDenseFeatureValueElementType(Type elemType)
        {
            return elemType == typeof(double)
                    || elemType == typeof(float)
                    || elemType == typeof(byte)
                    || elemType == typeof(sbyte)
                    || elemType == typeof(char)
                    || elemType == typeof(decimal)
                    || elemType == typeof(UInt16)
                    || elemType == typeof(UInt32)
                    || elemType == typeof(UInt64)
                    || elemType == typeof(Int16)
                    || elemType == typeof(Int32)
                    || elemType == typeof(Int64);
        }

        internal static Type GetDenseFeatureValueElementType(Type type)
        {
            if (type.IsArray)
            {
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
                                let name = attr.Name ?? p.Name
                                select new FeatureExpression
                                {
                                    Name = name,
                                    PropertyName = p.Name,
                                    Namespace = namespaceValue,
                                    Enumerize = attr.Enumerize,
                                    FeatureGroup = featureGroup,
                                    FeatureType = featureType,
                                    FeatureValueType = featureValueType,
                                    DenseFeatureValueElementType = GetDenseFeatureValueElementType(featureValueType),
                                    PropertyExpression = propertyExpression,
                                    Order = attr.Order,
                                    // CODE new Feature<T> { Namespace = ..., ... } 
                                    NewFeatureExpression = Expression.MemberInit(
                                       Expression.New(featureType),
                                       Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Name), Expression.Constant(name)),
                                       Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Enumerize), Expression.Constant(attr.Enumerize)),
                                       Expression.Bind(featureType.GetProperty("Value"), propertyExpression),
                                       Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Namespace), Expression.Constant(namespaceValue, typeof(string))),
                                       Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.FeatureGroup),
                                            featureGroup == null ? (Expression)Expression.Constant(null, typeof(char?)) :
                                            Expression.New((ConstructorInfo)ReflectionHelper.GetInfo((char v) => new char?(v)), Expression.Constant((char)featureGroup)))
                                       )
                                };

            // Recurse
            return localFeatures
                .Select(f =>
                {
                    var subFeatures = ExtractFeaturesCompiled(f.PropertyExpression, f.Namespace, f.FeatureGroup);
                    return subFeatures.Count == 0 ? new[] { f } : subFeatures;
                })
                .SelectMany(f => f)
                .ToList();
        }
    }
}
