using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Tasks;
using VW.Labels;
using VW.Reflection;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// Compiles a serializers for the given example user type.
    /// </summary>
    /// <typeparam name="TExample">The example user type.</typeparam>
    /// <returns>A serializer for the given user example type.</returns>
    public sealed class VowpalWabbitSingleExampleSerializerCompiler<TExample> : IVowpalWabbitSerializerCompiler<TExample>
    {
                /// <summary>
        /// Internal structure collecting all itmes required to marshal a single feature.
        /// </summary>
        [DebuggerDisplay("FeatureExpressionInternal(Source={Source}, MarshalMethod={MarshalMethod})")]
        internal sealed class FeatureExpressionInternal
        {
            /// <summary>
            /// The supplied feature expression.
            /// </summary>
            internal FeatureExpression Source;

            /// <summary>
            /// The resolved mrarshalling method.
            /// </summary>
            internal MarshalMethod MarshalMethod;
        }

        /// <summary>
        /// Describes the actual marshalling method and the feature type (e.g. <see cref="PreHashedFeature"/>).
        /// </summary>
        internal sealed class MarshalMethod
        {
            /// <summary>
            /// The actual marshalling method.
            /// </summary>
            internal MethodInfo Method;

            /// <summary>
            /// The feature type (e.g. <see cref="PreHashedFeature"/>).
            /// </summary>
            internal Type MetaFeatureType;

            /// <summary>
            /// True if the method can marshal a full namespace.
            /// </summary>
            internal bool IsNamespace;
        }

        /// <summary>
        /// All discovered features.
        /// </summary>
        private FeatureExpressionInternal[] allFeatures;

        private readonly Schema schema;

        /// <summary>
        /// Ordered list of featurizer types. Marshalling methods are resolved in order of this list.
        /// <see cref="VowpalWabbitDefaultMarshaller"/> is added last as default.
        /// </summary>
        private readonly List<Type> marshallerTypes;

        /// <summary>
        /// The main body of the serializer holding preemptive calcutions (e.g. <see cref="PreHashedFeature"/>.
        /// </summary>
        private readonly List<Expression> body;

        /// <summary>
        /// The body executed for example.
        /// </summary>
        private readonly List<Expression> perExampleBody;

        /// <summary>
        /// Local variables.
        /// </summary>
        private readonly List<ParameterExpression> variables;

        /// <summary>
        /// Local variables holding namespaces.
        /// </summary>
        private readonly List<ParameterExpression> namespaceVariables;

        /// <summary>
        /// The parameter of the main lambda to <see cref="VowpalWabbit"/>.
        /// </summary>
        private ParameterExpression vwParameter;

        /// <summary>
        /// The parameter of the main lambda to <see cref="VowpalWabbitMarshalContext"/>.
        /// </summary>
        private ParameterExpression contextParameter;

        /// <summary>
        /// The parameter of the per example lambda to <see cref="VowpalWabbitExample"/>.
        /// </summary>
        private ParameterExpression exampleParameter;

        /// <summary>
        /// The parameter of the per example lambda to <see cref="ILabel"/>
        /// </summary>
        private ParameterExpression labelParameter;

        /// <summary>
        /// The list of featurizers.
        /// </summary>
        private readonly List<ParameterExpression> marshallers;

        /// <summary>
        /// The list of meta features such as <see cref="PreHashedFeature"/>.
        /// </summary>
        private readonly List<ParameterExpression> metaFeatures;

        /// <summary>
        /// If true, VowpalWabbit string generation is disabled.
        /// </summary>
        private readonly bool disableStringExampleGeneration;

        internal VowpalWabbitSingleExampleSerializerCompiler(Schema schema, IReadOnlyList<Type> featurizerTypes, bool disableStringExampleGeneration)
        {
            if (schema == null || schema.Features.Count == 0)
                throw new ArgumentException("schema");
            Contract.EndContractBlock();

            this.schema = schema;
            this.disableStringExampleGeneration = disableStringExampleGeneration;

            this.allFeatures = schema.Features.Select(f => new FeatureExpressionInternal { Source = f }).ToArray();

            // collect the types used for marshalling
            this.marshallerTypes = featurizerTypes == null ? new List<Type>() : new List<Type>(featurizerTypes);

            // extract types from overrides defined on particular features
            var overrideFeaturizerTypes = schema.Features
                .Select(f => f.OverrideSerializeMethod)
                .Where(o => o != null)
                .Select(o => o.DeclaringType);
            this.marshallerTypes.AddRange(overrideFeaturizerTypes);

            // add as last
            this.marshallerTypes.Add(typeof(VowpalWabbitDefaultMarshaller));

            this.body = new List<Expression>();
            this.perExampleBody = new List<Expression>();
            this.variables = new List<ParameterExpression>();
            this.namespaceVariables = new List<ParameterExpression>();
            this.marshallers = new List<ParameterExpression>();
            this.metaFeatures = new List<ParameterExpression>();

            this.CreateMarshallers();

            this.ResolveFeatureMarshallingMethods();
            this.CreateParameters();

            this.CreateLabel();

            this.CreateNamespacesAndFeatures();

            this.CreateLambdas();

            this.Func = (Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>)this.SourceExpression.CompileToFunc();
        }

        internal bool DisableStringExampleGeneration { get { return this.disableStringExampleGeneration; } }

        /// <summary>
        /// Creates a bound serializers.
        /// </summary>
        /// <param name="vw">The vw instance to bind to.</param>
        /// <returns></returns>
        IVowpalWabbitSerializer<TExample> IVowpalWabbitSerializerCompiler<TExample>.Create(VowpalWabbit vw)
        {
            return this.Create(vw);
        }

        /// <summary>
        /// Creates a serializer for <typeparamref name="TExample"/> bound to <paramref name="vw"/>.
        /// </summary>
        /// <param name="vw">The VW native instance examples will be assocated with.</param>
        /// <returns>A serializer for <typeparamref name="TExample"/>.</returns>
        public VowpalWabbitSingleExampleSerializer<TExample> Create(VowpalWabbit vw)
        {
            return new VowpalWabbitSingleExampleSerializer<TExample>(this, vw);
        }

        private void CreateLabel()
        {
            // CODE if (labelParameter == null)
            this.perExampleBody.Add(Expression.IfThen(
                Expression.NotEqual(this.labelParameter, Expression.Constant(null, typeof(ILabel))),
                this.CreateMarshallerCall("MarshalLabel", this.contextParameter, this.labelParameter)));

            var label = this.schema.Label;
            if (label != null)
            {
                // CODE condition1 && condition2 && condition3 ...
                var condition = label.ValueValidExpressionFactories
                    .Skip(1)
                    .Aggregate(
                        label.ValueValidExpressionFactories.First()(this.exampleParameter),
                        (cond, factory) => Expression.AndAlso(cond, factory(this.exampleParameter)));

                // CODE if (labelParameter != null && example.Label != null && ...)
                this.perExampleBody.Add(
                    Expression.IfThen(
                        Expression.AndAlso(
                            Expression.Equal(this.labelParameter, Expression.Constant(null, typeof(ILabel))),
                            condition),
                        // CODE MarshalLabel(context, example.Label)
                        this.CreateMarshallerCall("MarshalLabel",
                            this.contextParameter,
                            label.ValueExpressionFactory(this.exampleParameter))));
            }
        }

        /// <summary>
        /// Define variables and instantiate marshaller types.
        /// </summary>
        private void CreateMarshallers()
        {
            foreach (var marshallerType in this.marshallerTypes)
            {
                var marshaller = Expression.Parameter(marshallerType, "marshaller_" + marshallerType.Name);

                this.marshallers.Add(marshaller);
                this.variables.Add(marshaller);

                // CODE new FeaturizerType(disableStringExampleGeneration)
                var newExpr = CreateNew(marshallerType, Expression.Constant(disableStringExampleGeneration));
                if (newExpr == null)
                {
                    // CODE new MarshallerType()
                    newExpr = Expression.New(marshallerType);
                }

                // marshaller = new ...
                this.body.Add(Expression.Assign(marshaller, newExpr));
            }
        }

        private MarshalMethod ResolveFeatureMarshalMethod(FeatureExpression feature)
        {
            if (feature.OverrideSerializeMethod != null)
            {
                return new MarshalMethod
                {
                    Method = feature.OverrideSerializeMethod,
                    MetaFeatureType = feature.OverrideSerializeMethod.GetParameters().Select(p => p.ParameterType).First(t => typeof(Feature).IsAssignableFrom(t))
                };
            }

            string methodName;
            Type[] metaFeatureTypeCandidates;

            if (feature.FeatureType == typeof(string))
            {
                switch(feature.StringProcessing)
                {
                    case StringProcessing.Escape:
                        methodName = "MarshalFeatureStringEscape";
                        break;
                    case StringProcessing.EscapeAndIncludeName:
                        methodName = "MarshalFeatureStringEscapeAndIncludeName";
                        break;
                    case StringProcessing.Split:
                        methodName = "MarshalFeatureStringSplit";
                        break;
                    default:
                        throw new ArgumentException("feature.StringProcessing is not supported: " + feature.StringProcessing);
                }
                metaFeatureTypeCandidates = new [] { typeof(Feature) };
            }
            else if (feature.FeatureType.IsEnum)
            {
                methodName = "MarshalEnumFeature";
                metaFeatureTypeCandidates = new [] { typeof(EnumerizedFeature<>).MakeGenericType(feature.FeatureType) };
            }
            else if (feature.Enumerize)
            {
                methodName = "MarshalEnumerizeFeature";
                metaFeatureTypeCandidates = new [] { typeof(Feature) };
            }
            else
            {
                // probe for PreHashedFeature marshal method, than fallback
                methodName = "MarshalFeature";
                metaFeatureTypeCandidates = new [] { typeof(PreHashedFeature), typeof(Feature) };
            }

            // remove Nullable<> from feature type
            var featureType = feature.FeatureType;
            if(featureType.IsGenericType &&
               featureType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                featureType = featureType.GetGenericArguments()[0];
            }

            var method = ResolveFeatureMarshalMethod("MarshalNamespace", metaFeatureTypeCandidates, featureType, isNamespace: true);
            if (method == null)
                method = ResolveFeatureMarshalMethod(methodName, metaFeatureTypeCandidates, featureType, isNamespace: false);

            return method;
        }

        private MarshalMethod ResolveFeatureMarshalMethod(string methodName, Type[] metaFeatureTypeCandidates, Type featureType, bool isNamespace)
        {
            foreach (var metaFeatureType in metaFeatureTypeCandidates)
            {
                // find visitor.<methodname>(VowpalWabbitMarshallingContext context, Namespace ns, <PreHashedFeature|Feature> feature, <valueType> value)
                var method = this.marshallerTypes
                    .Select(visitor => ReflectionHelper.FindMethod(
                            visitor,
                            methodName,
                            typeof(VowpalWabbitMarshalContext),
                            typeof(Namespace),
                            metaFeatureType,
                            featureType))
                    .FirstOrDefault(m => m != null);

                if (method != null)
                    return new MarshalMethod
                    {
                        Method = method,
                        MetaFeatureType = metaFeatureType,
                        IsNamespace = isNamespace
                    };
            }

            return null;
        }

        private bool ContainsAncestor(FeatureExpressionInternal candidate, List<FeatureExpressionInternal> validFeature)
        {
            if (candidate.Source.Parent == null)
                return false;

            if (validFeature.Any(valid => object.ReferenceEquals(valid.Source, candidate.Source.Parent)))
                return true;

            var parent = this.allFeatures.First(f => object.ReferenceEquals(f.Source, candidate.Source.Parent));
            return ContainsAncestor(parent, validFeature);
        }

        /// <summary>
        /// Resolve methods for each feature base on feature type and configuration.
        /// </summary>
        private void ResolveFeatureMarshallingMethods()
        {
            var validFeature = new List<FeatureExpressionInternal>(this.allFeatures.Length);
            foreach (var feature in this.allFeatures)
            {
                // skip any feature which parent feature is already resolved
                if (ContainsAncestor(feature, validFeature))
                    continue;

                feature.MarshalMethod = this.ResolveFeatureMarshalMethod(feature.Source);

                if (feature.MarshalMethod != null)
                    validFeature.Add(feature);
            }

            this.allFeatures = validFeature.ToArray();
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

        /// <summary>
        /// Instantiate the meta information object such as <see cref="PreHashedFeature"/>
        /// for a given feature.
        /// </summary>
        /// <param name="featureInternal">The feature.</param>
        /// <param name="namespace">The namespace.</param>
        /// <returns>The "new" expression for the meta information object.</returns>
        private Expression CreateFeature(FeatureExpressionInternal featureInternal, Expression @namespace)
        {
            FeatureExpression feature = featureInternal.Source;
            var metaFeatureType = featureInternal.MarshalMethod.MetaFeatureType;

            if (metaFeatureType.IsGenericType && metaFeatureType.GetGenericTypeDefinition() == typeof(EnumerizedFeature<>))
            {
                // preemptively calculate all hashes for each enum value
                var featureParameter = Expression.Parameter(metaFeatureType);
                var valueParameter = Expression.Parameter(feature.FeatureType);

                var body = new List<Expression>();

                var hashVariables = new List<ParameterExpression>();
                foreach (var value in Enum.GetValues(feature.FeatureType))
                {
                    var hashVar = Expression.Variable(typeof(UInt64));
                    hashVariables.Add(hashVar);

                    // CODE hashVar = feature.FeatureHashInternal(value);
                    body.Add(Expression.Assign(hashVar,
                        Expression.Call(featureParameter,
                            metaFeatureType.GetMethod("FeatureHashInternal"),
                            Expression.Constant(value))));
                }

                var cases = Enum.GetValues(feature.FeatureType)
                        .Cast<object>()
                        .Zip(hashVariables, (value, hash) => Expression.SwitchCase(
                            hash,
                            Expression.Constant(value, feature.FeatureType)))
                        .ToArray();

                // expand the switch(value) { case enum1: return hash1; .... }
                var hashSwitch = Expression.Switch(valueParameter,
                    Expression.Block(Expression.Throw(Expression.New(typeof(NotSupportedException))), Expression.Constant((UInt64)0, typeof(UInt64))),
                    cases);

                // CODE return value => switch(value) { .... }
                body.Add(Expression.Lambda(hashSwitch, valueParameter));

                return CreateNew(
                        metaFeatureType,
                        this.vwParameter,
                        @namespace,
                        Expression.Constant(feature.Name, typeof(string)),
                        Expression.Constant(feature.AddAnchor),
                        Expression.Constant(feature.Dictify),
                        Expression.Lambda(Expression.Block(hashVariables, body), featureParameter));
            }
            else if (metaFeatureType == typeof(PreHashedFeature))
            {
                // CODE new PreHashedFeature(vw, namespace, "Name", "AddAnchor");
                return CreateNew(
                        typeof(PreHashedFeature),
                        this.vwParameter,
                        @namespace,
                        Expression.Constant(feature.Name, typeof(string)),
                        Expression.Constant(feature.AddAnchor),
                        Expression.Constant(feature.Dictify));
            }
            else
                // CODE new Feature("Name", ...)
                return CreateNew(
                    metaFeatureType,
                    Expression.Constant(feature.Name, typeof(string)),
                    Expression.Constant(feature.AddAnchor),
                    Expression.Constant(feature.Dictify));
        }

        /// <summary>
        /// Helper to create the "new" expression using a matching constructor.
        /// </summary>
        /// <param name="type">The type of the new object.</param>
        /// <param name="constructorParameters">The actual parameters for the constructor.</param>
        /// <returns>The "new" expression bound to <paramref name="constructorParameters"/>.</returns>
        private static Expression CreateNew(Type type, params Expression[] constructorParameters)
        {
            var ctor = type.GetConstructor(constructorParameters.Select(e => e.Type).ToArray());
            if (ctor == null)
                return null;

            return Expression.New(ctor, constructorParameters);
        }

        private void CreateNamespacesAndFeatures()
        {
            var featuresByNamespace = this.allFeatures
                .GroupBy(
                    f => new { f.Source.Namespace, f.Source.FeatureGroup },
                    f => f);

            foreach (var ns in featuresByNamespace)
            {
                // each feature can have 2 additional parameters (namespace + feature)
                // Visit(VowpalWabbit, CustomNamespace, CustomFeature)

                // create default namespace object
                var namespaceVariable = Expression.Variable(typeof(Namespace), "ns_" + ns.Key.FeatureGroup + ns.Key.Namespace);
                this.variables.Add(namespaceVariable);

                // CODE ns = new Namespace(vw, name, featureGroup);
                this.body.Add(Expression.Assign(namespaceVariable,
                    CreateNew(
                        typeof(Namespace),
                        this.vwParameter,
                        Expression.Constant(ns.Key.Namespace, typeof(string)),
                        ns.Key.FeatureGroup == null ? (Expression)Expression.Constant(null, typeof(char?)) :
                         Expression.New((ConstructorInfo)ReflectionHelper.GetInfo((char v) => new char?(v)), Expression.Constant((char)ns.Key.FeatureGroup)))));

                var fullNamespaceCalls = new List<Expression>();

                var featureVisits = new List<Expression>(ns.Count());
                foreach (var feature in ns.OrderBy(f => f.Source.Order))
	            {
                    var newFeature = feature.Source.FeatureExpressionFactory != null ?
                        feature.Source.FeatureExpressionFactory(this.vwParameter, namespaceVariable) :
                        this.CreateFeature(feature, namespaceVariable);

                    var featureVariable = Expression.Variable(newFeature.Type, "feature_" + feature.Source.Name);
                    this.variables.Add(featureVariable);

                    // CODE var feature = new ...
                    this.body.Add(Expression.Assign(featureVariable, newFeature));

                    // TODO: optimize
                    var marshaller = this.marshallers.First(f => f.Type == feature.MarshalMethod.Method.ReflectedType);

                    var valueVariable = feature.Source.ValueExpressionFactory(this.exampleParameter);
                    Expression featureVisit;
                    if (feature.Source.IsNullable)
                    {
                        // if (value != null) featurizer.MarshalXXX(vw, context, ns, feature, (FeatureType)value);
                        featureVisit = Expression.IfThen(
                            Expression.NotEqual(valueVariable, Expression.Constant(null)),
                                Expression.Call(
                                     marshaller,
                                     feature.MarshalMethod.Method,
                                     this.contextParameter,
                                     namespaceVariable,
                                     featureVariable,
                                     Expression.Convert(valueVariable, feature.Source.FeatureType)));
                    }
                    else
                    {
                        // featurizer.MarshalXXX(vw, context, ns, feature, value);
                        featureVisit = Expression.Call(
                            marshaller,
                            feature.MarshalMethod.Method,
                            this.contextParameter,
                            namespaceVariable,
                            featureVariable,
                            valueVariable);
                    }

                    if (feature.Source.ValueValidExpressionFactories != null && feature.Source.ValueValidExpressionFactories.Count > 0)
                    {
                        // CODE condition1 && condition2 && condition3 ...
                        var condition = feature.Source.ValueValidExpressionFactories
                            .Skip(1)
                            .Aggregate(
                                feature.Source.ValueValidExpressionFactories.First()(this.exampleParameter),
                                (cond, factory) => Expression.AndAlso(cond, factory(this.exampleParameter)));

                        featureVisit = Expression.IfThen(condition, featureVisit);
                    }

                    if (feature.MarshalMethod.IsNamespace)
                        this.perExampleBody.Add(featureVisit);
                    else
                        featureVisits.Add(featureVisit);
	            }

                if (featureVisits.Count > 0)
                {
                    var featureVisitLambda = Expression.Lambda(Expression.Block(featureVisits));

                    // CODE: featurizer.MarshalNamespace(context, namespace, { ... })
                    this.perExampleBody.Add(this.CreateMarshallerCall("MarshalNamespace", this.contextParameter, namespaceVariable, featureVisitLambda));
                }
            }
        }

        /// <summary>
        /// Create the invocation expression of a marshalling method.
        /// </summary>
        /// <param name="methodName">The marshalling method to invoke.</param>
        /// <param name="parameters">The parameters for this method.</param>
        private MethodCallExpression CreateMarshallerCall(string methodName, params Expression[] parameters)
        {
            var parameterTypes = parameters.Select(p => p.Type).ToArray();
            foreach (var marshaller in this.marshallers)
	        {
                var method = marshaller.Type.GetMethod(methodName, parameterTypes);

                if (method != null)
                {
                    return Expression.Call(marshaller, method, parameters);
                }
	        }

            throw new ArgumentException("Unable to find MarshalNamespace(VowpalWabbitMarshallingContext, Namespace, Action) on any featurizer");
        }

        /// <summary>
        /// Creates the main lambda and the per example lambda.
        /// </summary>
        private void CreateLambdas()
        {
            // CODE (TExample, Label) => { ... }
            this.body.Add(
                Expression.Lambda(
                    typeof(Action<VowpalWabbitMarshalContext, TExample, ILabel>),
                    Expression.Block(this.perExampleBody),
                    this.contextParameter, this.exampleParameter, this.labelParameter));

            // CODE return (vw) => { ... return (ex, label) => { ... } }
            this.SourceExpression = Expression.Lambda<Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>>(
                Expression.Block(this.variables, this.body), this.vwParameter);
        }

        /// <summary>
        /// The source expression tree <seealso cref="Func"/> is built from.
        /// </summary>
        public Expression<Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>>> SourceExpression
        {
            get;
            private set;
        }

        /// <summary>
        /// The closure used for serialization.
        /// </summary>
        public Func<VowpalWabbit, Action<VowpalWabbitMarshalContext, TExample, ILabel>> Func
        {
            get;
            private set;
        }
    }
}
