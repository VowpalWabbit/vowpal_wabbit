using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Microsoft.Research.MachineLearning
{

    public sealed class VowpalWabbitInterface
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct FEATURE_SPACE
        {
            public byte name;
            public IntPtr features;
            public int len;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct FEATURE
        {
            public float x;
            public uint weight_index;
        }

	[StructLayout(LayoutKind.Sequential)]
	public struct StartOfExample{
	       public IntPtr labeldata;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct Label_Data {
	       public float label;
	       public float weight;
	       public float initial;
	}

	public static Label_Data DefaultLabelData() {
	       Label_Data ld;
	       ld.label = float.MaxValue;
	       ld.weight = 1;
	       ld.initial = 0;
	       return ld;
	}

        [DllImport("libvw.dll", EntryPoint="VW_Initialize", CallingConvention=CallingConvention.StdCall)]
        public static extern IntPtr Initialize([MarshalAs(UnmanagedType.LPWStr)]string arguments);

        [DllImport("libvw.dll", EntryPoint="VW_Finish", CallingConvention=CallingConvention.StdCall)]
        public static extern void Finish(IntPtr vw);

        [DllImport("libvw.dll", EntryPoint = "VW_ImportExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr ImportExample(IntPtr vw, IntPtr features, int length);

        [DllImport("libvw.dll", EntryPoint="VW_ReadExample", CallingConvention=CallingConvention.StdCall)]
        public static extern IntPtr ReadExample(IntPtr vw, [MarshalAs(UnmanagedType.LPWStr)]string exampleString);

        [DllImport("libvw.dll", EntryPoint="VW_FinishExample", CallingConvention=CallingConvention.StdCall)]
        public static extern void FinishExample(IntPtr vw, IntPtr example);

        [DllImport("libvw.dll", EntryPoint = "VW_HashSpace", CallingConvention = CallingConvention.StdCall)]
        public static extern uint HashSpace(IntPtr vw, [MarshalAs(UnmanagedType.LPWStr)]string s);

        [DllImport("libvw.dll", EntryPoint = "VW_HashFeature", CallingConvention = CallingConvention.StdCall)]
        public static extern uint HashFeature(IntPtr vw, [MarshalAs(UnmanagedType.LPWStr)]string s, ulong u);

        [DllImport("libvw.dll", EntryPoint="VW_Learn", CallingConvention=CallingConvention.StdCall)]
        public static extern float Learn(IntPtr vw, IntPtr example);
    }
}