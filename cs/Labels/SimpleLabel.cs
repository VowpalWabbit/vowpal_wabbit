// --------------------------------------------------------------------------------------------------------------------
// <copyright file="IVowpalWabbitVisitor.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Interfaces;
using System.Globalization;
using System.Text;

namespace VW.Labels
{
    public sealed class SimpleLabel : ILabel
    {
        /// <summary>
        /// Gets or sets the float label.
        /// </summary>
        public float Label { get; set; }

        /// <summary>
        /// Gets or sets the optional weight.
        /// </summary>
        public float? Weight { get; set; }

        /// <summary>
        /// Gets or sets the optional initial value.
        /// </summary>
        public float? Initial { get; set; }

        /// <summary>
        /// Serialize to Vowpal Wabbit string format.
        /// </summary>
        /// <remarks>see simple_label.cc: parse_simple_label</remarks>
        public string ToVowpalWabbitFormat()
        {
            if (Weight == null)
            {
                return this.Label.ToString(CultureInfo.InvariantCulture);
            }

            var sb = new StringBuilder();
            sb.Append(this.Label.ToString(CultureInfo.InvariantCulture));
            sb.Append(' ');
            sb.Append(this.Weight.Value.ToString(CultureInfo.InvariantCulture));

            if (Initial != null)
            {
                sb.Append(' ');
                sb.Append(this.Initial.Value.ToString(CultureInfo.InvariantCulture));
            }
            return sb.ToString();
        }
    }
}
