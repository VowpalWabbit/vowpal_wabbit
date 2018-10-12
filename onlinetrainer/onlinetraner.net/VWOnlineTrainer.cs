using OnlineTrainer.Net.Native;
using System;
using System.Runtime.InteropServices;

namespace OnlineTrainer.Net
{
    public sealed class VWOnlineTrainer : NativeObject<VWOnlineTrainer>
    {
        [DllImport("onlinetrainer.net.native.dll")]
        private static extern IntPtr CreateVWOnlineTrainer(string arg);

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void DeleteVWOnlineTrainer(IntPtr vwOnlineTrainer);

        private static New<VWOnlineTrainer> BindConstructorArguments(string arg)
        {
            return new New<VWOnlineTrainer>(() => CreateVWOnlineTrainer(arg));
        }

        private delegate void managed_callback_t(IntPtr apiStatus);

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void VWOnlineTrainerSetCallback(IntPtr vwOnlineTrainer, [MarshalAs(UnmanagedType.FunctionPtr)] managed_callback_t callback = null);

        private readonly managed_callback_t managedCallback;

        public VWOnlineTrainer(string arg) : base(BindConstructorArguments(arg), new Delete<VWOnlineTrainer>(DeleteVWOnlineTrainer))
        {
            managedCallback = new managed_callback_t(WrapStatusAndRaiseBackgroundError);
        }

        private void WrapStatusAndRaiseBackgroundError(IntPtr apiStatusHandle)
        {
            ApiStatus status = new ApiStatus(apiStatusHandle);

            BackgroundErrorInternal?.Invoke(this, status);
        }

        private event EventHandler<ApiStatus> BackgroundErrorInternal;
    }
}
