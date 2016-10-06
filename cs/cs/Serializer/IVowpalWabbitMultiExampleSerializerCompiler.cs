using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VW.Serializer
{
    /// <summary>
    /// Interface for multi-example serializer compilers.
    /// </summary>
    /// <typeparam name="TExample">The example type.</typeparam>
    public interface IVowpalWabbitMultiExampleSerializerCompiler<TExample>
    {
        /// <summary>
        /// Returns the number of action dependent examples <paramref name="example"/> holds.
        /// </summary>
        /// <param name="example">The example to inspect.</param>
        /// <returns>Returns the number of action dependent examples <paramref name="example"/> holds.</returns>
        int GetNumberOfActionDependentExamples(TExample example);
    }
}
