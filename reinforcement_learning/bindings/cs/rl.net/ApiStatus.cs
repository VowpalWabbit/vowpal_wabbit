using System;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net {
    public sealed class ApiStatus: NativeObject<ApiStatus>
    {
        [DllImport("rl.net.native.dll")]
        private static extern IntPtr CreateApiStatus();

        [DllImport("rl.net.native.dll")]
        private static extern void DeleteApiStatus(IntPtr config);

        [DllImport("rl.net.native.dll")]
        [return: MarshalAs(NativeMethods.StringMarshalling)]
        private static extern string GetApiStatusErrorMessage(IntPtr status);
        
        [DllImport("rl.net.native.dll")]
        private static extern int GetApiStatusErrorCode(IntPtr status);

        public ApiStatus() : base(new New<ApiStatus>(CreateApiStatus), new Delete<ApiStatus>(DeleteApiStatus))
        {
        }

        public int ErrorCode => GetApiStatusErrorCode(this.handle);

        public string ErrorMessage => GetApiStatusErrorMessage(this.handle);
    }
}