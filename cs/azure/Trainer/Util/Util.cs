// --------------------------------------------------------------------------------------------------------------------
// <copyright file="MyArrayPool.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Buffers;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VowpalWabbit.Azure.Trainer
{
    public class MyArrayPool : IArrayPool<char>
    {
        private readonly ArrayPool<char> pool;

        public MyArrayPool()
        {
            this.pool = ArrayPool<char>.Create();
        }

        public char[] Rent(int minimumLength)
        {
            return this.pool.Rent(minimumLength);
        }

        public void Return(char[] array)
        {
            this.pool.Return(array);
        }
    }

   
    public static class Util
    {
        public static unsafe bool ReadDoubleString(char[] chars, int offset, int length, out double acc)
        {
            // TODO: could further optimize if stringReference has space for 1 additional char
            // safe p != end check
            fixed (char* po = chars)
            {
                char* p = po + offset;
                acc = 0;

                char* end = p + length;

                // skip white space TODO: other
                for (; *p == ' ' && p != end; p++) ;

                if (p == end)
                    return false;

                int s = 1;
                if (*p == '-')
                {
                    s = -1;
                    p++;
                }

                for (; p != end && *p >= '0' && *p <= '9'; p++)
                    acc = acc * 10 + (*p - '0');

                if (p == end)
                {
                    acc *= s;
                    return true;
                }

                int num_dec = 0;
                int exp_acc = 0;

                if (*p == '.')
                {
                    p++;
                    for (; p != end && *p >= '0' && *p <= '9'; p++)
                    {
                        if (num_dec < 35)
                        {
                            acc = acc * 10 + (*p - '0');
                            num_dec++;
                        }
                    }

                    if (p == end)
                        goto done;
                }

                if (*p == 'e' || *p == 'E')
                {
                    p++;
                    if (p == end)
                        goto done;

                    int exp_s = 1;
                    if (*p == '-')
                    {
                        exp_s = -1; p++;
                    }
                    for (; p != end && *p >= '0' && *p <= '9'; p++)
                        exp_acc = exp_acc * 10 + (*p - '0');
                    exp_acc *= exp_s;
                }

                if (p != end)//easy case succeeded.
                {
                    string str = new string(chars, offset, length);
                    return double.TryParse(str, NumberStyles.Float | NumberStyles.AllowThousands, CultureInfo.InvariantCulture, out acc);
                }

                done:

                int exponent = exp_acc - num_dec;
                float exp_val; // = 10;
                               //if (exponent > 0)
                               //{
                               //    int result = 1;
                               //    while (exponent != 0)
                               //    {
                               //        if ((exponent & 1) == 1)
                               //        {
                               //            result *= exp_val;
                               //        }
                               //        exponent >>= 1;
                               //        exp_val *= exp_val;
                               //    }
                               //}
                               //else
                exp_val = (int)Math.Pow(10, exponent);

                //if (exponent > 0)
                //{
                //    exp_val = 1;
                //    while (exponent-- > 0)
                //    {
                //        exp_val *= 10;
                //    }
                //}
                //else
                //{
                //    exp_val = 1;
                //    while (exponent++ < 0)
                //    {
                //        exp_val /= 10;
                //    }
                //}

                acc *= s * exp_val;
                return true;
            }
        }
    }
}
