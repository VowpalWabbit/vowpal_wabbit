using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace MultiWorldTesting
{
    /// <summary>
    /// Translated implemetation of C++ murmurhash3 in explore-cpp.
    /// </summary>
    public static class MurMurHash3
    {
        private const uint c1 = 0xcc9e2d51;
        private const uint c2 = 0x1b873593;

        /// <summary>
        /// Computes hash code for the specified string or converts it to integer if applicable.
        /// </summary>
        /// <param name="s">The string to hash.</param>
        /// <returns>The computed hash code or converted integer for the string.</returns>
        public static ulong ComputeIdHash(string s)
        {
            int sInt = 0;
            if (int.TryParse(s, out sInt))
            {
                return (ulong)sInt;
            }
            else
            {
                // get raw bytes from string
                byte[] keys = System.Text.Encoding.UTF8.GetBytes(s);

                uint h1 = 0;
                uint k1 = 0;

                int length = keys.Length;
                int i = 0;
                while (i <= length - 4)
                {
                    // convert byte array to integer
                    k1 = (uint)(keys[i] | keys[i + 1] << 8 | keys[i + 2] << 16 | keys[i + 3] << 24);

                    k1 *= c1;
                    k1 = rotl32(k1, 15);
                    k1 *= c2;

                    h1 ^= k1;
                    h1 = rotl32(h1, 13);
                    h1 = h1 * 5 + 0xe6546b64;

                    i += 4;
                }

                k1 = 0;
                int tail = length - length % 4;
                switch (length & 3)
                {
                    case 3:
                        k1 ^= (uint)(keys[tail + 2] << 16);
                        goto case 2;
                    case 2:
                        k1 ^= (uint)(keys[tail + 1] << 8);
                        goto case 1;
                    case 1:
                        k1 ^= (uint)(keys[tail]);
                        k1 *= c1;
                        k1 = rotl32(k1, 15);
                        k1 *= c2;
                        h1 ^= k1;
                        break;
                }

                // finalization
                h1 ^= (uint)length;

                return fmix(h1);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static uint rotl32(uint x, byte r)
        {
            return (x << r) | (x >> (32 - r));
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static uint fmix(uint h)
        {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return h;
        }
    }
}
