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

            var overrideFeaturizerTypes = allFeatures
                .Select(f => f.OverrideSerializeMethod)
                .Where(o => o != null)
                .Select(o => o.DeclaringType);
            this.featurizerTypes.AddRange(overrideFeaturizerTypes);

            // add as last
            this.featurizerTypes.Add(typeof(VowpalWabbitDefaultMarshaller));

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

            this.CreateLambda();
            // this.CreateLambda();
            this.Compile();
        }

        public VowpalWabbitSerializer<TExample> Create(VowpalWabbit vw)
        {
            return new VowpalWabbitSerializer<TExample>(this, vw);
        }

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

        private MethodInfo ResolveFeatureMarshalMethod(FeatureExpression feature)
        {
            if (feature.OverrideSerializeMethod != null)
            {
                return feature.OverrideSerializeMethod;
            }

            string methodName;
            Type metaFeatureType;

            if (feature.FeatureType.IsEnum)
            {
                methodName = "MarshalEnumFeature";
                metaFeatureType = typeof(EnumerizedFeature<>).MakeGenericType(feature.FeatureType);
            }
            else if (feature.Enumerize)
            {
                methodName = "MarshalEnumerizeFeature";
                metaFeatureType = typeof(Feature);
            }
            else if (feature.IsNumeric)
            {
                methodName = "MarshalFeature";
                metaFeatureType = typeof(NumericFeature);
            }
            else
            {
                methodName = "MarshalFeature";
                metaFeatureType = typeof(Feature);
            }

            // find visitor.MarshalFeature(VowpalWabbitMarshallingContext context, Namespace ns, <NumericFeature|Feature> feature, <valueType> value)
            var method = this.featurizerTypes.Select(visitor =>
                ReflectionHelper.FindMethod(
                        visitor,
                        methodName,
                        typeof(VowpalWabbitMarshalContext),
                        typeof(Namespace),
                        metaFeatureType,
                        feature.FeatureType))
                .FirstOrDefault(m => m != null);

            if (method == null)
            {
                // TODO: implement ToString
                throw new ArgumentException("Unable to find featurize method for " + feature);
            }

            return method;
        }

        private void ResolveFeatureMarshallingMethods()
        {
            foreach (var feature in this.allFeatures)
            {
                feature.FeaturizeMethod = this.ResolveFeatureMarshalMethod(feature.Source);
            }
        }

        private Expression CreateFeature(FeatureExpression feature, Expression @namespace)
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
                            enumerizeFeatureType.GetMethod("FeatureHashInternal"),
                            Expression.Constant(value))));
                }

                var cases =  Enum.GetValues(feature.FeatureType)
                        .Cast<object>()
                        .Zip(hashVariables, (value, hash) => Expression.SwitchCase(
                            hash,
                            Expression.Constant(value, feature.FeatureType)))
                        .ToArray();

                // expand the switch(value) { case enum1: return hash1; .... }
                var hashSwitch = Expression.Switch(valueParameter,
                    Expression.Block(Expression.Throw(Expression.New(typeof(NotSupportedException))), Expression.Constant((uint)0, typeof(uint))),
                    cases);

                // CODE return value => switch(value) { .... }
                body.Add(Expression.Lambda(hashSwitch, valueParameter));

                return CreateNew(
                        enumerizeFeatureType,
                        this.vwParameter,
                        @namespace,
                        Expression.Constant(feature.Name, typeof(string)),
                        Expression.Constant(feature.AddAnchor),
                        Expression.Lambda(Expression.Block(hashVariables, body), featureParameter));
            }
            else if (!feature.Enumerize && feature.IsNumeric)
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
            var featuresByNamespace = this.allFeatures.GroupBy(
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

                var featureVisits = new List<Expression>(ns.Count());
                foreach (var feature in ns.OrderBy(f => f.Source.Order))
	            {
                    var newFeature = feature.Source.FeatureExpressionFactory != null ?
                        feature.Source.FeatureExpressionFactory(this.vwParameter, namespaceVariable) :
                        this.CreateFeature(feature.Source, namespaceVariable);

                    var featureVariable = Expression.Variable(newFeature.Type, "feature_" + feature.Source.Name);
                    this.variables.Add(featureVariable);

                    // CODE var feature = new ...
                    this.body.Add(Expression.Assign(featureVariable, newFeature));

                    // TODO: optimize
                    var featurizer = this.featurizers.First(f => f.Type == feature.FeaturizeMethod.ReflectedType);

                    var valueVariable = feature.Source.ValueExpressionFactory(this.exampleParameter);

                    if (feature.Source.IsNullable)
                    {
                        // if (value != null) featurzier.MarshalXXX(vw, context, ns, feature, (FeatureType)value);
                        featureVisits.Add(Expression.IfThen(
                            Expression.NotEqual(valueVariable, Expression.Constant(null)),
                                Expression.Call(
                                     featurizer,
                                     feature.FeaturizeMethod,
                                     this.contextParameter,
                                     namespaceVariable,
                                     featureVariable,
                                     Expression.Convert(valueVariable, feature.Source.FeatureType))));
                    }
                    else
                    {
                        // featurzier.MarshalXXX(vw, context, ns, feature, value);
                        featureVisits.Add(Expression.Call(
                            featurizer,
                            feature.FeaturizeMethod,
                            this.contextParameter,
                            namespaceVariable,
                            featureVariable,
                            valueVariable));
                    }
	            }

                var featureVisitLambda = Expression.Lambda(Expression.Block(featureVisits));

                // CODE: featurizer.MarshalNamespace(context, namespace, { ... })
                this.CreateFeaturizerCall("MarshalNamespace", this.contextParameter, namespaceVariable, featureVisitLambda);
            }
        }

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
                    Expression.Block(this.perExampleBody),
                    this.contextParameter, this.exampleParameter, this.labelParameter));

            // CODE return (vw) => { ... return (ex, label) => { ... } }
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
    }
}
