using System;
using System.Runtime.CompilerServices;

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
        public static ulong ComputeIdHash(string str)
        {
            uint length = 0;
            uint seed = 0;

            uint h1 = seed;

            byte[] ring = new byte[8];
            var pos1 = 0;
            var pos2 = 0;

            for (int i = 0; i < str.Length;)
            {
                ulong codePoint = str[i++];
                
                // Convert to Unicode code point
                // More information can be found at: https://en.wikipedia.org/wiki/UTF-16 
                if ((codePoint & 0xFC00) == 0xD800)
                { 
                    // Equiv. range check between 0xD800 to 0xDBFF inclusive
                    var code_unit_2 = str[i];
                    if ((code_unit_2 & 0xFC00) == 0xDC00) // Equiv. range check between 0xDC00 to 0xDFFF inclusive
                    {
                        codePoint = (codePoint << 10) + code_unit_2 - 0x35FDC00;
                        i++;
                    }
                }

                // Convert to UTF-8 bytes 
                if (codePoint < 0x80)
                {
                    ring[pos2] = (byte)codePoint;
                    pos2 = (pos2 + 1) & 7;
                    
                    Hash(ring, ref pos1, pos2, ref h1);
                    
                    length++;
                }
                else if (codePoint <= 0x7FF)
                {
                    ring[pos2] = (byte)((codePoint >> 6) + 0xC0);
                    pos2 = (pos2 + 1) & 7;
                    
                    ring[pos2] = (byte)((codePoint & 0x3F) + 0x80);
                    pos2 = (pos2 + 1) & 7;
                    
                    Hash(ring, ref pos1, pos2, ref h1);
                    
                    length += 2;
                }
                else if (codePoint <= 0xFFFF)
                {
                    ring[pos2] = (byte)((codePoint >> 12) + 0xE0);
                    pos2 = (pos2 + 1) & 7;
                    
                    ring[pos2] = (byte)(((codePoint >> 6) & 0x3F) + 0x80);
                    pos2 = (pos2 + 1) & 7;
                    
                    ring[pos2] = (byte)((codePoint & 0x3F) + 0x80);
                    pos2 = (pos2 + 1) & 7;
                    
                    Hash(ring, ref pos1, pos2, ref h1);
                    
                    length += 3;
                }
                else if (codePoint <= 0x10FFFF)
                {
                    ring[pos2] = (byte)((codePoint >> 18) + 0xF0);
                    pos2 = (pos2 + 1) & 7;
                    
                    ring[pos2] = (byte)(((codePoint >> 12) & 0x3F) + 0x80);
                    pos2 = (pos2 + 1) & 7;
                    
                    ring[pos2] = (byte)(((codePoint >> 6) & 0x3F) + 0x80);
                    pos2 = (pos2 + 1) & 7;
                    
                    ring[pos2] = (byte)((codePoint & 0x3F) + 0x80);
                    pos2 = (pos2 + 1) & 7;
                    
                    Hash(ring, ref pos1, pos2, ref h1);
                    
                    length += 4;
                }
                else
                {
                    throw new ArgumentException("Invalid codepoint: " + codePoint);
                }
            }

            uint k1 = 0;
            var ringLength = (pos2 - pos1 + 8) & 7;

            switch (ringLength)
            {
                case 3:
                    k1 ^= (uint)(ring[(pos1 + 2) & 7] << 16);
                    goto case 2;
                case 2:
                    k1 ^= (uint)(ring[(pos1 + 1) & 7] << 8);
                    goto case 1;
                case 1:
                    k1 ^= (uint)(ring[pos1]);
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

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private static void Hash(byte[] ring, ref int pos1, int pos2, ref uint h1)
        {
            var length = (pos2 - pos1 + 8) & 7;
            if (length < 4)
            {
                return;
            }

            // convert byte array to integer
            var k1 = (uint)ring[pos1];
            pos1 = (pos1 + 1) & 7;
            k1 |= (uint)ring[pos1] << 8;
            pos1 = (pos1 + 1) & 7;
            k1 |= (uint)ring[pos1] << 16;
            pos1 = (pos1 + 1) & 7;
            k1 |= (uint)ring[pos1] << 24;
            pos1 = (pos1 + 1) & 7;

            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;

            h1 ^= k1;
            h1 = rotl32(h1, 13);
            h1 = h1 * 5 + 0xe6546b64;
        }
    }
}
