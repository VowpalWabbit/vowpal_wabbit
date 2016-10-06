// --------------------------------------------------------------------------------------------------------------------
// <copyright file="AnnotationInspector.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using VW.Labels;
using VW.Serializer.Attributes;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    /// <summary>
    /// Utility class analyzing compile-time <see cref="FeatureAttribute"/> annotation.
    /// </summary>
    public static class TypeInspector
    {
        /// <summary>
        /// All properties are used as features.
        /// </summary>
        public static readonly ITypeInspector All;

        /// <summary>
        /// Only properties annotated using Feature attribute are considered.
        /// </summary>
        public static readonly ITypeInspector Default;

        static TypeInspector()
        {
            All = new AnnotationInspectorAll();
            Default = new AnnotationInspectorDefault();
        }

        private sealed class AnnotationInspectorDefault : ITypeInspector
        {
            public Schema CreateSchema(VowpalWabbitSettings settings, Type type)
            {
                return TypeInspector.CreateSchema(type,
                    featurePropertyPredicate: (_, attr) => attr != null,
                    labelPropertyPredicate: (_, attr) => attr != null);
            }
        }

        private sealed class AnnotationInspectorAll : ITypeInspector
        {
            public Schema CreateSchema(VowpalWabbitSettings settings, Type type)
            {
                return TypeInspector.CreateSchema(type,
                    featurePropertyPredicate: (_, __) => true,
                    labelPropertyPredicate: (_, __) => true);
            }
        }

        private static Schema CreateSchema(Type type,
            Func<PropertyInfo, FeatureAttribute, bool> featurePropertyPredicate,
            Func<PropertyInfo, LabelAttribute, bool> labelPropertyPredicate)
        {
            Contract.Requires(type != null);
            Contract.Requires(featurePropertyPredicate != null);
            Contract.Requires(labelPropertyPredicate != null);

            var validExpressions = new Stack<Func<Expression,Expression>>();

            // CODE example != null
            validExpressions.Push(valueExpression => Expression.NotEqual(valueExpression, Expression.Constant(null)));

            return CreateSchema(
                null,
                type,
                null,
                null,
                null,
                // CODE example
                valueExpression => valueExpression,
                validExpressions,
                featurePropertyPredicate,
                labelPropertyPredicate);
        }

        private static Schema CreateSchema(
            FeatureExpression parent,
            Type type,
            string parentNamespace,
            char? parentFeatureGroup,
            bool? parentDictify,
            Func<Expression, Expression> valueExpressionFactory,
            Stack<Func<Expression, Expression>> valueValidExpressionFactories,
            Func<PropertyInfo, FeatureAttribute, bool> featurePropertyPredicate,
            Func<PropertyInfo, LabelAttribute, bool> labelPropertyPredicate)
        {
            var props = type.GetProperties(BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public);

            var localFeatures = (from p in props
                                 let declaredAttr = (FeatureAttribute)p.GetCustomAttributes(typeof(FeatureAttribute), true).FirstOrDefault()
                                 where featurePropertyPredicate(p, declaredAttr)
                                 let attr = declaredAttr ?? new FeatureAttribute()
                                 select new FeatureExpression(
                                     featureType: p.PropertyType,
                                     name: attr.Name ?? p.Name,
                                     // CODE example.Property
                                     valueExpressionFactory: valueExpression => Expression.Property(valueExpressionFactory(valueExpression), p),
                                     // @Reverse: make sure conditions are specified in the right order
                                     valueValidExpressionFactories: valueValidExpressionFactories.Reverse().ToList(),
                                     @namespace: attr.Namespace ?? parentNamespace,
                                     featureGroup: attr.InternalFeatureGroup ?? parentFeatureGroup,
                                     enumerize: attr.Enumerize,
                                     variableName: p.Name,
                                     order: attr.Order,
                                     addAnchor: attr.AddAnchor,
                                     stringProcessing: attr.StringProcessing,
                                     dictify: attr.InternalDictify ?? parentDictify,
                                     parent: parent)
                                 ).ToList();

            var localLabels = from p in props
                              let declaredAttr = (LabelAttribute)p.GetCustomAttributes(typeof(LabelAttribute), true).FirstOrDefault()
                              where labelPropertyPredicate(p, declaredAttr) || typeof(ILabel).IsAssignableFrom(p.PropertyType)
                              let attr = declaredAttr ?? new LabelAttribute()
                              let labelType = p.PropertyType
                              where typeof(ILabel).IsAssignableFrom(labelType) || p.PropertyType == typeof(string)
                              select new LabelExpression
                              {
                                    Name = p.Name,
                                    LabelType = p.PropertyType,
                                    // CODE example.Property
                                    ValueExpressionFactory = valueExpression => Expression.Property(valueExpressionFactory(valueExpression), p),
                                    // @Reverse: make sure conditions are specified in the right order
                                    ValueValidExpressionFactories = valueValidExpressionFactories.Reverse().ToList()
                              };

            // Recurse
            var schemas = localFeatures
                .Select(f =>
                {
                    // CODE example.Prop1.Prop2 != null
                    valueValidExpressionFactories.Push(valueExpression => Expression.NotEqual(f.ValueExpressionFactory(valueExpression), Expression.Constant(null)));
                    var subSchema = CreateSchema(f, f.FeatureType, f.Namespace, f.FeatureGroup, f.Dictify, f.ValueExpressionFactory, valueValidExpressionFactories, featurePropertyPredicate, labelPropertyPredicate);
                    valueValidExpressionFactories.Pop();

                    return subSchema;
                })
                .ToList();

            return new Schema
            {
                Features = localFeatures.Union(schemas.SelectMany(s => s.Features)).ToList(),
                Label = localLabels.Union(schemas.Select(s => s.Label)).FirstOrDefault(l => l != null)
            };
        }
    }
}
