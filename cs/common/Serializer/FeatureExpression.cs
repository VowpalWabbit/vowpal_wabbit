// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureExpression.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq.Expressions;
using System.Reflection;
using VW.Reflection;

namespace VW.Serializer
{
    public delegate Expression NewFeatureExpressionDelegate(Expression vw, Expression @namespace);

    /// <summary>
    /// Feature data composed during compilation step.
    /// </summary>
    public sealed class FeatureExpression
    {
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
            bool? dictify = null)
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

            this.DenseFeatureValueElementType = InspectionHelper.GetDenseFeatureValueElementType(featureType);
        }

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
        /// The Name of the feature.
        /// </summary>
        public string Name { get; set; }

        public string Namespace { get; set; }

        public char? FeatureGroup { get; set; }

        public MethodInfo OverrideSerializeMethod { get; set; }

        public bool Enumerize { get; set; }

        public bool AddAnchor { get; set; }

        public bool Dictify { get; set; }

        /// <summary>
        /// Factory to extract the value for a given feature from the example object (input argument).
        /// </summary>
        public Func<Expression, Expression> ValueExpressionFactory { get; set; }

        /// <summary>
        /// Factories to provide validation before invoking the expression created through <see cref="ValueExpressionFactory"/>.
        /// </summary>
        public List<Func<Expression, Expression>> ValueValidExpressionFactories { get; set; }

        public NewFeatureExpressionDelegate FeatureExpressionFactory { get; set; }

        public Type DenseFeatureValueElementType { get; set; }

        public int Order { get; set; }

        public StringProcessing StringProcessing { get; set; }
    }
}
