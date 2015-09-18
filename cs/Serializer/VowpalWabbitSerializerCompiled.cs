// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializerCompiler.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Globalization;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
using VW.Serializer.Inspectors;
using VW.Serializer.Interfaces;
using VW.Serializer.Intermediate;
using VW.Serializer.Reflection;

namespace VW.Serializer
{
    /// <summary>
    /// Compiles a serializers for the given example user type.
    /// </summary>
    /// <typeparam name="TExample">The example user type.</typeparam>
    /// <typeparam name="TVisitor">The visitor to be used for serialization.</typeparam>
    /// <typeparam name="VowpalWabbitMarshallingContext">The resulting serialization type.</typeparam>
    /// <returns>A serializer for the given user example type.</returns>
    public sealed class VowpalWabbitSerializerCompiled<TExample>
    {
        internal class FeatureExpressionInternal
        {
            internal FeatureExpression Source;

            internal readonly List<ParameterExpression> Additional = new List<ParameterExpression>();

            internal MethodInfo FeaturizeMethod;

            internal Expression<Action> Visit;
        }

        private static readonly string SerializeMethodName = "Serialize";

        private static readonly ConstructorInfo ArgumentNullExceptionConstructorInfo = (ConstructorInfo)ReflectionHelper.GetInfo((ArgumentNullException t) => new ArgumentNullException(""));

        private readonly FeatureExpressionInternal[] allFeatures;
        private readonly List<Type> featurizerTypes;
        private readonly List<Expression> body;
        private readonly List<Expression> perExampleBody;
        private readonly List<ParameterExpression> variables;
        private readonly List<ParameterExpression> namespaceVariables;

        private ParameterExpression vwParameter;
        private ParameterExpression contextParameter;
        private ParameterExpression exampleParameter;
        private ParameterExpression labelParameter;

        //private ParameterExpression mainVisitorParameter;
        private readonly List<ParameterExpression> featurizers;
        private readonly List<ParameterExpression> metaFeatures;

        internal VowpalWabbitSerializerCompiled(IReadOnlyList<FeatureExpression> allFeatures, IReadOnlyList<Type> featurizerTypes)
        {
            if (allFeatures == null || allFeatures.Count == 0)
                throw new ArgumentException("allFeatures");

            Contract.EndContractBlock();

            this.allFeatures = allFeatures.Select(f => new FeatureExpressionInternal { Source = f }).ToArray();

            this.featurizerTypes = featurizerTypes == null ? new List<Type>() : new List<Type>(featurizerTypes);
            this.featurizerTypes.Add(typeof(VowpalWabbitDefaultMarshaller));

            var overrideFeaturizerTypes = allFeatures
                .Select(f => f.OverrideSerializeMethod)
                .Where(o => o != null)
                .Select(o => o.DeclaringType);

            this.body = new List<Expression>();
            this.perExampleBody = new List<Expression>();
            this.variables = new List<ParameterExpression>();
            this.namespaceVariables = new List<ParameterExpression>();
            this.featurizers = new List<ParameterExpression>();
            this.metaFeatures = new List<ParameterExpression>();

            this.CreateVisitors();

            this.ResolveFeatureMarshallingMethods();
            this.CreateParameters();

            // CODE MarshalLabel(...)
            this.CreateFeaturizerCall("MarshalLabel", this.contextParameter, this.labelParameter);

            this.CreateNamespacesAndFeatures();
            //this.CreateFeatures();

            // this.CreateFeatureVisits();

            this.CreateLambda();
            // this.CreateLambda();
            this.Compile();
        }

        public VowpalWabbitSerializer<TExample> Create(VowpalWabbit vw)
        {
            return new VowpalWabbitSerializer<TExample>(this, vw);
        }

        //private void CreateVisitor(ParameterExpression visitorParameter, params Expression[] constructorParameters)
        //{
        //    //var paramSubset = new List<Expression>(constructorParameters);

