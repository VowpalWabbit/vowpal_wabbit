// --------------------------------------------------------------------------------------------------------------------
// <copyright file="AnnotationInspector.cs">
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
using VW.Serializer.Attributes;
using VW.Serializer.Intermediate;

namespace VW.Serializer
{
    internal static class AnnotationInspector
    {
        internal static List<FeatureExpression> ExtractFeatures(Type type)
        {
            return ExtractFeatures(type, null, null, valueExpression => valueExpression);
        }

        private static List<FeatureExpression> ExtractFeatures(Type type, string parentNamespace, char? parentFeatureGroup, Func<Expression, Expression> valueExpressionFactory)
        {
            var props = type.GetProperties(BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public);

            var localFeatures = from p in props
                                let attr = (FeatureAttribute)p.GetCustomAttributes(typeof(FeatureAttribute), true).FirstOrDefault()
                                where attr != null
                                select new FeatureExpression(
                                    featureType: p.PropertyType,
                                    name: attr.Name ?? p.Name,
                                    valueExpressionFactory: valueExpression => Expression.Property(valueExpressionFactory(valueExpression), p),
                                    @namespace: attr.Namespace ?? parentNamespace,
                                    featureGroup: attr.InternalFeatureGroup ?? parentFeatureGroup,
                                    enumerize: attr.Enumerize,
                                    variableName: p.Name,
                                    order: attr.Order,
                                    addAnchor: attr.AddAnchor);

            // Recurse
            return localFeatures
                .SelectMany(f =>
                {
                    var subFeatures = ExtractFeatures(f.FeatureType, f.Namespace, f.FeatureGroup, f.ValueExpressionFactory);
                    return subFeatures.Count == 0 ? new List<FeatureExpression>{ f } : subFeatures;
                })
                .ToList();
        }
    }
}
