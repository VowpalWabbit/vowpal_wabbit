// --------------------------------------------------------------------------------------------------------------------
// <copyright file="PropertyConfiguration.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
namespace VW.Serializer
{
    /// <summary>
    /// Constants used throughout C# wrapper.
    /// </summary>
    public sealed class PropertyConfiguration
    {
        /// <summary>
        /// Default value for feature ignore prefix: '_'.
        /// </summary>
        public const string FeatureIgnorePrefixDefault = "_";

        /// <summary>
        /// Default value for text property: '_text'.
        /// </summary>
        public const string TextPropertyDefault = "_text";

        /// <summary>
        /// Default value for label property: '_label'.
        /// </summary>
        public const string LabelPropertyDefault = "_label";

        /// <summary>
        /// Default value for label index property: '_labelindex'.
        /// </summary>
        public const string LabelIndexPropertyDefault = "_labelindex";

        /// <summary>
        /// Default value for label property prefix: '_label_';
        /// </summary>
        public const string LabelPropertyPrefixDefault = "_label_";

        /// <summary>
        /// Default value for multi property: '_multi'.
        /// </summary>
        public const string MultiPropertyDefault = "_multi";

        /// <summary>
        /// Default singleton holding the default configuration.
        /// </summary>
        public static readonly PropertyConfiguration Default = new PropertyConfiguration();

        /// <summary>
        /// Initializes a new <see cref="PropertyConfiguration"/> instance.
        /// </summary>
        public PropertyConfiguration()
        {
            this.FeatureIgnorePrefix = FeatureIgnorePrefixDefault;
            this.TextProperty = TextPropertyDefault;
            this.LabelProperty = LabelPropertyDefault;
            this.MultiProperty = MultiPropertyDefault;
            this.LabelIndexProperty = LabelIndexPropertyDefault;
            this.LabelPropertyPrefix = LabelPropertyPrefixDefault;
        }

        /// <summary>
        /// JSON properties starting with underscore are ignored.
        /// </summary>
        public string FeatureIgnorePrefix { get; set; }

        /// <summary>
        /// JSON property "_text" is marshalled using <see cref="VW.Serializer.StringProcessing.Split"/>.
        /// </summary>
        public string TextProperty { get; set; }

        /// <summary>
        /// JSON property "_label" is used as label.
        /// </summary>
        public string LabelProperty { get; set; }

        /// <summary>
        /// JSON property "_labelIndex" determines the index this label is applied for multi-line examples.
        /// </summary>
        public string LabelIndexProperty { get; set; }

        /// <summary>
        /// JSON properties starting with "_label_$name" are used to specify nested properties. Has the same effect as _label: { "$name": ... }.
        /// </summary>
        public string LabelPropertyPrefix { get; set; }

        /// <summary>
        /// JSON property "_multi" is used to signal multi-line examples.
        /// </summary>
        public string MultiProperty { get; set; }

        /// <summary>
        /// True if <paramref name="property"/> is considered a special property and thus should not be skipped.
        /// </summary>
        /// <param name="property">The JSON property name.</param>
        /// <returns>True if <paramref name="property"/> is a special property, false otherwise.</returns>
        public bool IsSpecialProperty(string property)
        {
            return property.Equals(TextProperty, StringComparison.OrdinalIgnoreCase) ||
                property.Equals(LabelProperty, StringComparison.OrdinalIgnoreCase) ||
                property.Equals(MultiProperty, StringComparison.OrdinalIgnoreCase) ||
                property.Equals(LabelIndexProperty, StringComparison.OrdinalIgnoreCase) ||
                property.StartsWith(LabelPropertyPrefixDefault, StringComparison.OrdinalIgnoreCase);
        }
    }
}
