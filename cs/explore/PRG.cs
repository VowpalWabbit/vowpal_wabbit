using System.Runtime.InteropServices;

namespace Microsoft.DecisionService.Exploration
{
    /// <summary>
    /// Translated implemetation of C++ random generator in explore-cpp.
    /// </summary>
    public static class PRG
    {
        private const ulong A = 0xeece66d5deece66d;
        private const ulong C = 2147483647;
        private const int Bias = 127 << 23;

        /// <summary>
        /// Equivalent to C++ union containing int and float member.
        /// </summary>
        [StructLayout(LayoutKind.Explicit)]
        private struct IntFloat
        {
            [FieldOffset(0)]
            public int I;

            [FieldOffset(0)]
            public float F;
        }

        /// <summary>
        /// Returns a real number drawn uniformly from [0,1].
        /// </summary>
        /// <returns>The random number as a float.</returns>
        /// <remarks>Implements Merand48</remarks>
        public static float UniformUnitInterval(ulong seed)
        {
            seed = A * seed + C;
            IntFloat temp;
            temp.F = 0f;
            temp.I = (int)(((seed >> 25) & 0x7FFFFF) | Bias);
            return temp.F - 1;
        }
    }
}