        //    //// search for different constructors
        //    //while (paramSubset.Count > 0)
        //    //{
        //    //    var visitorCtor = visitorParameter.Type.GetConstructor(paramSubset.Select(p => p.Type).ToArray());
        //    //    if (visitorCtor != null)
        //    //    {
        //    //        // CODE visitor = new TVisitor(vw)
        //    //        this.body.Add(Expression.Assign(visitorParameter, Expression.New(visitorCtor, paramSubset)));
        //    //        return;
        //    //    }

        //    //    paramSubset.RemoveAt(paramSubset.Count - 1);
        //    //}

        //    // CODE visitor = new TVisitor()
        //    this.body.Add(Expression.Assign(visitorParameter, Expression.New(visitorParameter.Type)));
        //}

        private void CreateVisitors()
        {
            foreach (var featurizerType in this.featurizerTypes)
            {
                var featurizer = Expression.Parameter(featurizerType, "featurizer_" + featurizerType.Name);

                this.featurizers.Add(featurizer);
                this.variables.Add(featurizer);

                this.body.Add(Expression.Assign(featurizer, Expression.New(featurizerType)));
            }
        }

        private void ResolveFeatureMarshallingMethods()
        {
            foreach (var feature in this.allFeatures)
            {
                feature.FeaturizeMethod = feature.Source.FindMethod(this.featurizerTypes);

                if (feature.FeaturizeMethod == null)
                {
                    // TODO: implement ToString
                    throw new ArgumentException("Unable to find featurize method for " + feature);
                }
            }
        }

        private Expression CreateFeature(FeatureExpression feature, Expression @namespace)
        {
            if (feature.Enumerize)
            {
                if (feature.FeatureType.IsEnum)
                {
                    var enumerizeFeatureType = typeof(EnumerizedFeature<>).MakeGenericType(feature.FeatureType);
                    var featureParameter = Expression.Parameter(enumerizeFeatureType);
                    var valueParameter = Expression.Parameter(feature.FeatureType);

                    var body = new List<Expression>();

                    var hashVariables = new List<ParameterExpression>();
                    foreach (var value in Enum.GetValues(feature.FeatureType))
                    {
                        var hashVar = Expression.Variable(typeof(uint));
                        hashVariables.Add(hashVar);

                        // CODE hashVar = feature.FeatureHashInternal(value);
                        body.Add(Expression.Assign(hashVar,
                            Expression.Call(featureParameter,
                                enumerizeFeatureType.GetMethod("FeatureHash"),
                                Expression.Constant(value))));
                    }

                    // expand the switch(value) { case enum1: return hash1; .... }
                    var hashSwitch = Expression.Switch(valueParameter,
                        Enum.GetValues(feature.FeatureType)
                            .OfType<Type>()
                            .Zip(hashVariables, (value, hash) => Expression.SwitchCase(hash, Expression.Constant(value)))
                            .ToArray());

                    // CODE return value => switch(value) { .... }
                    body.Add(Expression.Lambda(hashSwitch, valueParameter));

                    return Expression.New(
                            enumerizeFeatureType.GetConstructor(Type.EmptyTypes),
                            this.vwParameter,
                            @namespace,
                            Expression.Constant(feature.Name, typeof(string)),
                            Expression.Constant(feature.AddAnchor),
                            Expression.Lambda(Expression.Block(hashVariables, body), featureParameter));
                }
            }
            else if (InspectionHelper.IsNumericType(feature.FeatureType) ||
                InspectionHelper.IsNumericType(InspectionHelper.GetDenseFeatureValueElementType(feature.FeatureType)))
            {
                // CODE: new NumericFeature(vw, namespace, "Name", "AddAnchor");
                return CreateNew(
                        typeof(NumericFeature),
                        this.vwParameter,
                        @namespace,
                        Expression.Constant(feature.Name, typeof(string)),
                        Expression.Constant(feature.AddAnchor));
            }

            return CreateNew(
                typeof(Feature),
                Expression.Constant(feature.Name, typeof(string)),
                Expression.Constant(feature.AddAnchor));
        }

