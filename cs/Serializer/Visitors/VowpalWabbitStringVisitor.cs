using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Research.MachineLearning.Serializer.Interfaces;

namespace Microsoft.Research.MachineLearning.Serializer.Visitor
{
    public class VowpalWabbitStringVisitor : IVowpalWabbitVisitor<string, string, string>
    {
        public VowpalWabbitStringVisitor()
        {
        }

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
                return string.Empty;
            }

            return this.VisitNamespace(namespaceDense) +
                   string.Join(" ", namespaceDense.DenseFeature.Value.Select(v => ":" + v));
        }

        #region Dictionary support
        /*        
        public void Visit<TValue>(IFeature<IDictionary<Int16, TValue>> feature)
        {
            this.Visit(feature, key => (UInt32)key);
        }

        public void Visit<TValue>(IFeature<IDictionary<Int32, TValue>> feature)
        {
            this.Visit(feature, key => (UInt32)key);
        }

        // TODO: not clear when to hash... should we check for negative values?
        public void Visit<TValue>(IFeature<IDictionary<Int64, TValue>> feature)
        {
            this.Visit(feature, key => (UInt32)key);
        }

        public void Visit<TValue>(IFeature<IDictionary<UInt16, TValue>> feature)
        {
            this.Visit(feature, key => key);
        }
        public void Visit<TValue>(IFeature<IDictionary<UInt32, TValue>> feature)
        {
            this.Visit(feature, key => key);
        }
        */

        public string Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature)
        {
            // TODO: call VwHash
            return this.Visit(feature, key => Convert.ToString(key));
        }

        private string Visit<TKey, TValue>(IFeature<IDictionary<TKey, TValue>> feature, Func<TKey, string> keyMapper)
        {
            // lhs: int, hash(string), hash(long), hash(*) -> uint
            // rhs: int, short, long, float, bool -> float

            return string.Join(" ",
                feature.Value.Select(kvp => string.Format(
                    CultureInfo.InvariantCulture,
                    "{0}:{1}",
                    keyMapper(kvp.Key),
                    kvp.Value)));
        }

        #endregion

        public string Visit(IFeature<string> feature)
        {
            return this.Visit<string>(feature);
        }

        public string Visit<TValue>(IFeature<IEnumerable<TValue>> feature)
        {
            // TODO: call VwHash
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
            return string.Format(
                CultureInfo.InvariantCulture,
                "{0} {1}",
                    VisitNamespace(namespaceSparse),
                    string.Join(" ", namespaceSparse.Features.Select(f => f.Visit())));
        }

        public string Visit(string comment, IVisitableNamespace<string>[] namespaces)
        {
            return string.Format(
                CultureInfo.InvariantCulture,
                "`{0} {1}", 
                comment,
                string.Join(" ", namespaces.Select(n => n.Visit())));

            // TODO: it's unclear who generates the separating new line in the case of action dependent features
        }
    }
}
