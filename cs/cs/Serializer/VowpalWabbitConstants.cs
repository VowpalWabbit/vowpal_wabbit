using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    internal static class VowpalWabbitConstants
    {
        /// <summary>
        /// The VW default namespace is denoted by a blank.
        /// </summary>
        internal const char DefaultNamespace = ' ';

        /// <summary>
        /// JSON properties starting with underscore are ignored.
        /// </summary>
        internal const string FeatureIgnorePrefix = "_";

        /// <summary>
        /// JSON property "_text" is marshalled using <see cref="VW.Serializer.StringProcessing.Split"/>.
        /// </summary>
        internal const string TextProperty = "_text";

        /// <summary>
        /// JSON property "_label" is used as label.
        /// </summary>
        internal const string LabelProperty = "_label";

        /// <summary>
        /// JSON property "_multi" is used to signal multi-line examples.
        /// </summary>
        internal const string MultiProperty = "_multi";

        internal static bool IsSpecialProperty(string property)
        {
            return property == TextProperty ||
                property == LabelProperty ||
                property == MultiProperty;
        }
    }
}
