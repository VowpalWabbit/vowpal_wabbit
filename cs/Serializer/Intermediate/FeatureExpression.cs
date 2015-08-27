// --------------------------------------------------------------------------------------------------------------------
// <copyright file="FeatureExpression.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Diagnostics.Contracts;
using System.Linq.Expressions;
using System.Reflection;
using VW.Serializer.Inspectors;
using VW.Serializer.Reflection;

namespace VW.Serializer.Intermediate
{
    /// <summary>
    /// Feature data composed during compilation step.
    /// </summary>
    public sealed class FeatureExpression
    {
        public FeatureExpression(Type featureType, 
            string name, 
            Func<Expression, Expression> valueExpressionFactory, 
            string @namespace = null, 
            char? featureGroup = null, 
            bool enumerize = false, 
            string variableName = null, 
            int? order = null,
            bool addAnchor = false,
            MethodInfo overrideSerializeMethod = null)
        {
            if (featureType == null)
                throw new ArgumentNullException("featureType");

            if (name == null)
                throw new ArgumentNullException("name");

            if (valueExpressionFactory == null)
                throw new ArgumentNullException("valueExpressionFactory");

            Contract.EndContractBlock();

            this.FeatureType = featureType;
            this.Name = name;
            this.ValueExpressionFactory = valueExpressionFactory;
            this.Namespace = @namespace;
            this.FeatureGroup = featureGroup;
            this.Enumerize = enumerize;
            this.VariableName = variableName ?? name;
            this.Order = order ?? 1;
            this.AddAnchor = addAnchor;
            this.OverrideSerializeMethod = overrideSerializeMethod;

            this.DenseFeatureValueElementType = InspectionHelper.GetDenseFeatureValueElementType(featureType);
            this.IsDense = this.DenseFeatureValueElementType != null;
            this.IntermediateFeatureType = typeof(Feature<>).MakeGenericType(featureType);
        }

        /// <summary>
        /// Serializer variable name.
        /// </summary>
        /// <remarks>Useful to debug</remarks>
        public string VariableName { get; private set; }

        /// <summary>
        /// The type of the feature.
        /// </summary>
        public Type FeatureType { get; private set; }

        internal Type IntermediateFeatureType { get; private set; }

        /// <summary>
        /// The Name of the feature.
        /// </summary>
        public string Name { get; private set; }

        public string Namespace { get; private set; }

        public char? FeatureGroup { get; private set; }

        public MethodInfo OverrideSerializeMethod { get; private set; }

        public bool IsDense { get; private set; }

        public bool Enumerize { get; private set; }

        public bool AddAnchor { get; private set; }

        public Func<Expression, Expression> ValueExpressionFactory { get; private set; }

        public Type DenseFeatureValueElementType { get; private set; }

        public int Order { get; private set; }

        internal MemberInitExpression CreateFeatureExpression(Expression valueExpression)
        {

            // CODE new Feature<T> { Namespace = ..., ... } 
            return Expression.MemberInit(
                    Expression.New(IntermediateFeatureType),
                    Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Name), Expression.Constant(this.Name)),
                    Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Enumerize), Expression.Constant(this.Enumerize)),
                    Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.AddAnchor), Expression.Constant(this.AddAnchor)),
                    Expression.Bind(IntermediateFeatureType.GetProperty("Value"), this.ValueExpressionFactory(valueExpression)),
                    Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.Namespace), Expression.Constant(this.Namespace, typeof(string))),
                    Expression.Bind(ReflectionHelper.GetInfo((Feature f) => f.FeatureGroup),
                        this.FeatureGroup == null ? (Expression)Expression.Constant(null, typeof(char?)) :
                        Expression.New((ConstructorInfo)ReflectionHelper.GetInfo((char v) => new char?(v)), Expression.Constant((char)this.FeatureGroup)))
                    );
        }
    }
}
