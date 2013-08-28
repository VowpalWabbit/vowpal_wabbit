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
            public IntPtr features;     // points to a FEATURE[]
            public int len;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct FEATURE
        {
            public float x;
            public uint weight_index;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct LABEL
        {
            public float label;
            public float weight;
            public float initial;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct FLAT_RAW_EXAMPLE
        {
            public IntPtr ld;
            public float final_prediction;  

            //byte[] tag;//An identifier for the example.
            public int tag_len;
            public IntPtr tag;//An identifier for the example. 

            public UInt64 example_counter;

            //FEATURE[] feature_map; //map to store sparse feature vectors  
            public int feature_map_len;
            public IntPtr feature_map; //map to store sparse feature vectors  

            public UInt32 ft_offset;

            public UInt64 num_features;//precomputed, cause it's fast&easy.  
            public float partial_prediction;//shared data for prediction.  

            //float[] topic_predictions;
            public int topic_predictions_len;
            public IntPtr topic_predictions;

            public float loss;
            public float eta_round;
            public float eta_global;
            public float global_weight;
            public float example_t;//sum of importance weights so far.  

            //float[] sum_feat_sq;//helper for total_sum_feat_sq.  
            public int sum_feat_sq_len;
            public IntPtr sum_feat_sq;//helper for total_sum_feat_sq.

            public float total_sum_feat_sq;//precomputed, cause it's kind of fast & easy.  
            public float revert_weight;

            public bool end_pass;//special example indicating end of pass.  
            public bool sorted;//Are the features sorted or not?  
            public bool in_use; //in use or not (for the parser)  
            public bool done; //set to false by setup_example()  
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct FLAT_EXAMPLE
        {
            public LABEL ld;
            public float final_prediction;

            public byte[] tag;//An identifier for the example. 

            public UInt64 example_counter;

            public FEATURE[] feature_map; //map to store sparse feature vectors  

            public UInt32 ft_offset;

            public UInt64 num_features;//precomputed, cause it's fast&easy.  
            public float partial_prediction;//shared data for prediction.  

            public float[] topic_predictions;

            public float loss;
            public float eta_round;
            public float eta_global;
            public float global_weight;
            public float example_t;//sum of importance weights so far.  

            public float[] sum_feat_sq;//helper for total_sum_feat_sq.  

            public float total_sum_feat_sq;//precomputed, cause it's kind of fast & easy.  
            public float revert_weight;

            public bool end_pass;//special example indicating end of pass.  
            public bool sorted;//Are the features sorted or not?  
            public bool in_use; //in use or not (for the parser)  
            public bool done; //set to false by setup_example()  
        }

        [DllImport("libvw.dll", EntryPoint = "VW_Initialize", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr Initialize([MarshalAs(UnmanagedType.LPWStr)]string arguments);

        [DllImport("libvw.dll", EntryPoint = "VW_Finish", CallingConvention = CallingConvention.StdCall)]
        public static extern void Finish(IntPtr vw);
        
        [DllImport("libvw.dll", EntryPoint = "VW_ImportExample", CallingConvention = CallingConvention.StdCall)]
        // features points to a FEATURE_SPACE[]
        public static extern IntPtr ImportExample(IntPtr vw, IntPtr features, int length);

        [DllImport("libvw.dll", EntryPoint = "VW_ExportExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr ExportExample(IntPtr vw, IntPtr example, ref int length);

        [DllImport("libvw.dll", EntryPoint = "VW_ReleaseFeatureSpace", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr ReleaseFeatureSpace(IntPtr fs, int length);
       
        [DllImport("libvw.dll", EntryPoint = "VW_ReadExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr ReadExample(IntPtr vw, [MarshalAs(UnmanagedType.LPWStr)]string exampleString);

        [DllImport("libvw.dll", EntryPoint = "VW_StartParser", CallingConvention = CallingConvention.StdCall)]
        public static extern void StartParser(IntPtr vw, bool do_init);

        [DllImport("libvw.dll", EntryPoint = "VW_EndParser", CallingConvention = CallingConvention.StdCall)]
        public static extern void EndParser(IntPtr vw);

        [DllImport("libvw.dll", EntryPoint = "VW_GetExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr GetExample(IntPtr parser);

        [DllImport("libvw.dll", EntryPoint = "VW_GetLabel", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr GetLabel(IntPtr vw, IntPtr example);

        [DllImport("libvw.dll", EntryPoint = "VW_FinishExample", CallingConvention = CallingConvention.StdCall)]
        public static extern void FinishExample(IntPtr vw, IntPtr example);
        
        [DllImport("libvw.dll", EntryPoint = "VW_HashSpace", CallingConvention = CallingConvention.StdCall)]
        public static extern uint HashSpace(IntPtr vw, [MarshalAs(UnmanagedType.LPWStr)]string s);
        
        [DllImport("libvw.dll", EntryPoint = "VW_HashFeature", CallingConvention = CallingConvention.StdCall)]
        public static extern uint HashFeature(IntPtr vw, [MarshalAs(UnmanagedType.LPWStr)]string s, ulong u);
        
        [DllImport("libvw.dll", EntryPoint = "VW_Learn", CallingConvention = CallingConvention.StdCall)]
        public static extern float Learn(IntPtr vw, IntPtr example);

        [DllImport("libvw.dll", EntryPoint = "VW_AddLabel", CallingConvention = CallingConvention.StdCall)]
        public static extern void AddLabel(IntPtr example, float label=float.MaxValue, float weight=1, float initial=0);

        [DllImport("libvw.dll", EntryPoint = "VW_Get_Weight", CallingConvention = CallingConvention.StdCall)]
        public static extern void Get_Weight(IntPtr vw, UInt32 index);

        [DllImport("libvw.dll", EntryPoint = "VW_Num_Weights", CallingConvention = CallingConvention.StdCall)]
        public static extern UInt32 Num_Weights(IntPtr vw);

        [DllImport("libvw.dll", EntryPoint = "VW_Get_Stride", CallingConvention = CallingConvention.StdCall)]
        public static extern UInt32 Get_Stride(IntPtr vw);

        [DllImport("libvw.dll", EntryPoint = "VW_FlattenExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr Flatten_Example(IntPtr vw, IntPtr example);

        [DllImport("libvw.dll", EntryPoint = "VW_FreeFlattenExample", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr FreeFlattenExample(IntPtr fec);
    
    }
}
