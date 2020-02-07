// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureExpression.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Linq.Expressions;
using System.Reflection;
using VW.Reflection;

namespace VW.Serializer
{
    /// <summary>
    /// Delegate defintion for feature object creation expressions.
    /// </summary>
    /// <param name="vw">An expression resolving to a VowpalWabbit instance.</param>
    /// <param name="namespace">An expression resolving to a Namespace instance.</param>
    /// <returns>An expression constructing a new Feature object.</returns>
    public delegate Expression NewFeatureExpressionDelegate(Expression vw, Expression @namespace);

    /// <summary>
    /// Feature data composed during compilation step.
    /// </summary>
    [DebuggerDisplay("FeatureExpression({Name})")]
    public sealed class FeatureExpression
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="FeatureExpression"/> class.
        /// </summary>
        /// <param name="featureType">The type of the feature.</param>
        /// <param name="name">The name of the feature.</param>
        /// <param name="valueExpressionFactory">Factory to extract the value for a given feature from the example object (input argument).</param>
        /// <param name="valueValidExpressionFactories">Factories to provide validation before invoking the expression created through <see cref="ValueExpressionFactory"/>.</param>
        /// <param name="featureExpressionFactory">The expression must create new Feature instances.</param>
        /// <param name="namespace">The namespace this feature belongs to.</param>
        /// <param name="featureGroup">The feature group this feature belongs to.</param>
        /// <param name="enumerize">If true the marshaller enumerates the feature (as in creates a 1-hot encoding).</param>
        /// <param name="variableName">The variable name to be used in the generated code.</param>
        /// <param name="order">Used to order feature serialization.</param>
        /// <param name="addAnchor">True if an anchor element should be added at the beginning of a dense feature array.</param>
        /// <param name="stringProcessing">Configures string pre-processing for this feature.</param>
        /// <param name="overrideSerializeMethod">An optional method overriding the otherwise auto-resolved serialization method.</param>
        /// <param name="dictify">True if a dictionary should be build for this feature.</param>
        /// <param name="parent">The parent feature expression.</param>
        public FeatureExpression(Type featureType,
            string name,
            Func<Expression, Expression> valueExpressionFactory,
            List<Func<Expression, Expression>> valueValidExpressionFactories = null,
            NewFeatureExpressionDelegate featureExpressionFactory = null,
            string @namespace = null,
            char? featureGroup = null,
            bool enumerize = false,
            string variableName = null,
            int? order = null,
            bool addAnchor = false,
            StringProcessing stringProcessing = StringProcessing.Split,
            MethodInfo overrideSerializeMethod = null,
            bool? dictify = null,
            FeatureExpression parent = null)
        {
            if (featureType == null)
                throw new ArgumentNullException("featureType");

            if (valueExpressionFactory == null)
                throw new ArgumentNullException("valueExpressionFactory");

            Contract.EndContractBlock();

            if(featureType.IsGenericType &&
               featureType.GetGenericTypeDefinition() == typeof(Nullable<>))
            {
                this.IsNullable = true;
                this.FeatureType = featureType.GetGenericArguments()[0];
            }
            else
            {
                this.IsNullable = false;
                this.FeatureType = featureType;
            }

            this.Name = name;
            this.ValueExpressionFactory = valueExpressionFactory;
            this.ValueValidExpressionFactories = valueValidExpressionFactories;
            this.FeatureExpressionFactory = featureExpressionFactory;
            this.Namespace = @namespace;
            this.FeatureGroup = featureGroup;
            this.Enumerize = enumerize;
            this.VariableName = variableName ?? name;
            this.Order = order ?? 1;
            this.AddAnchor = addAnchor;
            this.Dictify = dictify ?? false;
            this.StringProcessing = stringProcessing;
            this.OverrideSerializeMethod = overrideSerializeMethod;
            this.Dictify = dictify ?? false;
            this.Parent = parent;

            this.DenseFeatureValueElementType = InspectionHelper.GetEnumerableElementType(featureType);

            if (!InspectionHelper.IsNumericType(this.DenseFeatureValueElementType))
                this.DenseFeatureValueElementType = null;
        }

        /// <summary>
        /// The parent feature expression.
        /// </summary>
        public FeatureExpression Parent { get; private set; }

        /// <summary>
        /// True if the type is nullable.
        /// </summary>
        public bool IsNullable { get; set; }

        /// <summary>
        /// Serializer variable name.
        /// </summary>
        /// <remarks>Useful to debug</remarks>
        public string VariableName { get; set; }

        /// <summary>
        /// The type of the feature.
        /// </summary>
        public Type FeatureType { get; private set; }

        internal Type IntermediateFeatureType { get; set; }

        /// <summary>
        /// The name of the feature.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// The namespace.
        /// </summary>
        public string Namespace { get; set; }

        /// <summary>
        /// The feature group.
        /// </summary>
        public char? FeatureGroup { get; set; }

        /// <summary>
        /// An optional method overriding the otherwise auto-resolved serialization method.
        /// </summary>
        public MethodInfo OverrideSerializeMethod { get; set; }

        /// <summary>
        /// True if this feature should be enumerized.
        /// </summary>
        public bool Enumerize { get; set; }

        /// <summary>
        /// True if an anchor element should be added at the beginning of a dense feature array.
        /// </summary>
        public bool AddAnchor { get; set; }

        /// <summary>
        /// True if a dictionary should be build for this feature.
        /// </summary>
        public bool Dictify { get; set; }

        /// <summary>
        /// Factory to extract the value for a given feature from the example object (input argument).
        /// </summary>
        public Func<Expression, Expression> ValueExpressionFactory { get; set; }

        /// <summary>
        /// Factories to provide validation before invoking the expression created through <see cref="ValueExpressionFactory"/>.
        /// </summary>
        public List<Func<Expression, Expression>> ValueValidExpressionFactories { get; set; }

        /// <summary>
        /// The expression must create new Feature instances.
        /// </summary>
        public NewFeatureExpressionDelegate FeatureExpressionFactory { get; set; }

        /// <summary>
        /// The element type of an enumerable feature type.
        /// </summary>
        public Type DenseFeatureValueElementType { get; set; }

        /// <summary>
        /// Used to order feature serialization.
        /// </summary>
        public int Order { get; set; }

        /// <summary>
        /// Configures string pre-processing for this feature.
        /// </summary>
        public StringProcessing StringProcessing { get; set; }
    }
}
