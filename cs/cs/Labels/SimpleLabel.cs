// --------------------------------------------------------------------------------------------------------------------
// <copyright file="SimpleLabel.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using VW.Interfaces;
using System.Globalization;
using System.Text;
using Newtonsoft.Json;

namespace VW.Labels
{
    /// <summary>
    /// Helper to ease construction of simple label.
    /// </summary>
    public sealed class SimpleLabel : ILabel
    {
        /// <summary>
        /// Gets or sets the float label.
        /// </summary>
        [JsonProperty]
        public float Label { get; set; }

        /// <summary>
        /// Gets or sets the optional weight.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public float? Weight { get; set; }

        /// <summary>
        /// Gets or sets the optional initial value.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public float? Initial { get; set; }

        /// <summary>
        /// Serialize to Vowpal Wabbit string format.
        /// </summary>
        /// <remarks>see simple_label.cc: parse_simple_label</remarks>
        public string ToVowpalWabbitFormat()
        {
            // Note: this code was inspected closely as it is on the hot performance path.
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
