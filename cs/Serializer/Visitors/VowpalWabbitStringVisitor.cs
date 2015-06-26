// --------------------------------------------------------------------------------------------------------------------
// <copyright file="VowpalWabbitStringVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using VW.Serializer.Interfaces;

namespace VW.Serializer.Visitors
{
    /// <summary>
    /// Front-end to serialize data into Vowpal Wabbit string format.
    /// </summary>
    //public sealed class VowpalWabbitStringVisitor : IVowpalWabbitVisitor<string, string, string>
    //{
    //    private string VisitNamespace(INamespace @namespace)
    //    {
    //        return string.Format(
    //            CultureInfo.InvariantCulture,
    //            "|{0}{1}",
    //            @namespace.FeatureGroup,
    //            @namespace.Name);
    //    }

    //    public string Visit<T>(INamespaceDense<T> namespaceDense)
    //    {
    //        return string.Format(
    //            CultureInfo.InvariantCulture,
    //            "{0} {1}",
    //            this.VisitNamespace(namespaceDense),
    //            string.Join(" ", namespaceDense.DenseFeature.Value.Select(v => ":" + v)));
    //    }


    //    public string Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature)
    //    {
    //        return this.Visit(feature, key => Convert.ToString(key));
    //    }

    //    private string Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature, Func<TKey, string> keyMapper)
    //    {
    //        return string.Join(" ",
    //            feature.Value.Select(kvp => string.Format(
    //                CultureInfo.InvariantCulture,
    //                "{0}:{1}",
    //                keyMapper(kvp.Key),
    //                kvp.Value)));
    //    }

    //    public string Visit(IFeature<string> feature)
    //    {
    //        return this.Visit<string>(feature);
    //    }

    //    public string Visit<TValue>(IFeature<IEnumerable<TValue>> feature)
    //    {
    //        return string.Join(" ", 
    //            feature.Value.Select((value, i) =>
    //                string.Format(
    //                    CultureInfo.InvariantCulture,
    //                    "{0}:{1}",
    //                    i,
    //                    value)));
    //    }

    //    public string VisitEnumerize<T>(IFeature<T> feature)
    //    {
    //        return string.Format(
    //            CultureInfo.InvariantCulture,
    //            "{0}_{1}",
    //            feature.Name,
    //            feature.Value);
    //    }

    //    public string Visit<T>(IFeature<T> feature)
    //    {
    //        // can't specify constraints to narrow for enums
    //        var valueType = typeof(T);
    //        if (valueType.IsEnum)
    //        {
    //            return string.Format(
    //                CultureInfo.InvariantCulture, 
    //                "{0}_{1}", 
    //                feature.Name, 
    //                Enum.GetName(valueType, feature.Value));
    //        }

    //        return string.Format(
    //            CultureInfo.InvariantCulture, 
    //            "{0}:{1}", 
    //            feature.Name, 
    //            feature.Value);
    //    }

    //    public string Visit(INamespaceSparse<string> namespaceSparse)
    //    {
    //        var featureResults = from feature in namespaceSparse.Features
    //                             let result = feature.Visit()
    //                             where result != null
    //                             select result;

    //        return string.Format(
    //            CultureInfo.InvariantCulture,
    //            "{0} {1}",
    //                VisitNamespace(namespaceSparse),
    //                string.Join(" ", featureResults));
    //    }

    //    public string Visit(string label, IVisitableNamespace<string>[] namespaces)
    //    {
    //        // see https://github.com/JohnLangford/vowpal_wabbit/wiki/Input-format 
    //        // prefix with label
    //        var sb = new StringBuilder();
    //        if (!string.IsNullOrEmpty(label))
    //        {
    //            sb.Append(label).Append(' ');
    //        }

    //        sb.Append(string.Join(" ", namespaces.Select(n => n.Visit())));

    //        return sb.ToString();
    //    }
    //}
}
