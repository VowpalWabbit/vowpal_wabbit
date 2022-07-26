using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Vw.Net.Native
{
    internal static partial class NativeMethods
    {
        [DllImport("vw.net.native")]
        public static extern UIntPtr StdStringGetLength(IntPtr str);

        [DllImport("vw.net.native")]
        public static extern int StdStringCopyToBuffer(IntPtr str, IntPtr buffer, int buffer_length);
    }

    internal static class StringExtensions
    {
        internal static int GetUtf8ByteCount(this IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
            {
                throw new ArgumentNullException("ptr", "Trying to get null-terminated byte-count of nullptr");
            }

            // This can run into issues with strings that are > IntMax in size. Unlikely to actually be an issue, though.
            int length = 0;

            // TODO: Decide whether overhead of checked() here is worth it
            while (Marshal.ReadByte(ptr, length) != 0)
            {
                length = checked(length + 1);
            }

            return length;
        }

        unsafe internal static string PtrToStringUtf8(this IntPtr intPtr)
        {
            if (intPtr == IntPtr.Zero)
            {
                return String.Empty; 
            }

            Encoding utf8 = Encoding.UTF8;

            // Go through the string twice - would be nice to do it all at once, but that is non-trivial for some reason.
            int byteCount = intPtr.GetUtf8ByteCount();

            if (byteCount == 0)
            {
                return String.Empty;
            }

            string result = utf8.GetString((byte*)intPtr.ToPointer(), byteCount);

            return result;
        }
    }
}
