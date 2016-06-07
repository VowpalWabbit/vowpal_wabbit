using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace VW
{
    using SizeT = IntPtr;
    using VwHandle = IntPtr;
    using VwFeatureSpace = IntPtr;
    using VwExample = IntPtr;
    using VwFeature = IntPtr;
    using BytePtr = IntPtr;

    public sealed class VowpalWabbitInterface
    {
        private const string LIBVW = "libvw.dll";

        [StructLayout(LayoutKind.Sequential)]
        public struct FEATURE_SPACE
        {
            public byte name;
            public IntPtr features;     // points to a FEATURE[]
            public int len;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct FEATURE
        {
            public float x;
            public uint weight_index;
        }

        [DllImport(LIBVW, EntryPoint = "VW_Initialize")]
        public static extern VwHandle Initialize([MarshalAs(UnmanagedType.LPWStr)]string arguments);

        [DllImport(LIBVW, EntryPoint = "VW_Finish")]
        public static extern void Finish(VwHandle vw);

        [DllImport(LIBVW, EntryPoint = "VW_ImportExample")]
        // features points to a FEATURE_SPACE[]
        public static extern VwExample ImportExample(VwHandle vw, VwFeatureSpace features, SizeT length);

        [DllImport(LIBVW, EntryPoint = "VW_ExportExample")]
        public static extern VwFeatureSpace ExportExample(VwHandle vw, VwExample example, ref SizeT length);

        [DllImport(LIBVW, EntryPoint = "VW_ReleaseFeatureSpace")]
        public static extern void ReleaseFeatureSpace(VwFeatureSpace fs, SizeT length);

        [DllImport(LIBVW, EntryPoint = "VW_ReadExample")]
        public static extern VwExample ReadExample(VwHandle vw, [MarshalAs(UnmanagedType.LPWStr)]string exampleString);

        // Have to marshal bools, C++ considers them 4 byte quantities, and C# considers them 1 byte.
        [DllImport(LIBVW, EntryPoint = "VW_StartParser")]
        public static extern void StartParser(VwHandle vw, [MarshalAs(UnmanagedType.Bool)]bool do_init);

        [DllImport(LIBVW, EntryPoint = "VW_EndParser")]
        public static extern void EndParser(VwHandle vw);

        [DllImport(LIBVW, EntryPoint = "VW_GetExample")]
        public static extern VwExample GetExample(VwHandle parser);

        [DllImport(LIBVW, EntryPoint = "VW_FinishExample")]
        public static extern void FinishExample(VwHandle vw, VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_GetTopicPrediction")]
        public static extern float GetTopicPrediction(VwExample example, SizeT i);
        
        [DllImport(LIBVW, EntryPoint = "VW_GetLabel")]
        public static extern float GetLabel(VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_GetImportance")]
        public static extern float GetImportance(VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_GetInitial")]
        public static extern float GetInitial(VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_GetMultilabelPredictions")]
        public static extern IntPtr GetMultilabelPredictions(VwHandle vw, VwExample example, ref SizeT length);

        [DllImport(LIBVW, EntryPoint = "VW_GetPrediction")]
        public static extern float GetPrediction(VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_GetTagLength")]
        public static extern SizeT GetTagLength(VwExample example);

        // Saying this returned a byte was inappropriate, because you were returning
        // actually a pointer to a seqeunce of bytes.  (Not sure what the interpretation
        // of this should be, utf8 or something?)
        [DllImport(LIBVW, EntryPoint = "VW_GetTag")]
        public static extern BytePtr GetTag(VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_GetFeatureNumber")]
        public static extern SizeT GetFeatureNumber(VwExample example);

        // Same note regarding ref int vs size_t*
        [DllImport(LIBVW, EntryPoint = "VW_GetFeatures")]
        public static extern VwFeature GetFeatures(VwHandle vw, VwExample example, ref SizeT length);

        [DllImport(LIBVW, EntryPoint = "VW_ReturnFeatures")]
        public static extern void ReturnFeatures(VwExample features);

        [DllImport(LIBVW, EntryPoint = "VW_HashSpace")]
        public static extern uint HashSpace(VwHandle vw, [MarshalAs(UnmanagedType.LPWStr)]string s);

        [DllImport(LIBVW, EntryPoint = "VW_HashSpaceStatic")]
        public static extern uint HashSpace([MarshalAs(UnmanagedType.LPWStr)]string s, [MarshalAs(UnmanagedType.LPWStr)]string h = "strings");

        // The DLL defines the last argument "u" as being an "unsigned long".
        // In C++ under current circumstances, both ints and longs are four byte integers.
        // If you wanted an eight byte integer you should use "long long" (or probably
        // more appropriately in this circumstance size_t).
        // In C#, "int" is four bytes, "long" is eight bytes.
        [DllImport(LIBVW, EntryPoint = "VW_HashFeature")]
        public static extern uint HashFeature(VwHandle vw, [MarshalAs(UnmanagedType.LPWStr)]string s, uint u);

        [DllImport(LIBVW, EntryPoint = "VW_HashFeatureStatic")]
        public static extern uint HashFeature([MarshalAs(UnmanagedType.LPWStr)]string s, uint u, [MarshalAs(UnmanagedType.LPWStr)]string h = "strings", uint numBits = 18);

        [DllImport(LIBVW, EntryPoint = "VW_Learn")]
        public static extern float Learn(VwHandle vw, VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_Predict")]
        public static extern float Predict(VwHandle vw, VwExample example);

        [DllImport(LIBVW, EntryPoint = "VW_AddLabel")]
        public static extern void AddLabel(VwExample example, float label = float.MaxValue, float weight = 1, float initial = 0);

        [DllImport(LIBVW, EntryPoint = "VW_Get_Weight")]
        public static extern float Get_Weight(VwHandle vw, SizeT index, SizeT offset);

        [DllImport(LIBVW, EntryPoint = "VW_Set_Weight")]
        public static extern void Set_Weight(VwHandle vw, SizeT index, SizeT offset, float value);

        [DllImport(LIBVW, EntryPoint = "VW_Num_Weights")]
        public static extern SizeT Num_Weights(VwHandle vw);

        [DllImport(LIBVW, EntryPoint = "VW_Get_Stride")]
        public static extern SizeT Get_Stride(VwHandle vw);

        [DllImport(LIBVW, EntryPoint = "VW_SaveModel")]
        public static extern void SaveModel(VwHandle vw);
    }
}
