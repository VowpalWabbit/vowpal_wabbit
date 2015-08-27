// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitSerializerCompiler.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Tasks;
using VW.Interfaces;
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
    /// <typeparam name="TExampleResult">The resulting serialization type.</typeparam>
    /// <returns>A serializer for the given user example type.</returns>
    internal sealed class VowpalWabbitSerializerCompiler<TExample, TVisitor, TExampleResult>
    {
        private static readonly string SerializeMethodName = "Serialize";

        private static readonly ConstructorInfo ArgumentNullExceptionConstructorInfo = (ConstructorInfo)ReflectionHelper.GetInfo((ArgumentNullException t) => new ArgumentNullException(""));

        private readonly List<FeatureExpression> allFeatures;
        private readonly List<Type> featurizerTypes;
        private readonly List<Expression> body;
        private readonly List<ParameterExpression> variables;
        private readonly List<ParameterExpression> namespaceVariables;
        
        private ParameterExpression valueParameter;
        private ParameterExpression labelParameter;
        private ParameterExpression vwParameter;
        private ParameterExpression mainVisitorParameter;
        private List<ParameterExpression> featurizers;

        internal VowpalWabbitSerializerCompiler(List<FeatureExpression> allFeatures, List<Type> featurizerTypes)
        {
            this.allFeatures = allFeatures;
            this.featurizerTypes = featurizerTypes ?? new List<Type>(0);

            var overrideFeaturizerTypes = this.allFeatures
                .Where(f => f.OverrideSerializeMethod != null)
                .Select(f => f.OverrideSerializeMethod.DeclaringType);

            this.featurizerTypes = this.featurizerTypes.Union(overrideFeaturizerTypes).Distinct().ToList();

            this.body = new List<Expression>();
            this.variables = new List<ParameterExpression>();
            this.namespaceVariables = new List<ParameterExpression>();
            this.featurizers = new List<ParameterExpression>();

            this.CreateParameters();
            this.CreateVisitor();
            this.CreateFeatures();
            this.CreateLambda();
            this.Compile();
        }

        internal Expression<Func<VowpalWabbit, TExample, ILabel, TExampleResult>> ResultExpression
        {
            get;
            private set;
        }

        internal Func<VowpalWabbit, TExample, ILabel, TExampleResult> Result
        {
            get;
            private set;
        }

        private void Compile()
        {
            var asmName = new AssemblyName("VowpalWabbitSerializer." + typeof(TExample).Name + "." + typeof(TVisitor));
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
                typeof(void),
                new[] { typeof(VowpalWabbit), typeof(TExample), typeof(ILabel) });

            // compared to Compile this looks rather ugly, but there is a feature-bug 
            // that adds a security check to every call of the Serialize method
            //#if !DEBUG
            //var debugInfoGenerator = DebugInfoGenerator.CreatePdbGenerator();
            //visit.CompileToMethod(methodBuilder, debugInfoGenerator);
            //#else
            this.ResultExpression.CompileToMethod(methodBuilder);
            //#endif

            var dynType = typeBuilder.CreateType();

            this.Result = (Func<VowpalWabbit, TExample, ILabel, TExampleResult>)Delegate.CreateDelegate(
                typeof(Func<VowpalWabbit, TExample, ILabel, TExampleResult>),
                dynType.GetMethod(SerializeMethodName));
        }

        /// <summary>
        /// define functions input parameter
        /// </summary>
        private void CreateParameters()
        {
            this.valueParameter = Expression.Parameter(typeof(TExample), "value");
            this.labelParameter = Expression.Parameter(typeof(ILabel), "label");
            this.vwParameter = Expression.Parameter(typeof(VowpalWabbit), "vw");
            this.mainVisitorParameter = Expression.Variable(typeof(TVisitor), "visitor");
            this.variables.Add(this.mainVisitorParameter);

            if (this.featurizerTypes != null)
            {
                foreach (var visitorType in this.featurizerTypes)
                {
                    var parameter = Expression.Parameter(visitorType);

                    this.featurizers.Add(parameter);
                    this.variables.Add(parameter);
                }
            }
        }

        private void CreateVisitor(ParameterExpression visitorParameter, params Expression[] constructorParameters)
        {
            var paramSubset = new List<Expression>(constructorParameters);

            // search for different constructors
            while (paramSubset.Count > 0)
            {
                var visitorCtor = visitorParameter.Type.GetConstructor(paramSubset.Select(p => p.Type).ToArray());
                if (visitorCtor != null)
                {
                    // CODE visitor = new TVisitor(vw)
                    this.body.Add(Expression.Assign(visitorParameter, Expression.New(visitorCtor, paramSubset)));
                    return;
                }

                paramSubset.RemoveAt(paramSubset.Count - 1);
            }

            // CODE visitor = new TVisitor()
            this.body.Add(Expression.Assign(visitorParameter, Expression.New(visitorParameter.Type)));
        }

        private void CreateVisitor()
        {
            this.CreateVisitor(this.mainVisitorParameter, this.vwParameter);

            foreach (var featurizer in this.featurizers)
            {
                this.CreateVisitor(featurizer, this.vwParameter, this.mainVisitorParameter);
            }
        }

        private void CreateFeatures()
        {
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

            var featuresByNamespace = allFeatures.GroupBy(f => new { f.Namespace, f.FeatureGroup, f.IsDense }, f => f);
            foreach (var ns in featuresByNamespace)
            {
                var features = ns.OrderBy(f => f.Order).ToList();

                if (ns.Key.IsDense)
                {
                    CreateDenseFeatureVisits(features);
                }
                else
                {
                    CreateSparseFeaturesVisits(ns.Key.Namespace, ns.Key.FeatureGroup, features);
                }
            }
        }

        private void CreateLambda()
        {
            // CODE return visitor.Visit(label, new[] { ns1, ns2, ... })
            //body.Add(Log());
            var visitWithLabelMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(ILabel), typeof(IVisitableNamespace[]) });
            this.body.Add(
                Expression.Call(
                    this.mainVisitorParameter,
                    visitWithLabelMethod,
                    this.labelParameter,
                    Expression.NewArrayInit(
                        typeof(IVisitableNamespace),
                        namespaceVariables.ToArray<Expression>())));

            // CODE (example, visitor) => { ... }
            this.ResultExpression = Expression.Lambda<Func<VowpalWabbit, TExample, ILabel, TExampleResult>>(
                Expression.Block(this.variables.Union(this.namespaceVariables), this.body),
                this.vwParameter,
                this.valueParameter,
                this.labelParameter);
        }

        private static List<MemberAssignment> CreateNamespaceAndFeatureGroupBinding(string @namespace, char? featureGroup)
        {
            var baseNamespaceInits = new List<MemberAssignment> 
                {
                    Expression.Bind(
                        ReflectionHelper.GetInfo((Namespace n) => n.Name),
                        Expression.Constant(@namespace, typeof(string)))
                };

            if (featureGroup != null)
            {
                baseNamespaceInits.Add(
                    Expression.Bind(
                        ReflectionHelper.GetInfo((Namespace n) => n.FeatureGroup),
                        Expression.Convert(Expression.Constant((char)featureGroup), typeof(char?))));
            }

            return baseNamespaceInits;
        }

        private void CreateDenseFeatureVisits(List<FeatureExpression> features)
        {
            // Dense namespace
            if (features.Count != 1)
            {
                throw new NotSupportedException("Only a single dense vector is supported per namespace");
            }

            var feature = features.First();

            var namespaceType = typeof(NamespaceDense<>).MakeGenericType(feature.DenseFeatureValueElementType);

            var bindings = CreateNamespaceAndFeatureGroupBinding(feature.Namespace, feature.FeatureGroup);
            bindings.Add(Expression.Bind(namespaceType.GetProperty("DenseFeature"), feature.CreateFeatureExpression(valueParameter)));

            var namespaceDense = Expression.MemberInit(Expression.New(namespaceType), bindings);

            var namespaceVariable = Expression.Variable(namespaceType);
            namespaceVariables.Add(namespaceVariable);

            // CODE namespace = new Namespace<float> { ... };
            //body.Add(Log());
            this.body.Add(Expression.Assign(namespaceVariable, namespaceDense));

            // find the first featurizer that supports this feature type
            foreach (var featurizer in this.featurizers)
            {
                if (this.CreateDenseFeatureVisit(featurizer, namespaceVariable))
                {
                    return;
                }
            }

            this.CreateDenseFeatureVisit(this.mainVisitorParameter, namespaceVariable);
        }

        private bool CreateDenseFeatureVisit(ParameterExpression visitor, ParameterExpression namespaceVariable)
        {
            var method = ReflectionHelper.FindMethod(visitor.Type, "Visit", namespaceVariable.Type);
            if (method == null)
            {
                return false;
            }

            // CODE namespace.Visit = () => visitor.Visit(namespace)
            //body.Add(Log());
            this.body.Add(Expression.Assign(
                    Expression.Property(namespaceVariable, namespaceVariable.Type.GetProperty("Visit")),
                    Expression.Lambda<Action>(
                        Expression.Call(
                            visitor,
                            method,
                            namespaceVariable))));

            return true;
        }

        private void CreateSparseFeaturesVisits(string @namespace, char? featureGroup, List<FeatureExpression> features)
        {
            // Sparse namespace
            var featureVariables = new List<ParameterExpression>();

            foreach (var feature in features)
            {
                var featureVariable = Expression.Parameter(feature.IntermediateFeatureType, feature.VariableName);

                variables.Add(featureVariable);
                featureVariables.Add(featureVariable);

                // CODE feature = new Feature<float> { ... };
                //body.Add(Log());
                this.body.Add(Expression.Assign(featureVariable, feature.CreateFeatureExpression(valueParameter)));
            }

            // features belong to the same namespace
            var bindings = CreateNamespaceAndFeatureGroupBinding(@namespace, featureGroup);
            bindings.Add(Expression.Bind(
                                ReflectionHelper.GetInfo((NamespaceSparse n) => n.Features),
                                Expression.NewArrayInit(typeof(IVisitableFeature), featureVariables)));

            // CODE new NamespaceSparse { Features = new[] { feature1, feature2, ... } }
            var namespaceSparse = Expression.MemberInit(Expression.New(typeof(NamespaceSparse)), bindings);

            var namespaceVariable = Expression.Variable(typeof(NamespaceSparse), "namespaceSparse");
            namespaceVariables.Add(namespaceVariable);

            // CODE namespace = new NamespaceSparse { ... }
            //body.Add(Log());
            this.body.Add(Expression.Assign(namespaceVariable, namespaceSparse));

            // loop unrolling to have dispatch onto the correct Visit<T>
            for (var i = 0; i < features.Count; i++)
            {
                var feature = features[i];
                var featureVariable = featureVariables[i];

                Expression visitFeatureCall = this.CreateFeatureVisitFromOverride(feature, featureVariable);

                if (visitFeatureCall == null)
                {
                    foreach (var featurizer in this.featurizers)
                    {
                        visitFeatureCall = this.CreateFeatureVisit(feature, featurizer, featureVariable);
                        if (visitFeatureCall != null)
                        {
                            break;
                        }
                    }
                }

                if (visitFeatureCall == null)
                {
                    visitFeatureCall = CreateFeatureVisit(feature, this.mainVisitorParameter, featureVariable);
                }

                if (visitFeatureCall == null)
                {
                    throw new NotSupportedException("Feature type is not supported: " + featureVariable.Type);
                }

                // CODE feature.Visit = visitor.Visit;
                //body.Add(Log());
                this.body.Add(
                    Expression.Assign(
                        Expression.Property(featureVariable, featureVariable.Type.GetProperty("Visit")),
                        Expression.Lambda<Action>(visitFeatureCall)));
            }

            // CODE namespace.Visit = () => { visitor.Visit(namespace); });
            //body.Add(Log());
            // return (MethodInfo)ReflectionHelper.GetInfo((IVowpalWabbitVisitor<TExampleResult> e) => e.Visit((NamespaceSparse)null));
            var visitMethod = typeof(TVisitor).GetMethod("Visit", new[] { typeof(INamespaceSparse) });
            this.body.Add(
                Expression.Assign(
                    Expression.Property(
                        namespaceVariable,
                        (PropertyInfo)ReflectionHelper.GetInfo((NamespaceSparse n) => n.Visit)),
                    Expression.Lambda<Action>(
                        Expression.Call(
                            this.mainVisitorParameter,
                            visitMethod,
                            namespaceVariable))));
        }

        private Expression CreateFeatureVisitFromOverride(FeatureExpression feature, ParameterExpression featureVariable)
        {
            var method = feature.OverrideSerializeMethod;
            if (method == null)
            {
                return null;
            }

            var overrideFeaturizer = this.featurizers.FirstOrDefault(f => f.Type == method.DeclaringType);
            if (overrideFeaturizer == null)
            {
                throw new ArgumentException("Featurizer missing");
            }

            return CreateFeatureVisit(overrideFeaturizer, featureVariable, method);
        }

        private Expression CreateFeatureVisit(FeatureExpression feature, ParameterExpression visitorParameter, ParameterExpression featureVariable)
        {
            var method = ReflectionHelper.FindMethod(visitorParameter.Type, feature.Enumerize ? "VisitEnumerize" : "Visit", featureVariable.Type);
            if (method == null)
            {
                return null;
            }

            return CreateFeatureVisit(visitorParameter, featureVariable, method);
        }

        private static Expression CreateFeatureVisit(ParameterExpression visitorParameter, ParameterExpression featureVariable, MethodInfo method)
        {
            // CODE: visitor.Visit(feature1); 
            Expression visitFeatureCall = Expression.Call(
                        visitorParameter,
                        method,
                        featureVariable);

            var featureValue = Expression.Property(featureVariable, "Value");
            if (!featureValue.Type.IsValueType || (featureValue.Type.IsGenericType && featureValue.Type.GetGenericTypeDefinition() == typeof(Nullable<>)))
            {
                // CODE feature1.Value != null ? visitor.Visit(feature1) : default(TFeatureResult);
                visitFeatureCall = Expression.IfThen(
                        test: Expression.NotEqual(featureValue, Expression.Constant(null)),
                        ifTrue: visitFeatureCall);
            }

            return visitFeatureCall;
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
    }
}
