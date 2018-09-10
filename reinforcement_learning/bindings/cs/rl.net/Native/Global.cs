using System;
using System.Threading;
using System.Runtime.InteropServices;

namespace Rl.Net.Native {
    internal partial class NativeMethods
    {
        [DllImport("rl.net.native.dll")]
        public static extern void SayHello();
    }
}