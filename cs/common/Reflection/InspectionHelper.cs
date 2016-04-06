// --------------------------------------------------------------------------------------------------------------------
// <copyright file="InspectionHelper.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;

namespace VW.Reflection
{
    public static class InspectionHelper
    {
        public static bool IsNumericType(Type elemType)
        {
            return IsNumericTypeInternal(elemType) ||
                   (elemType != null
                    && elemType.IsGenericType
                    &&  elemType.GetGenericTypeDefinition() == typeof(Nullable<>)
                    && IsNumericTypeInternal(elemType.GetGenericArguments()[0]));
        }

        private static bool IsNumericTypeInternal(Type elemType)
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

        public static Type GetEnumerableElementType(Type type)
        {
            Contract.Requires(type != null);

            if (type.IsArray)
                return type.GetElementType();

            var enumerableType = type.GetInterfaces().Union(new[] { type })
                    .FirstOrDefault(it => it.IsGenericType && it.GetGenericTypeDefinition() == typeof(IEnumerable<>));

            // let's get T of IEnumerable<T>
            if (enumerableType != null)
                return enumerableType.GetGenericArguments()[0];

            return null;
        }
    }
}
