using OnlineTrainer.Net.Native;
using System;
using System.Runtime.InteropServices;

namespace OnlineTrainer.Net
{
    public sealed class VWOnlineTrainer : NativeObject<VWOnlineTrainer>
    {
        [DllImport("onlinetrainer.net.native.dll")]
        private static extern IntPtr CreateVWOnlineTrainer(string arg, byte[] model, long length);

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void DeleteVWOnlineTrainer(IntPtr vwOnlineTrainer);

        public VWOnlineTrainer(string arg, byte[] model = null) : base(BindConstructorArguments(arg, model), new Delete<VWOnlineTrainer>(DeleteVWOnlineTrainer))
        {
            managedCallback = new managed_callback_t(WrapStatusAndRaiseBackgroundError);
        }

        #region Callback
        private static New<VWOnlineTrainer> BindConstructorArguments(string arg, byte[] model)
        {
            return new New<VWOnlineTrainer>(() => CreateVWOnlineTrainer(arg, model, model == null? 0 : model.Length));
        }

        private delegate void managed_callback_t(IntPtr apiStatus);

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void VWOnlineTrainerSetCallback(IntPtr vwOnlineTrainer, [MarshalAs(UnmanagedType.FunctionPtr)] managed_callback_t callback = null);

        private readonly managed_callback_t managedCallback;

        private void WrapStatusAndRaiseBackgroundError(IntPtr apiStatusHandle)
        {
            ApiStatus status = new ApiStatus(apiStatusHandle);

            BackgroundErrorInternal?.Invoke(this, status);
        }

        private event EventHandler<ApiStatus> BackgroundErrorInternal;
        #endregion

        #region P/Invoke APIs
        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void SetModel(IntPtr onlineTrainer, byte[] model, long length);
        #endregion
    }
}