        private static Expression CreateNew(Type type, params Expression[] constructorParameters)
        {
            return Expression.New(
                type.GetConstructor(constructorParameters.Select(e => e.Type).ToArray()),
                constructorParameters);
        }

        private void CreateNamespacesAndFeatures()
        {
            // TODO: using(...)
            // CODE context = new VowpalWabbitMarshallingContext(vw)
            //this.perExampleBody.Add(Expression.Assign(this.contextParameter,
            //    CreateNew(
            //        typeof(VowpalWabbitMarshallingContext),
            //        this.vwParameter)));

            var featuresByNamespace = this.allFeatures.GroupBy(
                f => new { f.Source.Namespace, f.Source.FeatureGroup, f.Source.IsDense },
                f => f);

            foreach (var ns in featuresByNamespace)
            {
                // each feature can have 2 additional parameters (namespace + feature)
                // Visit(VowpalWabbit, CustomNamespace, CustomFeature)

                // create default namespace object
                var namespaceVariable = Expression.Variable(typeof(Namespace), "ns_" + ns.Key.FeatureGroup + ns.Key.Namespace);
                this.variables.Add(namespaceVariable);

                //var bindings = CreateNamespaceAndFeatureGroupBinding(feature.Namespace, feature.FeatureGroup);
                //this.body.Add(Expression.Assign(namespaceVariable, Expression.MemberInit(Expression.New(typeof(Namespace)), bindings)));

                // CODE ns = new Namespace(vw, name, featureGroup);
                this.body.Add(Expression.Assign(namespaceVariable,
                    CreateNew(
                        typeof(Namespace),
                        this.vwParameter,
                        Expression.Constant(ns.Key.Namespace, typeof(string)),
                        ns.Key.FeatureGroup == null ? (Expression)Expression.Constant(null, typeof(char?)) :
                         Expression.New((ConstructorInfo)ReflectionHelper.GetInfo((char v) => new char?(v)), Expression.Constant((char)ns.Key.FeatureGroup)))));

                // find all parameters types that have a ctor(Namespace ns)
                //var customNamespaceTypes = ns.SelectMany(f => f.FeaturizeMethod
                //                                        .GetParameters()
                //                                        .Where(p => typeof(ICustomNamespace).IsAssignableFrom(p.ParameterType)))
                //                             .Distinct();

                //var customNamespaceVariables = new List<ParameterExpression>();
                //foreach (var customNamespaceType in customNamespaceTypes)
                //{
                //    var customNamespaceVariable = Expression.Variable(customNamespaceType);
                //    customNamespaceVariables.Add(customNamespaceVariable);

                //    // CODE var ns = new CustomNamespaceType();
                //    this.body.Add(Expression.Assign(customNamespaceVariable, Expression.New(customNamespaceType)));

                //    // CODE ns.Initialize(vw, nsPlain)
                //    this.body.Add(Expression.Call(customNamespaceVariable,
                //        ReflectionHelper.GetInfo((ICustomNamespace ns) => ns.Initialize(null, null)),
                //        this.vwParameter,
                //        namespaceVariable));
                //}

                //this.namespaceVariables.AddRange(customNamespaceVariables);
                var featureVisits = new List<Expression>(ns.Count());

                foreach (var feature in ns.OrderBy(f => f.Source.Order))
	            {
                    //feature.Additional.Add(this.vwParameter);
                    //feature.Additional.Add(namespaceVariable);

                    var newFeature = feature.Source.FeatureExpressionFactory != null ?
                        feature.Source.FeatureExpressionFactory(this.vwParameter, namespaceVariable) :
                        this.CreateFeature(feature.Source, namespaceVariable);

                    var featureVariable = Expression.Variable(newFeature.Type, "feature_" + feature.Source.Name);
                    this.variables.Add(featureVariable);

                    // CODE var feature = new ...
                    this.body.Add(Expression.Assign(featureVariable, newFeature));

                    // TODO: optimize
                    var featurizer = this.featurizers.First(f => f.Type == feature.FeaturizeMethod.ReflectedType);

                    // this.vwParameter
                    featureVisits.Add(Expression.Call(featurizer, feature.FeaturizeMethod, this.contextParameter, namespaceVariable, featureVariable,
                        feature.Source.ValueExpressionFactory(this.exampleParameter)));
	            }

                var featureVisitLambda = Expression.Lambda(Expression.Block(featureVisits));

                // CODE: featurizer.MarshalNamespace(context, namespace, { ... })
                this.CreateFeaturizerCall("MarshalNamespace", this.contextParameter, namespaceVariable, featureVisitLambda);
            }
        }

