// --------------------------------------------------------------------------------------------------------------------
// <copyright file="InspectionHelper.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;

namespace VW.Serializer.Inspectors
{
    public static class InspectionHelper
    {
        public static bool IsNumericType(Type elemType)
        {
            return elemType == typeof(double)
                    || elemType == typeof(float)
                    || elemType == typeof(byte)
                    || elemType == typeof(sbyte)
                    || elemType == typeof(char)
                    || elemType == typeof(decimal)
                    || elemType == typeof(UInt16)
                    || elemType == typeof(UInt32)
                    || elemType == typeof(UInt64)
                    || elemType == typeof(Int16)
                    || elemType == typeof(Int32)
                    || elemType == typeof(Int64);
        }

        public static Type GetDenseFeatureValueElementType(Type type)
        {
            if (type.IsArray)
            {
                var elemType = type.GetElementType();

                // numeric types
                if (IsNumericType(elemType))
                {
                    return elemType;
                }
            }

            if (typeof(IEnumerable<object>).IsAssignableFrom(type))
            {
                // let's get T of IEnumerable<T>
                var elemType = type.GetInterfaces().Union(new[] { type })
                    .First(it => it.IsGenericType && it.GetGenericTypeDefinition() == typeof(IEnumerable<>))
                    .GetGenericArguments()[0];

                if (IsNumericType(elemType))
                {
                    return elemType;
                }
            }

            return null;
        }
    }
}
