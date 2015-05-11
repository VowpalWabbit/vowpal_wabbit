using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Research.MachineLearning.Serializer.Intermediate
{
    /// <summary>
    /// Intermediate representation of a Feature
    /// </summary>
    internal sealed class FeatureExpression
    {
        internal Type FeatureType { get; set; }

        internal Type FeatureValueType { get; set; }

        internal string Name { get; set; }

        internal string Namespace { get; set; }

        internal char? FeatureGroup { get; set; }

        internal bool IsDense { get { return this.DenseFeatureValueElementType != null; } }

        internal bool Enumerize { get; set; }

        internal MemberInitExpression NewFeatureExpression { get; set; }

        internal MemberExpression PropertyExpression { get; set; }

        internal Type DenseFeatureValueElementType { get; set; }
    }

}
