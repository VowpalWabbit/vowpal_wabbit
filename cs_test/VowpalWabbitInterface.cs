using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Microsoft.Research.MachineLearning
{
    public sealed class VowpalWabbitInterface
    {
        [DllImport("libvw.dll", EntryPoint="VW_Initialize", CallingConvention=CallingConvention.StdCall)]
        public static extern IntPtr Initialize(string arguments);

        [DllImport("libvw.dll", EntryPoint="VW_Finish", CallingConvention=CallingConvention.StdCall)]
        public static extern void Finish(IntPtr vw);

        [DllImport("libvw.dll", EntryPoint="VW_ReadExample", CallingConvention=CallingConvention.StdCall)]
        public static extern IntPtr ReadExample(IntPtr vw, string exampleString);

        [DllImport("libvw.dll", EntryPoint="VW_FinishExample", CallingConvention=CallingConvention.StdCall)]
        public static extern void FinishExample(IntPtr vw, IntPtr example);

        [DllImport("libvw.dll", EntryPoint="VW_Learn", CallingConvention=CallingConvention.StdCall)]
        public static extern float Learn(IntPtr vw, IntPtr example);
    }
}