using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    public sealed class LabelExpression
    {
        /// <summary>
        /// The name of the label.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The type of the feature.
        /// </summary>
        public Type LabelType { get; set; }

        /// <summary>
        /// Factory to extract the value for a given feature from the example object (input argument).
        /// </summary>
        public Func<Expression, Expression> ValueExpressionFactory { get; set; }

        /// <summary>
        /// Factories to provide validation before invoking the expression created through <see cref="ValueExpressionFactory"/>.
        /// </summary>
        public List<Func<Expression, Expression>> ValueValidExpressionFactories { get; set; }

    }
}
