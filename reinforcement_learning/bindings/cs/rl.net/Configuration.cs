using System;
using System.Threading;
using System.Runtime.InteropServices;

using Rl.Net.Native;

namespace Rl.Net {
    public sealed class Configuration: NativeObject<Configuration>
    {
        [DllImport("rl.net.native.dll")]
        private static extern IntPtr CreateConfig();

        [DllImport("rl.net.native.dll")]
        private static extern void DeleteConfig(IntPtr config);

        [DllImport("rl.net.native.dll")]
        private static extern int LoadConfigurationFromJson(int jsonLength, [MarshalAs(NativeMethods.StringMarshalling)] string json, IntPtr config, IntPtr apiStatus);

        public Configuration() : base(new New<Configuration>(CreateConfig), new Delete<Configuration>(DeleteConfig))
        {
        }

        public static bool TryLoadConfigurationFromJson(string json, out Configuration config, ApiStatus apiStatus = null)
        {
            config = new Configuration();

            int result = LoadConfigurationFromJson(json.Length, json, config.NativeHandle, apiStatus.ToNativeHandleOrNullptr());
            return result == NativeMethods.SuccessStatus;
        }
    }
}