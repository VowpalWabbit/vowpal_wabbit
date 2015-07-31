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
        public static T[] Permutate<T>(this IReadOnlyCollection<T> that, int[] permutation)
        {
            // re-shuffle
            var result = new T[that.Length];
            for (var i = 0; i < that.Length; i++)
            {
                // VW multi-label indicies are 0-based
                result[i] = example.ActionDependentFeatures[permutation[i]];
            }

            return result;
        }
    }
}
