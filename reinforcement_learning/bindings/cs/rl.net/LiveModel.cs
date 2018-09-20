using System;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net {
    public sealed class LiveModel: NativeObject<LiveModel>
    {
        [DllImport("rl.net.native.dll")]
        private static extern IntPtr CreateLiveModel(IntPtr config);

        [DllImport("rl.net.native.dll")]
        private static extern void DeleteLiveModel(IntPtr liveModel);

        private static New<LiveModel> BindConstructorArguments(Configuration config)
        {
            return new New<LiveModel>(() => CreateLiveModel(config.NativeHandle));
        }

        [DllImport("rl.net.native.dll")]
        private static extern int InitLiveModel(IntPtr liveModel, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelChooseRank(IntPtr liveModel,  [MarshalAs(NativeMethods.StringMarshalling)] string eventId,  [MarshalAs(NativeMethods.StringMarshalling)] string contextJson, IntPtr rankingResponse, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelReportOutcome(IntPtr liveModel,  [MarshalAs(NativeMethods.StringMarshalling)] string eventId,  float outcome, IntPtr apiStatus);

        public LiveModel(Configuration config) : base(BindConstructorArguments(config), new Delete<LiveModel>(DeleteLiveModel))
        {
        }

        public bool TryInit(ApiStatus apiStatus = null)
        {
            int result = InitLiveModel(this.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        // TODO: This is a bit of an akward API, as it requires pre-construction of the RankingResponse.
        public bool TryChooseRank(string actionId, string contextJson, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelChooseRank(this.NativeHandle, actionId, contextJson, response.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryReportOutcome(string actionId, float outcome, ApiStatus apiStatus = null)
        {
            int result = LiveModelReportOutcome(this.NativeHandle, actionId, outcome, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }
    }
}