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

        [DllImport("rl.net.native.dll")] // We need to decide on accepted string encodings here. By default, I'm declaring this a unicode string.
        private static extern int LoadConfigurationFromJson(int jsonLength, [MarshalAs(UnmanagedType.LPWStr)] string json, IntPtr config);

        // TODO: Once we expose direct manipulation methods on configuration, this can go public
        public Configuration() : base(new New<Configuration>(CreateConfig), new Delete<Configuration>(DeleteConfig))
        {
        }

        public static bool TryLoadConfigurationFromJson(string json, out Configuration config)
        {
            config = new Configuration();

            int result = LoadConfigurationFromJson(json.Length, json, config.handle);
            return true;
        }
    }
}