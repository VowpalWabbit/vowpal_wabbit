using System;
using System.Threading;
using System.Runtime.InteropServices;

namespace Rl.Net.Native {
    internal static class NativeMethods
    {
        public const UnmanagedType StringMarshalling = UnmanagedType.LPStr; // TODO - where did UTF marshalling go?
        public const int SuccessStatus = 0; // See err_constants.h

        public static IntPtr ToNativeHandleOrNullptr<TObject>(this NativeObject<TObject> nativeObject) where TObject : NativeObject<TObject>
        {
            if (nativeObject == null)
            {
                return IntPtr.Zero;
            }

            return nativeObject.NativeHandle;
        }
    }
}