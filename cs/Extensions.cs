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
        /// Reshuffles the the action dependent features based on indices returned by native space.
        /// </summary>
        /// <param name="example">The example used for prediction.</param>
        /// <param name="multiLabelPredictions">The indices used to reshuffle.</param>
        /// <returns>The action dependent features ordered by <paramref name="multiLabelPredictions"/></returns>
        public static T[] Subset<T>(this IEnumerable<T> that, int[] permutation)
        {
            // re-shuffle
            var result = new T[permutation.Length];
            var i = 0;
            foreach (var item in that)
	        {
               result[permutation[i]] = item; 
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