        // TODO:
        //if (ns.Key.IsDense)
                //{
                //    // Dense namespace
                //    if (features.Count != 1)
                //    {
                //        throw new NotSupportedException("Only a single dense vector is supported per namespace");
                //    }

                //    var feature = features.First();

                // var namespaceType = typeof(NamespaceDense<>).MakeGenericType(feature.DenseFeatureValueElementType);

                // bindings.Add(Expression.Bind(namespaceType.GetProperty("DenseFeature"), feature.CreateFeatureExpression(this.valueParameter)));

                    // CODE namespace = new Namespace { ... };
                    //body.Add(Log());
                //}
                //else
                //{
                //    CreateSparseFeaturesVisits(ns.Key.Namespace, ns.Key.FeatureGroup, features);
                //}


        private void CreateFeaturizerCall(string methodName, params Expression[] parameters)
        {
            var parameterTypes = parameters.Select(p => p.Type).ToArray();
            foreach (var featurizer in this.featurizers)
	        {
                var method = featurizer.Type.GetMethod(methodName, parameterTypes);

                if (method != null)
                {
                    this.perExampleBody.Add(Expression.Call(featurizer, method, parameters));
                    return;
                }
	        }

            throw new ArgumentException("Unable to find MarshalNamespace(VowpalWabbitMarshallingContext, Namespace, Action) on any featurizer");
        }

        private void CreateLambda()
        {
            // CODE (TExample, Label) => { ... }
            this.body.Add(
                Expression.Lambda(
                    typeof(Action<VowpalWabbitMarshalContext, TExample, ILabel>),
                    //Expression.Assign(this.contextParameter, Expression.Constant(null,  typeof(VowpalWabbitMarshallingContext))),
                    Expression.Block(this.perExampleBody),
                    this.contextParameter, this.exampleParameter, this.labelParameter));

            // CODE return (vw) => { ... return (ex, label) => { ... } }
            // this.body.Add(Expression.Lambda(Expression.Block(new[] { this.contextParameter }, this.body), this.vwParameter));

            this.SourceExpression = Expression.Lambda<Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>>(
                Expression.Block(this.variables, this.body), this.vwParameter);
        }

        public Expression<Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>> SourceExpression
        {
            get;
            private set;
        }

        public Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>> Func
        {
            get;
            private set;
        }

        private void Compile()
        {
            var asmName = new AssemblyName("VowpalWabbitSerializer." + typeof(TExample).Name);
            var dynAsm = AppDomain.CurrentDomain.DefineDynamicAssembly(asmName, AssemblyBuilderAccess.RunAndSave);

            // Create a dynamic module and type
            //#if !DEBUG
            //var moduleBuilder = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule", asmName.Name + ".dll", true);
            //#else
            var moduleBuilder = dynAsm.DefineDynamicModule("VowpalWabbitSerializerModule");

            var typeBuilder = moduleBuilder.DefineType("VowpalWabbitSerializer" + Guid.NewGuid().ToString().Replace('-', '_'));

            // Create our method builder for this type builder
            var methodBuilder = typeBuilder.DefineMethod(
                SerializeMethodName,
                MethodAttributes.Public | MethodAttributes.Static,
                typeof(Action<VowpalWabbitMarshalContext, TExample, ILabel>),
                new[] { typeof(VowpalWabbit) });

            // compared to Compile this looks rather ugly, but there is a feature-bug
            // that adds a security check to every call of the Serialize method
            //#if !DEBUG
            //var debugInfoGenerator = DebugInfoGenerator.CreatePdbGenerator();
            //visit.CompileToMethod(methodBuilder, debugInfoGenerator);
            //#else
            this.SourceExpression.CompileToMethod(methodBuilder);
            //#endif

            var dynType = typeBuilder.CreateType();

            this.Func = (Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>)Delegate.CreateDelegate(
                typeof(Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>),
                dynType.GetMethod(SerializeMethodName));
        }

