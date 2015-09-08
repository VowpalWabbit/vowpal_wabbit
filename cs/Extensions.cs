using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW
{
    public static class Extensions
    {
        /// <summary>
        /// Returns the elements specified by indicies/
        /// </summary>
        /// <typeparam name="T">The element type.</typeparam>
        /// <param name="that">The enumerable source.</param>
        /// <param name="indicies">The indicies to be selected.</param>
        /// <returns>The subset of elements.</returns>
        public static T[] Subset<T>(this IEnumerable<T> that, int[] indicies)
        {
            // re-shuffle
            var result = new T[indicies.Length];
            var i = 0;
            foreach (var item in that)
	        {
               result[indicies[i]] = item; 
               i++;
	        }

            return result;
        }


        public static int IndexOf<T>(this IEnumerable<T> that, Predicate<T> predicate)
        {
            var i = 0;
            foreach (var t in that)
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
