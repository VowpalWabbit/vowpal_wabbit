// --------------------------------------------------------------------------------------------------------------------
// <copyright file="Extensions.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW
{
    /// <summary>
    /// LINQ extensions.
    /// </summary>
    public static class Extensions
    {
        /// <summary>
        /// Returns the elements specified by indicies/
        /// </summary>
        /// <typeparam name="T">The element type.</typeparam>
        /// <param name="source">The enumerable source.</param>
        /// <param name="indicies">The indicies to be selected.</param>
        /// <returns>The subset of elements.</returns>
        public static T[] Subset<T>(this IEnumerable<T> source, int[] indicies)
        {
            Contract.Requires(source != null);
            Contract.Requires(indicies != null);

            // re-shuffle
            var result = new T[indicies.Length];
            var i = 0;
            foreach (var item in source)
	        {
               result[indicies[i]] = item; 
               i++;
	        }

            return result;
        }

        /// <summary>
        /// Returns the index of the first element matching <paramref name="predicate"/>.
        /// </summary>
        /// <typeparam name="T">The collection type.</typeparam>
        /// <param name="source">The source enumerable.</param>
        /// <param name="predicate">The predicate to match.</param>
        /// <returns>The index of the first element to match or -1 if none matched.</returns>
        public static int IndexOf<T>(this IEnumerable<T> source, Predicate<T> predicate)
        {
            Contract.Requires(source != null);
            Contract.Requires(predicate != null);

            var i = 0;
            foreach (var t in source)
            {
                if (predicate(t))
                {
                    return i;
                }
                i++;
            }
            return -1;
        }
    }
}