        /// <summary>
        /// define functions input parameter
        /// </summary>
        private void CreateParameters()
        {
            this.vwParameter = Expression.Parameter(typeof(VowpalWabbit), "vw");
            this.contextParameter = Expression.Parameter(typeof(VowpalWabbitMarshalContext), "context");
            this.exampleParameter = Expression.Parameter(typeof(TExample), "example");
            this.labelParameter = Expression.Parameter(typeof(ILabel), "label");

        }

        //private void CreateFeatures()
        //{
        //    foreach (var feature in this.allFeatures)
        //    {
        //        var featureVariable = Expression.Variable(feature.Source.VariableName);
        //        this.variables.Add(featureVariable);
        //        feature.Additional.Add(featureVariable);

        //        // CODE: var featurePlain = new Feature { ... };
        //        this.body.Add(Expression.Assign(featureVariable,
        //                Expression.MemberInit(
        //                            Expression.New(Feature),
        //                            Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Name), Expression.Constant(feature.Source.Name, typeof(string))),
        //                            Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Enumerize), Expression.Constant(feature.Source.Enumerize)),
        //                            Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.AddAnchor), Expression.Constant(feature.Source.AddAnchor)),
        //                            Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Namespace), Expression.Constant(feature.Source.Namespace, typeof(string))),
        //                            Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Source.FeatureGroup),
        //                                this.FeatureGroup == null ? (Expression)Expression.Constant(null, typeof(char?)) :
        //                                Expression.New((ConstructorInfo)ReflectionHelper.GetInfo((char v) => new char?(v)), Expression.Constant((char)feature.Source.FeatureGroup))))));

        //        var customFeatureTypes = feature.FeaturizeMethod.GetParameters().Where(pi => typeof(ICustomFeature).IsAssignableFrom(pi.ParameterType));
        //        foreach (var customFeatureType in customFeatureTypes)
        //        {
        //            var customFeatureVariable = Expression.Variable(customFeatureType);
        //            this.variables.Add(customFeatureVariable);
        //            feature.Additional.Add(customFeatureVariable);

        //            // CODE var customFeature = new CustomFeatureType();
        //            this.body.Add(Expression.Assign(customFeatureVariable, Expression.New(customFeatureType)));
        //            // CODE customFeature.Initialize(vw, featurePlain);
        //            this.body.Add(Expression.Call(customFeatureVariable,
        //                            ReflectionHelper.GetInfo((ICustomFeature f) => f.Initialize(null, null)), this.vwParameter, featureVariable));
        //        }
        //    }

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

            //var featuresByNamespace = allFeatures.GroupBy(f => new { f.Namespace, f.FeatureGroup, f.IsDense }, f => f);
            //foreach (var ns in featuresByNamespace)
            //{
            //    var features = ns.OrderBy(f => f.Order).ToList();

            //    if (ns.Key.IsDense)
            //    {
            //        CreateDenseFeatureVisits(features);
            //    }
            //    else
            //    {
            //        CreateSparseFeaturesVisits(ns.Key.Namespace, ns.Key.FeatureGroup, features);
            //    }
            //}
        //}

        //private void CreateLambda()
        //{
        //    // CODE return visitor.Visit(label, new[] { ns1, ns2, ... })
        //    //body.Add(Log());
        //    var visitWithLabelMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(ILabel), typeof(IVisitableNamespace[]) });
        //    this.body.Add(
        //        Expression.Call(
        //            this.mainVisitorParameter,
        //            visitWithLabelMethod,
        //            this.labelParameter,
        //            Expression.NewArrayInit(
        //                typeof(IVisitableNamespace),
        //                namespaceVariables.ToArray<Expression>())));

