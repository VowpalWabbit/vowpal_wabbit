using OnlineTrainer.Net.Native;
using System;
using System.Runtime.InteropServices;

namespace OnlineTrainer.Net
{
    public sealed class ApiStatus : NativeObject<ApiStatus>
    {
        [DllImport("onlinetrainer.net.native.dll")]
        private static extern IntPtr CreateApiStatus();

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void DeleteApiStatus(IntPtr config);

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern IntPtr GetApiStatusErrorMessage(IntPtr status);

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern int GetApiStatusErrorCode(IntPtr status);

        public ApiStatus() : base(new New<ApiStatus>(CreateApiStatus), new Delete<ApiStatus>(DeleteApiStatus))
        {
        }

        internal ApiStatus(IntPtr sharedApiStatusHandle) : base(sharedApiStatusHandle, ownsHandle: false)
        {
        }

        public int ErrorCode => GetApiStatusErrorCode(this.handle);

        public string ErrorMessage
        {
            get
            {
                IntPtr errorMessagePtr = GetApiStatusErrorMessage(this.handle);

                // We cannot rely on P/Invoke's marshalling here, because it assumes that it can deallocate the string
                // it receives, after converting it to a managed string. We cannot do this, in this case.

                return NativeMethods.StringMarshallingFunc(errorMessagePtr);
            }
        }
    }
}
