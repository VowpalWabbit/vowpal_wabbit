using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  // In the long run, the correct thing to do is probably model PolyPrediction and PolyLabel, along with accessor
  // methods on the actual complex objects inside, so that we do not need to create individual accessors for the
  // different simple (blittable) prediction types.
  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern float GetPredictionScalar(IntPtr example);

    [DllImport("vw.net.native")]
    public static extern VW.VowpalWabbitScalar GetPredictionScalarConfidence(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetPredictionScalarsCount(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern int GetPredictionScalars(IntPtr workspace, IntPtr example, IntPtr scalars_buffer, int scalars_buffer_size, bool limited = false);

    [DllImport("vw.net.native")]
    public static extern float GetPredictionProb(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern float GetPredictionCostSensitive(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern uint GetPredictionMulticlassClass(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetPredictionMultilabelCount(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern int GetPredictionMultilabel(IntPtr workspace, IntPtr example, IntPtr multilabel_buffer, int multilabel_buffer_size);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetPredictionActionScoresCount(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern int GetPredictionActionScores(IntPtr workspace, IntPtr example, IntPtr action_scores_buffer, int action_scores_buffer_size);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetPredictionTopicProbsCount(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern int GetPredictionTopicProbs(IntPtr workspace, IntPtr example, IntPtr topic_probs_buffer, int topic_probs_buffer_size);

    [DllImport("vw.net.native")]
    public static extern int GetPredictionActiveMulticlassClass(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetPredictionActiveMulticlassMoreInfoRequiredClassesCount(IntPtr workspace, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern int GetPredictionActiveMulticlassMoreInfoRequiredClasses(IntPtr workspace, IntPtr example, IntPtr more_info_required_classes_buffer, int more_info_required_classes_buffer_size);

    [DllImport("vw.net.native")]
    public static extern VW.VowpalWabbitActionPdfValue GetPredictionActionPdfValue(IntPtr workspace, IntPtr example);
  }
}

namespace VW
{
    using System.Linq;
    using Vw.Net.Native;
  using static PredictionHelpers;

  // This is cloned from prediction_type.h. If that enum is changed, it needs to be updated here. Unfortunately,
  // we made the decision in the C#/CLI implementation of the bindings to expose the native type, so in order to
  // be a drop-in replacement, we need to re-duplicate it into a managed type. On the bright side it makes a bit
  // of this easier to implement. In the longer run we can use SourceGenerators to generate these kinds of types.
  public enum prediction_type_t : uint
  {
    scalar,
    scalars,
    action_scores,
    pdf,
    action_probs,
    multiclass,
    multilabels,
    prob,
    multiclassprobs,  // not in use (technically oaa.cc)
    decision_probs, // TODO: Not currently exposed, and this is a fairly complex object.
    action_pdf_value,
    active_multiclass,
    nopred
  }

  public interface IVowpalWabbitPredictionFactory<T>
  {
    T Create(VowpalWabbit vw, VowpalWabbitExample example);

    prediction_type_t PredictionType { get; }
  }

  internal static class PredictionHelpers
  {
    public static void CheckArguments(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      // Note that we need for the argument names here (ex, vw) to match the C#/CLI implementation,
      // otherwise the exception will be inconsistent between the two. With that said, still follow
      // best practices and use nameof() for the argument names.
      if (vw == null)
        throw new ArgumentNullException(nameof(vw));
      if (ex == null)
        throw new ArgumentNullException(nameof(ex));
    }
    
    public static void CheckExample(this VowpalWabbit vw, VowpalWabbitExample ex, prediction_type_t desiredPredictionType)
    {
      CheckArguments(vw, ex);
      
      if (ex.PredictionType != desiredPredictionType)
        throw new ArgumentException($"Requested prediction type {desiredPredictionType}, but example contains {ex.PredictionType}.");
    }
  }

  public class VowpalWabbitDynamicPredictionFactory : IVowpalWabbitPredictionFactory<object>
  {
    // This is set up to do the same thing that the C#/CLI implementation does, though there is no principled reason why we cannot do a best-efforts
    // guess. On the flip side, there may be some ambiguity?
    prediction_type_t IVowpalWabbitPredictionFactory<object>.PredictionType => throw new NotSupportedException("Prediction type is not available.");
    
    object IVowpalWabbitPredictionFactory<object>.Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      CheckArguments(vw, ex);

      prediction_type_t outputPredictionType = vw.GetOutputPredictionType();
      switch (outputPredictionType)
      {
        case prediction_type_t.scalar:
          return VowpalWabbitPredictionType.Scalar.Create(vw, ex);
        case prediction_type_t.scalars:
          return VowpalWabbitPredictionType.Scalars.Create(vw, ex);
        case prediction_type_t.multiclass:
          return VowpalWabbitPredictionType.Multiclass.Create(vw, ex);
        case prediction_type_t.multilabels:
          return VowpalWabbitPredictionType.Multilabel.Create(vw, ex);
        case prediction_type_t.action_scores:
          return VowpalWabbitPredictionType.ActionScore.Create(vw, ex);
        case prediction_type_t.action_probs:
          return VowpalWabbitPredictionType.ActionProbabilities.Create(vw, ex);
        case prediction_type_t.prob:
          return VowpalWabbitPredictionType.Probability.Create(vw, ex);
        case prediction_type_t.multiclassprobs:
          return VowpalWabbitPredictionType.MultiClassProbabilities.Create(vw, ex);
        case prediction_type_t.active_multiclass:
          return VowpalWabbitPredictionType.ActiveMulticlass.Create(vw, ex);
        case prediction_type_t.action_pdf_value:
          return VowpalWabbitPredictionType.ActionPdfValue.Create(vw, ex);
        /*
         * There are other types we should consider implementing, but the goal is to hit 1:1 with previous
         * bindings first.
         */
        default:
          throw new NotSupportedException("Prediction type is not supported.");
      }
    }
  };
  
  public class VowpalWabbitScalarPredictionFactory : IVowpalWabbitPredictionFactory<float>
  {
    public prediction_type_t PredictionType => prediction_type_t.scalar;
    
    public float Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);
      
      float result = NativeMethods.GetPredictionScalar(ex.DangerousGetNativeHandle());
      GC.KeepAlive(vw);
      ex.KeepAliveNative();

      return result;
    }
  };
  
  // This name is chosen to match the C#/CLI implementation. A better name would be something
  // like ScalarConfidence.
  [StructLayout(LayoutKind.Sequential)]
  public struct VowpalWabbitScalar
  {
    public float Value;
    public float Confidence;
  }

  public class VowpalWabbitScalarConfidencePredictionFactory : IVowpalWabbitPredictionFactory<VowpalWabbitScalar>
  {
    public prediction_type_t PredictionType => prediction_type_t.scalar;
    
    public VowpalWabbitScalar Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);
      
      VowpalWabbitScalar result = NativeMethods.GetPredictionScalarConfidence(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      GC.KeepAlive(vw);
      ex.KeepAliveNative();
      
      return result;
    }
  };

  public class VowpalWabbitScalarsPredictionFactory : IVowpalWabbitPredictionFactory<float[]>
  {
    public prediction_type_t PredictionType => prediction_type_t.scalars;
    
    internal static float[] ReadScalarsUnchecked(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      UIntPtr count = NativeMethods.GetPredictionScalarsCount(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      if (count.ToUInt64() > Int32.MaxValue)
      {
        // This is inconsistent with what happens in the C#/CLI implementation, but that implementation
        // has a downcast from size_t to int (int32_t), and the behaviour when casting is implementation
        // specific. Thus, it is UB when the number of scalars is too large, and introducing an exception
        // should not be considered  a breaking change.
        // TODO: The exception type is not the best, but keep it for consistency with the others.
        throw new ArgumentOutOfRangeException("Number of scalars is too large.");
      }

      float[] result = new float[(int)count.ToUInt32()];
      unsafe
      {
        fixed (float* resultPtr = result)
        {
          int returned = NativeMethods.GetPredictionScalars(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle(), new IntPtr(resultPtr), result.Length);
          GC.KeepAlive(vw);
          ex.KeepAliveNative();
        
          Debug.Assert(returned >= 0, "The size returned by GetPredictionScalarsCount is insufficient to hold the scalars. This is a bug.");
          Debug.Assert(returned == result.Length, "Returned scalars count does not match requested count. This is a bug.");
        }
      }

      return result;
    }

    public float[] Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);

      return ReadScalarsUnchecked(vw, ex);
    }
  };
  
  public class VowpalWabbitProbabilityPredictionFactory : IVowpalWabbitPredictionFactory<float>
  {
    public prediction_type_t PredictionType => prediction_type_t.prob;

    public float Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);
      
      float result = NativeMethods.GetPredictionProb(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      GC.KeepAlive(vw);
      ex.KeepAliveNative();
      
      return result;
    }
  };
  
  public class VowpalWabbitCostSensitivePredictionFactory : IVowpalWabbitPredictionFactory<float>
  {
    public prediction_type_t PredictionType => prediction_type_t.multiclass;

    public float Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);
      
      float result = NativeMethods.GetPredictionCostSensitive(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      GC.KeepAlive(vw);
      ex.KeepAliveNative();
      
      return result;
    }
  };

  public class VowpalWabbitMulticlassPredictionFactory : IVowpalWabbitPredictionFactory<uint>
  {
    public prediction_type_t PredictionType => prediction_type_t.multiclass;
    
    public uint Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);
      
      uint result = NativeMethods.GetPredictionMulticlassClass(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      GC.KeepAlive(vw);
      ex.KeepAliveNative();
      
      return result;
    }
  };

  public class VowpalWabbitMulticlassProbabilitiesPredictionFactory : IVowpalWabbitPredictionFactory<Dictionary<int, float>>
  {
    public prediction_type_t PredictionType => prediction_type_t.multiclassprobs;
    
    public Dictionary<int, float> Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      // Althrough the prediction technically contains multiclassprobs, under the hood it is kept as 
      // "scalars". See VW::get_cost_sensitive_prediction_confidence_scores.
      PredictionHelpers.CheckExample(vw, ex, prediction_type_t.scalars);
      
      float[] confidenceScores = VowpalWabbitScalarsPredictionFactory.ReadScalarsUnchecked(vw, ex);

      int running_index = 0;

      Dictionary<int, float> result = confidenceScores.ToDictionary(score => ++running_index);
            
      return result;
    }
  };

  public class VowpalWabbitMultilabelPredictionFactory : IVowpalWabbitPredictionFactory<int[]>
  {
    public prediction_type_t PredictionType => prediction_type_t.multilabels;

    public int[] Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);

      UIntPtr count = NativeMethods.GetPredictionMultilabelCount(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      if (count.ToUInt64() > Int32.MaxValue)
      {
        throw new ArgumentOutOfRangeException("Multi-label predictions too large");
      }

      // TODO: The type here should really be uint (since it is it uint32_t on the other side), but 
      // the C#/CLI implementation is using int, and we need to match that.
      int[] result = new int[(int)count.ToUInt32()];
      unsafe
      {
        fixed (int* resultPtr = result)
        {
          int returned = NativeMethods.GetPredictionMultilabel(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle(), new IntPtr(resultPtr), result.Length);
          GC.KeepAlive(vw);
          ex.KeepAliveNative();
          
          Debug.Assert(returned >= 0, "The size returned by GetPredictionMultilabelCount is insufficient to hold the multilabel. This is a bug.");
          Debug.Assert(returned == result.Length, "Returned multilabel count does not match requested count. This is a bug.");
        }
      }
      
      return result;
    }
  };

  [StructLayout(LayoutKind.Sequential)]
  public struct ActionScore
  {
    public uint Action;
    public float Score;
  };

  public abstract class VowpalWabbitActionScoreBasePredictionFactory : IVowpalWabbitPredictionFactory<ActionScore[]>
  {
    public abstract prediction_type_t PredictionType { get; }

    private void ExtractActionScores()
    {
    }

    public ActionScore[] Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);
      
      UIntPtr count = NativeMethods.GetPredictionActionScoresCount(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      if (count.ToUInt64() > UInt32.MaxValue)
      {
        throw new ArgumentOutOfRangeException("The number of action scores is too large.");
      }

      // This works by relying on how P/Invoke deals with marshalling blittable types (which inclues structs marked
      // for Sequential layout). This allows us to pass the buffer and write into it. In the longer run, we may want
      // to have have the Native-bound objects work on Span<T> instead, and then we can avoid the copy in the first
      // place.
      // See https://docs.microsoft.com/en-us/dotnet/framework/interop/default-marshalling-behavior#default-marshalling-for-value-types
      ActionScore[] result = new ActionScore[(int)count.ToUInt32()];

      unsafe
      {
        fixed (ActionScore* resultPtr = result)
        {
          int returned = NativeMethods.GetPredictionActionScores(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle(), new IntPtr(resultPtr), result.Length);
          GC.KeepAlive(vw);
          ex.KeepAliveNative();
          
          Debug.Assert(returned >= 0, "The size returned by GetPredictionActionScoresCount is insufficient to hold the action scores. This is a bug.");
          Debug.Assert(returned == result.Length, "Returned action scores count does not match requested count. This is a bug.");
        }
      }
      
      return result;
    }
  };

  public class VowpalWabbitActionScorePredictionFactory : VowpalWabbitActionScoreBasePredictionFactory
  {
    public override prediction_type_t PredictionType => prediction_type_t.action_scores;
  }

  public class VowpalWabbitActionProbabilitiesPredictionFactory : VowpalWabbitActionScoreBasePredictionFactory
  {
    public override prediction_type_t PredictionType => prediction_type_t.action_probs;
  };

  public class VowpalWabbitTopicPredictionFactory : IVowpalWabbitPredictionFactory<float[]>
  {
    public prediction_type_t PredictionType => prediction_type_t.scalars;

    public float[] Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);

      UIntPtr count = NativeMethods.GetPredictionTopicProbsCount(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());

      if (count.ToUInt64() > UInt32.MaxValue)
      {
        // This is inconsistent with the C#/CLI implementation, but similar to the raw scalars accessor,
        // C#/CLI does an implicit downcast from uint32_t to int, and similarly, if there is not enough 
        // space, the result is undefined behaviour, so throwing an exception is the best option. 
        throw new ArgumentOutOfRangeException("The number of topics is too large.");
      }

      float[] result = new float[(int)count.ToUInt32()];
      unsafe
      {
        fixed (float* resultPtr = result)
        {
          int returned = NativeMethods.GetPredictionTopicProbs(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle(), new IntPtr(resultPtr), result.Length);
          GC.KeepAlive(vw);
          ex.KeepAliveNative();
          
          Debug.Assert(returned >= 0, "The size returned by GetPredictionTopicCount is insufficient to hold the topics. This is a bug.");
          Debug.Assert(returned == result.Length, "Returned topic count does not match requested count. This is a bug.");
        }
      }

      return result;
    }
  };

  public struct VowpalWabbitActiveMulticlass
  {
    public int predicted_class;
    public int[] more_info_required_for_classes;
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct VowpalWabbitActionPdfValue
  {
    public float Action;
    public float PdfValue;
  }

  public class VowpalWabbitActiveMulticlassPredictionFactory : IVowpalWabbitPredictionFactory<VowpalWabbitActiveMulticlass>
  {
    public prediction_type_t PredictionType => prediction_type_t.multiclass;

    private unsafe int[] ReadMoreInfoRequiredClasses(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      UIntPtr count = NativeMethods.GetPredictionActiveMulticlassMoreInfoRequiredClassesCount(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());

      if (count.ToUInt64() > Int32.MaxValue)
      {
        // This is inconsistent with the C#/CLI implementation, but similar to the raw scalars accessor,
        // C#/CLI does an implicit downcast from size_t to int, and similarly, if there is not enough 
        // space, the result is undefined behaviour, so throwing an exception is the best option. 
        throw new ArgumentOutOfRangeException("The number of classes requiring more info is too large.");
      }

      int[] result = new int[(int)count.ToUInt32()];
      fixed (int* resultPtr = result)
      {
        int returned = NativeMethods.GetPredictionActiveMulticlassMoreInfoRequiredClasses(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle(), new IntPtr(resultPtr), result.Length);

        // Strictly speaking these are not necessary, but it is better to avoid having remarks about
        // consumers of ReadMoreInfoRequiredClasses being responsible for dealing with the lifecycle
        // of the input arguments.
        GC.KeepAlive(vw);
        ex.KeepAliveNative();
        
        Debug.Assert(returned >= 0, "The size returned by GetPredictionActiveMulticlassMoreInfoRequiredClassesCount is insufficient to hold the classes. This is a bug.");
        Debug.Assert(returned == result.Length, "Returned class count does not match requested count. This is a bug.");
      }

      return result;
    }

    public VowpalWabbitActiveMulticlass Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);

      VowpalWabbitActiveMulticlass result = new VowpalWabbitActiveMulticlass
      {
        predicted_class = NativeMethods.GetPredictionActiveMulticlassClass(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle()),
        more_info_required_for_classes = this.ReadMoreInfoRequiredClasses(vw, ex)
      };

      GC.KeepAlive(vw);
      ex.KeepAliveNative();

      return result;
    }
  }

  public class VowpalWabbitActionPdfValuePredictionFactory : IVowpalWabbitPredictionFactory<VowpalWabbitActionPdfValue>
  {
    public prediction_type_t PredictionType => prediction_type_t.action_pdf_value;

    public VowpalWabbitActionPdfValue Create(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      PredictionHelpers.CheckExample(vw, ex, this.PredictionType);

      VowpalWabbitActionPdfValue result = NativeMethods.GetPredictionActionPdfValue(vw.DangerousGetHandle(), ex.DangerousGetNativeHandle());
      GC.KeepAlive(vw);
      ex.KeepAliveNative();

      return result;
    }
  }

  public static class VowpalWabbitPredictionType
  {
    public static VowpalWabbitScalarPredictionFactory Scalar = new VowpalWabbitScalarPredictionFactory();

    public static VowpalWabbitScalarConfidencePredictionFactory ScalarConfidence = new VowpalWabbitScalarConfidencePredictionFactory();

    public static VowpalWabbitScalarsPredictionFactory Scalars = new VowpalWabbitScalarsPredictionFactory();

    public static VowpalWabbitCostSensitivePredictionFactory CostSensitive = new VowpalWabbitCostSensitivePredictionFactory();

    public static VowpalWabbitMultilabelPredictionFactory Multilabel = new VowpalWabbitMultilabelPredictionFactory();

    public static VowpalWabbitMulticlassPredictionFactory Multiclass = new VowpalWabbitMulticlassPredictionFactory();

    public static VowpalWabbitActionScorePredictionFactory ActionScore = new VowpalWabbitActionScorePredictionFactory();

    public static VowpalWabbitActionProbabilitiesPredictionFactory ActionProbabilities = new VowpalWabbitActionProbabilitiesPredictionFactory();

    public static VowpalWabbitTopicPredictionFactory Topic = new VowpalWabbitTopicPredictionFactory();

    public static VowpalWabbitDynamicPredictionFactory Dynamic = new VowpalWabbitDynamicPredictionFactory();

    public static VowpalWabbitProbabilityPredictionFactory Probability = new VowpalWabbitProbabilityPredictionFactory();

    public static VowpalWabbitMulticlassProbabilitiesPredictionFactory MultiClassProbabilities = new VowpalWabbitMulticlassProbabilitiesPredictionFactory();

    public static VowpalWabbitActiveMulticlassPredictionFactory ActiveMulticlass = new VowpalWabbitActiveMulticlassPredictionFactory();

    public static VowpalWabbitActionPdfValuePredictionFactory ActionPdfValue = new VowpalWabbitActionPdfValuePredictionFactory();

  }
}

