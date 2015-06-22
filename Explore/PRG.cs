using System.Runtime.InteropServices;

namespace MultiWorldTesting
{
    /// <summary>
    /// Translated implemetation of C++ random generator in explore-cpp.
    /// </summary>
    public class PRG
    {
        private const ulong A = 0xeece66d5deece66d;
        private const ulong C = 2147483647;

        private const int Bias = 127 << 23;

        private ulong V;

        /// <summary>
        /// Equivalent to C++ union containing int & float member.
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
        /// Constructs the random number generator using the default seed.
        /// </summary>
        public PRG()
        {
            this.V = C;
        }

        /// <summary>
        /// Constructs the random number generator using the specified seed.
        /// </summary>
        public PRG(ulong initial)
        {
            this.V = initial;
        }

        private float Merand48(ref ulong initial)
        {
            initial = A * initial + C;
            IntFloat temp;
            temp.F = 0f;
            temp.I = (int)(((initial >> 25) & 0x7FFFFF) | Bias);
            return temp.F - 1;
        }

        /// <summary>
        /// Returns a real number drawn uniformly from [0,1].
        /// </summary>
        /// <returns>The random number as a float.</returns>
        public float UniformUnitInterval()
        {
            return Merand48(ref V);
        }

        /// <summary>
        /// Returns an integer drawn uniformly from the specified interval.
        /// </summary>
        /// <param name="low">The inclusive start of the interval.</param>
        /// <param name="high">The inclusive end of the interval.</param>
        /// <returns>The random number as an unsigned integer.</returns>
        public uint UniformInt(uint low, uint high)
        {
            Merand48(ref V);
            uint ret = low + (uint)((V >> 25) % (high - low + 1));
            return ret;
        }
    }
}
