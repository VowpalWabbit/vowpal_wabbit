using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace OnlineTrainer.Net
{
    public sealed class VWOnlineTrainer : NativeObject<VWOnlineTrainer>
    {
        [DllImport("onlinetrainer.net.native.dll")]
        private static extern IntPtr CreateVWOnlineTrainer();

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern void DeleteVWOnlineTrainer(IntPtr vwOnlineTrainer);

        private static New<VWOnlineTrainer> BindConstructorArguments()
        {
            return new New<VWOnlineTrainer>(() => CreateVWOnlineTrainer());
        }

        public VWOnlineTrainer() : base(BindConstructorArguments(), new Delete<VWOnlineTrainer>(DeleteVWOnlineTrainer))
        {
        }

        [DllImport("onlinetrainer.net.native.dll")]
        private static extern int Initialise(IntPtr liveModel, IntPtr apiStatus);
    }
}