        //    // CODE (example, visitor) => { ... }
        //    this.ResultExpression = Expression.Lambda<Func<VowpalWabbit, TExample, ILabel, VowpalWabbitMarshallingContext>>(
        //        Expression.Block(this.variables.Union(this.namespaceVariables), this.body),
        //        this.vwParameter,
        //        this.exampleParamter,
        //        this.labelParameter);
        //}

        //private static List<MemberAssignment> CreateNamespaceAndFeatureGroupBinding(string @namespace, char? featureGroup)
        //{
        //    var baseNamespaceInits = new List<MemberAssignment>
        //        {
        //            Expression.Bind(
        //                ReflectionHelper.GetInfo((Namespace n) => n.Name),
        //                Expression.Constant(@namespace, typeof(string)))
        //        };

        //    if (featureGroup != null)
        //    {
        //        baseNamespaceInits.Add(
        //            Expression.Bind(
        //                ReflectionHelper.GetInfo((Namespace n) => n.FeatureGroup),
        //                Expression.Convert(Expression.Constant((char)featureGroup), typeof(char?))));
        //    }

        //    return baseNamespaceInits;
        //}

        //private void CreateDenseFeatureVisits(List<FeatureExpression> features)
        //{
        //    // TODO: INSERT ns lookup variable

        //    // find the first featurizer that supports this feature type
        //    foreach (var featurizer in this.featurizers)
        //    {
        //        if (this.CreateDenseFeatureVisit(featurizer, namespaceVariable))
        //        {
        //            return;
        //        }
        //    }

        //    this.CreateDenseFeatureVisit(this.mainVisitorParameter, namespaceVariable);
        //}

        //private bool CreateDenseFeatureVisit(ParameterExpression visitor, ParameterExpression namespaceVariable)
        //{
        //    var method = ReflectionHelper.FindMethod(visitor.Type, "Visit", namespaceVariable.Type);
        //    if (method == null)
        //    {
        //        return false;
        //    }

        //    // CODE namespace.Visit = () => visitor.Visit(namespace)
        //    //body.Add(Log());
        //    this.body.Add(Expression.Assign(
        //            Expression.Property(namespaceVariable, namespaceVariable.Type.GetProperty("Visit")),
        //            Expression.Lambda<Action>(
        //                Expression.Call(
        //                    visitor,
        //                    method,
        //                    namespaceVariable))));

        //    return true;
        //}

        //private void CreateSparseFeaturesVisits(string @namespace, char? featureGroup, List<FeatureExpression> features)
        //{
        //    // Sparse namespace
        //    var featureVariables = new List<ParameterExpression>();

        //    foreach (var feature in features)
        //    {
        //        var featureVariable = Expression.Parameter(feature.IntermediateFeatureType, feature.VariableName);

        //        variables.Add(featureVariable);
        //        featureVariables.Add(featureVariable);

        //        // CODE feature = new Feature<float> { ... };
        //        //body.Add(Log());
        //        this.body.Add(Expression.Assign(featureVariable, feature.CreateFeatureExpression(this.exampleParamter)));
        //    }

        //    // features belong to the same namespace
        //    var bindings = CreateNamespaceAndFeatureGroupBinding(@namespace, featureGroup);
        //    bindings.Add(Expression.Bind(
        //                        ReflectionHelper.GetInfo((NamespaceSparse n) => n.Features),
        //                        Expression.NewArrayInit(typeof(IVisitableFeature), featureVariables)));

        //    // CODE new NamespaceSparse { Features = new[] { feature1, feature2, ... } }
        //    var namespaceSparse = Expression.MemberInit(Expression.New(typeof(NamespaceSparse)), bindings);

        //    var namespaceVariable = Expression.Variable(typeof(NamespaceSparse), "namespaceSparse");
        //    namespaceVariables.Add(namespaceVariable);

        //    // CODE namespace = new NamespaceSparse { ... }
        //    //body.Add(Log());
        //    this.body.Add(Expression.Assign(namespaceVariable, namespaceSparse));

