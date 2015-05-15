using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Visitors
{
    public class VowpalWabbitStringVisitor : IVowpalWabbitVisitor<string, string, string>
    {
        private string VisitNamespace(INamespace @namespace)
        {
            return string.Format(
                CultureInfo.InvariantCulture,
                "|{0}{1}",
                @namespace.FeatureGroup,
                @namespace.Name);
        }

        public string Visit<T>(INamespaceDense<T> namespaceDense)
        {
            // TODO: move to compiled Lambda
            if (namespaceDense.DenseFeature.Value == null)
            {
                return null;
            }

            return string.Format(
                CultureInfo.InvariantCulture,
                "{0} {1}",
                this.VisitNamespace(namespaceDense),
                string.Join(" ", namespaceDense.DenseFeature.Value.Select(v => ":" + v)));
        }


        public string Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature)
        {
            return this.Visit(feature, key => Convert.ToString(key));
        }

        private string Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature, Func<TKey, string> keyMapper)
        {
            return string.Join(" ",
                feature.Value.Select(kvp => string.Format(
                    CultureInfo.InvariantCulture,
                    "{0}:{1}",
                    keyMapper(kvp.Key),
                    kvp.Value)));
        }

        public string Visit(IFeature<string> feature)
        {
            return this.Visit<string>(feature);
        }

        public string Visit<TValue>(IFeature<IEnumerable<TValue>> feature)
        {
            // this.Visit(feature, key => (UInt32)Convert.ToString(key).GetHashCode());
            return string.Join(" ", 
                feature.Value.Select((value, i) =>
                    string.Format(
                        CultureInfo.InvariantCulture,
                        "{0}:{1}",
                        i,
                        value)));
        }

        public string VisitEnumerize<T>(IFeature<T> feature)
        {
            return string.Format(
                CultureInfo.InvariantCulture,
                "{0}_{1}",
                feature.Name,
                feature.Value);
        }

        public string Visit<T>(IFeature<T> feature)
        {
            // can't specify constraints to narrow for enums
            var valueType = typeof(T);
            if (valueType.IsEnum)
            {
                return string.Format(
                    CultureInfo.InvariantCulture, 
                    "{0}_{1}", 
                    feature.Name, 
                    Enum.GetName(valueType, feature.Value));
            }
            // TODO: more support for built-in types
            return string.Format(
                CultureInfo.InvariantCulture, 
                "{0}:{1}", 
                feature.Name, 
                feature.Value);
        }

        public string Visit(INamespaceSparse<string> namespaceSparse)
        {
            var featureResults = from feature in namespaceSparse.Features
                                 let result = feature.Visit()
                                 where result != null
                                 select result;

            return string.Format(
                CultureInfo.InvariantCulture,
                "{0} {1}",
                    VisitNamespace(namespaceSparse),
                    string.Join(" ", featureResults));
        }

        public string Visit(IVisitableNamespace<string>[] namespaces)
        {
            return string.Join(" ", namespaces.Select(n => n.Visit()));
        }
    }
}
