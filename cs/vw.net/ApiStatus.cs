using System;
using System.Threading;
using System.Runtime.InteropServices;

using Vw.Net.Native;
using System.Text;

namespace Vw.Net {


    public sealed class ApiStatus : NativeObject<ApiStatus>
    {
        [DllImport("vw.net.native")]
        private static extern IntPtr CreateApiStatus();

        [DllImport("vw.net.native")]
        private static extern void DeleteApiStatus(IntPtr config);

        [DllImport("vw.net.native")]
        private static extern IntPtr GetApiStatusErrorMessage(IntPtr status);

        [DllImport("vw.net.native")]
        private static extern int GetApiStatusErrorCode(IntPtr status);

        // [DllImport("vw.net.native")]
        // private static extern void UpdateApiStatusSafe(IntPtr status, int error_code, IntPtr message);
        
        // [DllImport("vw.net.native")] 
        // private static extern void ClearApiStatusSafe(IntPtr status);

        public ApiStatus() : base(new New<ApiStatus>(CreateApiStatus), new Delete<ApiStatus>(DeleteApiStatus))
        {
        }

        internal ApiStatus(IntPtr sharedApiStatusHandle) : base(sharedApiStatusHandle, ownsHandle: false)
        {
        }

        public int ErrorCode
        {
            get
            {
                int result = GetApiStatusErrorCode(this.DangerousGetHandle());

                GC.KeepAlive(this);
                return result;
            }
        } 

        public string ErrorMessage
        {
            get
            {
                IntPtr errorMessagePtr = GetApiStatusErrorMessage(this.DangerousGetHandle());

                // We cannot rely on P/Invoke's marshalling here, because it assumes that it can deallocate the string
                // it receives, after converting it to a managed string. We cannot do this, in this case.
                string result = NativeMethods.StringMarshallingFunc(errorMessagePtr);

                GC.KeepAlive(this);
                return result;
            }
        }
    }

}