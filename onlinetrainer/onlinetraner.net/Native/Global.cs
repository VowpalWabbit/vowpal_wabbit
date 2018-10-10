using System;
using System.Runtime.InteropServices;

namespace OnlineTrainer.Net
{
    internal static class NativeMethods
    {
        // TODO - UTF Marshalling (exists in NetCoreApp2.1 and Net[FrameworkApp]462, but not NetStandard2.0, and there is not yet a 2.1)
        public const UnmanagedType StringMarshalling = UnmanagedType.LPStr;
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
