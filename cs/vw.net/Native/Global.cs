using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;

namespace Vw.Net.Native {
    internal static partial class NativeMethods
    {
        public static readonly Encoding StringEncoding = Encoding.UTF8;
        public static readonly Func<IntPtr, string> StringMarshallingFunc = StringExtensions.PtrToStringUtf8;

        public const int SuccessStatus = 0; // See err_constants.h
        // public const int OpaqueBindingError = 39; // See err_contants.h

        public static IntPtr ToNativeHandleOrNullptrDangerous<TObject>(this NativeObject<TObject> nativeObject) where TObject : NativeObject<TObject>
        {
            if (nativeObject == null)
            {
                return IntPtr.Zero;
            }

            return nativeObject.DangerousGetHandle();
        }

        [DllImport("vw.net.native")]
        public static extern void FreeDupString(IntPtr dupString);

        [DllImport("vw.net.native")]
        public static extern ulong VwUniformHash(IntPtr key, UIntPtr len, ulong seed);

        public unsafe static ulong VwUniformHash(string key, ulong seed)
        {
            fixed (byte* keyBytes = NativeMethods.StringEncoding.GetBytes(key))
            {
                IntPtr keyPtr = new IntPtr(keyBytes);
                UIntPtr keyLen = new UIntPtr((uint)key.Length);
                
                return VwUniformHash(keyPtr, keyLen, seed);
            }
        }

        // [DllImport("vw.net.native")]
        // public static extern IntPtr LookupMessageForErrorCode(int error_code);

        // public static string MarshalMessageForErrorCode(int error_code)
        // {
        //     IntPtr nativeMessage = LookupMessageForErrorCode(error_code);
        //     return StringMarshallingFunc(nativeMessage);
        // }

        // public static readonly string OpaqueBindingErrorMessage = MarshalMessageForErrorCode(OpaqueBindingError);
    }
}