        //    // loop unrolling to have dispatch onto the correct Visit<T>
        //    for (var i = 0; i < features.Count; i++)
        //    {
        //        var feature = features[i];
        //        var featureVariable = featureVariables[i];

        //        Expression visitFeatureCall = this.CreateFeatureVisitFromOverride(feature, featureVariable);

        //        if (visitFeatureCall == null)
        //        {
        //            foreach (var featurizer in this.featurizers)
        //            {
        //                visitFeatureCall = this.CreateFeatureVisit(feature, featurizer, featureVariable);
        //                if (visitFeatureCall != null)
        //                {
        //                    break;
        //                }
        //            }
        //        }

        //        if (visitFeatureCall == null)
        //        {
        //            visitFeatureCall = CreateFeatureVisit(feature, this.mainVisitorParameter, featureVariable);
        //        }

        //        if (visitFeatureCall == null)
        //        {
        //            throw new NotSupportedException("Feature type is not supported: " + featureVariable.Type);
        //        }

        //        // CODE feature.Visit = visitor.Visit;
        //        //body.Add(Log());
        //        this.body.Add(
        //            Expression.Assign(
        //                Expression.Property(featureVariable, featureVariable.Type.GetProperty("Visit")),
        //                Expression.Lambda<Action>(visitFeatureCall)));
        //    }

        //    // CODE namespace.Visit = () => { visitor.Visit(namespace); });
        //    //body.Add(Log());
        //    // return (MethodInfo)ReflectionHelper.GetInfo((IVowpalWabbitVisitor<VowpalWabbitMarshallingContext> e) => e.Visit((NamespaceSparse)null));
        //    var visitMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(INamespaceSparse) });
        //    this.body.Add(
        //        Expression.Assign(
        //            Expression.Property(
        //                namespaceVariable,
        //                (PropertyInfo)ReflectionHelper.GetInfo((NamespaceSparse n) => n.Visit)),
        //            Expression.Lambda<Action>(
        //                Expression.Call(
        //                    this.mainVisitorParameter,
        //                    visitMethod,
        //                    namespaceVariable))));
        //}

        //private Expression CreateFeatureVisitFromOverride(FeatureExpression feature, ParameterExpression featureVariable)
        //{
        //    var method = feature.OverrideSerializeMethod;
        //    if (method == null)
        //    {
        //        return null;
        //    }

        //    var overrideFeaturizer = this.featurizers.FirstOrDefault(f => f.Type == method.DeclaringType);
        //    if (overrideFeaturizer == null)
        //    {
        //        throw new ArgumentException("Featurizer missing");
        //    }

        //    return CreateFeatureVisit(overrideFeaturizer, featureVariable, method);
        //}

        //private Expression CreateFeatureVisit(FeatureExpression feature, ParameterExpression visitorParameter, ParameterExpression featureVariable)
        //{
        //    var method = ReflectionHelper.FindMethod(visitorParameter.Type, feature.Enumerize ? "VisitEnumerize" : "Visit", featureVariable.Type);
        //    if (method == null)
        //    {
        //        return null;
        //    }

        //    return CreateFeatureVisit(visitorParameter, featureVariable, method);
        //}

        //private static Expression CreateFeatureVisit(ParameterExpression visitorParameter, ParameterExpression featureVariable, MethodInfo method)
        //{
        //    // CODE: visitor.Visit(feature1);
        //    Expression visitFeatureCall = Expression.Call(
        //                visitorParameter,
        //                method,
        //                featureVariable);

        //    var featureValue = Expression.Property(featureVariable, "Value");
        //    if (!featureValue.Type.IsValueType || (featureValue.Type.IsGenericType && featureValue.Type.GetGenericTypeDefinition() == typeof(Nullable<>)))
        //    {
        //        // CODE feature1.Value != null ? visitor.Visit(feature1) : default(TFeatureResult);
        //        visitFeatureCall = Expression.IfThen(
        //                test: Expression.NotEqual(featureValue, Expression.Constant(null)),
        //                ifTrue: visitFeatureCall);
        //    }

        //    return visitFeatureCall;
        //}

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
    }
}
