using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace Microsoft.Research.MachineLearning
{
    public sealed class VowpalWabbitInterface
    {
        public class FeatureSpace
        {
            public string Name;
            public Feature[] Features;
        }

        public class Feature
        {
            public float X;
            public UInt32 WeightIndex;
        }

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

        [DllImport("libvw.dll", EntryPoint="VW_Initialize", CallingConvention=CallingConvention.StdCall)]
        public static extern IntPtr Initialize(string arguments);

        [DllImport("libvw.dll", EntryPoint="VW_Finish", CallingConvention=CallingConvention.StdCall)]
        public static extern void Finish(IntPtr vw);

        [DllImport("libvw.dll", EntryPoint = "VW_ImportExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr ImportExample(IntPtr vw,
            [In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)]
            FEATURE_SPACE[] features,
            int length);

        [DllImport("libvw.dll", EntryPoint="VW_ReadExample", CallingConvention=CallingConvention.StdCall)]
        public static extern IntPtr ReadExample(IntPtr vw, string exampleString);

        [DllImport("libvw.dll", EntryPoint="VW_FinishExample", CallingConvention=CallingConvention.StdCall)]
        public static extern void FinishExample(IntPtr vw, IntPtr example);

        [DllImport("libvw.dll", EntryPoint = "VW_HashSpace", CallingConvention = CallingConvention.StdCall)]
        public static extern uint HashSpace(IntPtr vw, string s);

        [DllImport("libvw.dll", EntryPoint = "VW_HashFeature", CallingConvention = CallingConvention.StdCall)]
        public static extern uint HashFeature(IntPtr vw, string s, ulong u);

        [DllImport("libvw.dll", EntryPoint="VW_Learn", CallingConvention=CallingConvention.StdCall)]
        public static extern float Learn(IntPtr vw, IntPtr example);
    }
}