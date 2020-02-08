using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;
using VW.Reflection;

namespace VW.Serializer
{
    /// <summary>
    /// Factory class to create <see cref="IVowpalWabbitSerializer{TExample}"/>.
    /// </summary>
    public static class VowpalWabbitMultiExampleSerializerCompiler
    {
        /// <summary>
        /// Creates a serializer for <typeparamref name="TExample"/> based on <paramref name="settings"/> and <paramref name="schema"/>,
        /// </summary>
        /// <typeparam name="TExample">The example type.</typeparam>
        /// <param name="settings">Settings for inspection.</param>
        /// <param name="schema">The schema used for serializer creation.</param>
        /// <returns>If the schema is valid a compiler is created, otherwise null.</returns>
        public static IVowpalWabbitSerializerCompiler<TExample> TryCreate<TExample>(VowpalWabbitSettings settings, Schema schema)
        {
            // check for _multi
            var multiFeature = schema.Features.FirstOrDefault(fe => fe.Name == settings.PropertyConfiguration.MultiProperty);
            if (multiFeature == null)
                return null;

            // multi example path
            // IEnumerable<> or Array
            var adfType = InspectionHelper.GetEnumerableElementType(multiFeature.FeatureType);
            if (adfType == null)
                throw new ArgumentException(settings.PropertyConfiguration.MultiProperty + " property must be array or IEnumerable<>. Actual type: " + multiFeature.FeatureType);

            var compilerType = typeof(VowpalWabbitMultiExampleSerializerCompilerImpl<,>).MakeGenericType(typeof(TExample), adfType);
            return (IVowpalWabbitSerializerCompiler<TExample>)Activator.CreateInstance(compilerType, settings, schema, multiFeature);
        }

        private sealed class VowpalWabbitMultiExampleSerializerCompilerImpl<TExample, TActionDependentFeature> : IVowpalWabbitSerializerCompiler<TExample>, IVowpalWabbitMultiExampleSerializerCompiler<TExample>
        {
            private readonly VowpalWabbitSingleExampleSerializerCompiler<TExample> sharedSerializerCompiler;

            private readonly VowpalWabbitSingleExampleSerializerCompiler<TActionDependentFeature> adfSerializerComputer;

            private readonly Func<TExample, IEnumerable<TActionDependentFeature>> adfAccessor;

            public VowpalWabbitMultiExampleSerializerCompilerImpl(VowpalWabbitSettings settings, Schema schema, FeatureExpression multiFeature)
            {
                Contract.Requires(settings != null);
                Contract.Requires(schema != null);
                Contract.Requires(multiFeature != null);

                var nonMultiFeatures = schema.Features.Where(fe => fe != multiFeature).ToList();

                this.sharedSerializerCompiler = nonMultiFeatures.Count == 0 ? null :
                    new VowpalWabbitSingleExampleSerializerCompiler<TExample>(
                        new Schema { Features = nonMultiFeatures },
                        settings == null ? null : settings.CustomFeaturizer,
                        !settings.EnableStringExampleGeneration);

                this.adfSerializerComputer = new VowpalWabbitSingleExampleSerializerCompiler<TActionDependentFeature>(
                    settings.TypeInspector.CreateSchema(settings, typeof(TActionDependentFeature)),
                    settings == null ? null : settings.CustomFeaturizer,
                    !settings.EnableStringExampleGeneration);

                var exampleParameter = Expression.Parameter(typeof(TExample), "example");

                // CODE condition1 && condition2 && condition3 ...
                var condition = multiFeature.ValueValidExpressionFactories
                    .Skip(1)
                    .Aggregate(
                        multiFeature.ValueValidExpressionFactories.First()(exampleParameter),
                        (cond, factory) => Expression.AndAlso(cond, factory(exampleParameter)));

                var multiExpression = multiFeature.ValueExpressionFactory(exampleParameter);

                // CODE example => (IEnumerable<TActionDependentFeature>)(example._multi != null ? example._multi : null)
                var expr = Expression.Lambda<Func<TExample, IEnumerable<TActionDependentFeature>>>(
                        Expression.Condition(
                            condition,
                            multiExpression,
                            Expression.Constant(null, multiExpression.Type),
                            typeof(IEnumerable<TActionDependentFeature>)),
                    exampleParameter);

                this.adfAccessor = (Func<TExample, IEnumerable<TActionDependentFeature>>)expr.CompileToFunc();
            }

            public int GetNumberOfActionDependentExamples(TExample example)
            {
                var adfs = this.adfAccessor(example);
                return adfs == null ? 0 : adfs.Count();
            }

            public IVowpalWabbitSerializer<TExample> Create(VowpalWabbit vw)
            {
                return new VowpalWabbitMultiExampleSerializer<TExample, TActionDependentFeature>(
                    vw,
                    this.sharedSerializerCompiler != null ? this.sharedSerializerCompiler.Create(vw) : null,
                    this.adfSerializerComputer.Create(vw),
                    this.adfAccessor);
            }
        }
    }
}
