using System;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;
using Newtonsoft.Json;

namespace Rl.Net {
    public sealed class LiveModel : NativeObject<LiveModel>, ILiveModel
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
        private static extern int LiveModelInit(IntPtr liveModel, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelChooseRank(IntPtr liveModel,  [MarshalAs(NativeMethods.StringMarshalling)] string eventId,  [MarshalAs(NativeMethods.StringMarshalling)] string contextJson, IntPtr rankingResponse, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelReportOutcome(IntPtr liveModel,  [MarshalAs(NativeMethods.StringMarshalling)] string eventId,  float outcome, IntPtr apiStatus);

        [DllImport("rl.net.native.dll")]
        private static extern int LiveModelReportOutcome(IntPtr liveModel, [MarshalAs(NativeMethods.StringMarshalling)] string eventId, string outcome, IntPtr apiStatus);

        public LiveModel(Configuration config) : base(BindConstructorArguments(config), new Delete<LiveModel>(DeleteLiveModel))
        {
        }

        public bool TryInit(ApiStatus apiStatus = null)
        {
            int result = LiveModelInit(this.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryChooseRank(string actionId, string contextJson, out RankingResponse response, ApiStatus apiStatus = null)
        {
            response = new RankingResponse();
            return this.TryChooseRank(actionId, contextJson, response, apiStatus);
        }

        public bool TryChooseRank(string actionId, string contextJson, RankingResponse response, ApiStatus apiStatus = null)
        {
            int result = LiveModelChooseRank(this.NativeHandle, actionId, contextJson, response.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }

        public bool TryReportOutcome(string actionId, object outcome, ApiStatus apiStatus = null)
        {
            string objStr = JsonConvert.SerializeObject(outcome);
            int result;
            if(float.TryParse(objStr, out float temp))
            {
                result = LiveModelReportOutcome(this.NativeHandle, actionId, temp, apiStatus.ToNativeHandleOrNullptr());
            }
            else
            {
                result = LiveModelReportOutcome(this.NativeHandle, actionId, objStr, apiStatus.ToNativeHandleOrNullptr());
            }
            return result == NativeMethods.SuccessStatus;
        }

        public new void Dispose()
        {
            base.Dispose();
        }
    }
}