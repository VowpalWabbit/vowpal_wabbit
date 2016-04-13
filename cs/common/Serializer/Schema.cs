using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    public sealed class Schema
    {
        public List<FeatureExpression> Features { get; set; }

        public LabelExpression Label { get; set; }
    }
}
