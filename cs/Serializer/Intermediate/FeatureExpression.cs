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
using VW.Serializer.Inspectors;
using VW.Serializer.Reflection;

namespace VW.Serializer.Intermediate
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
            NewFeatureExpressionDelegate featureExpressionFactory = null,
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

            // actually it's optional for custom types
            //if (string.IsNullOrEmpty(name))
            //    throw new ArgumentNullException("name");

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
            this.FeatureExpressionFactory = featureExpressionFactory;
            this.Namespace = @namespace;
            this.FeatureGroup = featureGroup;
            this.Enumerize = enumerize;
            this.VariableName = variableName ?? name;
            this.Order = order ?? 1;
            this.AddAnchor = addAnchor;
            this.OverrideSerializeMethod = overrideSerializeMethod;

            this.DenseFeatureValueElementType = InspectionHelper.GetDenseFeatureValueElementType(featureType);
            this.IsDense = this.DenseFeatureValueElementType != null;

            this.IsNumeric = InspectionHelper.IsNumericType(this.FeatureType) ||
                InspectionHelper.IsNumericType(InspectionHelper.GetDenseFeatureValueElementType(this.FeatureType));

            //// handle nullable features
            //if (this.FeatureType.IsEnum)
            //{
            //    this.IsEnum = true;
            //    this.EnumType = this.FeatureType;
            //}
            //else if(this.FeatureType.IsGenericType &&
            //        this.FeatureType.GetGenericTypeDefinition() == typeof(Nullable<>) &&
            //        .IsEnum)
            //{
            //    this.IsEnum = true;
            //    this.EnumType = this.FeatureType.GetGenericArguments()[0];
            //}
        }

        public bool IsNullable { get; private set; }

        /// <summary>
        /// Serializer variable name.
        /// </summary>
        /// <remarks>Useful to debug</remarks>
        public string VariableName { get; private set; }

        /// <summary>
        /// The type of the feature.
        /// </summary>
        public Type FeatureType { get; private set; }

        //public Type EnumType { get; private set; }

        internal Type IntermediateFeatureType { get; private set; }

        /// <summary>
        /// The Name of the feature.
        /// </summary>
        public string Name { get; private set; }

        public string Namespace { get; private set; }

        public char? FeatureGroup { get; private set; }

        public MethodInfo OverrideSerializeMethod { get; private set; }

        public bool IsDense { get; private set; }

        //public bool IsEnum { get; private set; }

        public bool Enumerize { get; private set; }

        public bool AddAnchor { get; private set; }

        public Func<Expression, Expression> ValueExpressionFactory { get; private set; }

        public NewFeatureExpressionDelegate FeatureExpressionFactory { get; private set; }

        public Type DenseFeatureValueElementType { get; private set; }

        public int Order { get; private set; }

        public bool IsNumeric { get; private set; }

        //internal MemberInitExpression CreateFeatureExpression(Expression valueExpression)
        //{
        //    var e = this.ValueExpressionFactory(valueExpression);

        //    // CODE new Feature<T> { Namespace = ..., ... }
        //    return Expression.MemberInit(
        //            Expression.New(IntermediateFeatureType),
        //            Expression.Bind(ReflectionHelper.GetInfo((MetaFeature f) => f.Name), Expression.Constant(this.Name, typeof(string))),
        //            Expression.Bind(ReflectionHelper.GetInfo((MetaFeature f) => f.Enumerize), Expression.Constant(this.Enumerize)),
        //            Expression.Bind(ReflectionHelper.GetInfo((MetaFeature f) => f.AddAnchor), Expression.Constant(this.AddAnchor)),
        //            //Expression.Bind(IntermediateFeatureType.GetProperty("Value"), e),
        //            Expression.Bind(ReflectionHelper.GetInfo((MetaFeature f) => f.Namespace), Expression.Constant(this.Namespace, typeof(string))),
        //            Expression.Bind(ReflectionHelper.GetInfo((MetaFeature f) => f.FeatureGroup),
        //                this.FeatureGroup == null ? (Expression)Expression.Constant(null, typeof(char?)) :
        //                Expression.New((ConstructorInfo)ReflectionHelper.GetInfo((char v) => new char?(v)), Expression.Constant((char)this.FeatureGroup)))
        //            );
        //}
    }
}
