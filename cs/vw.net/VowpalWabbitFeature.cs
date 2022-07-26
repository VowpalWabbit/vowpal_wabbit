using System;
using System.Runtime;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  using feature_index = System.UInt64;

  // defined in feature_group.h
  [StructLayout(LayoutKind.Sequential)]
  internal struct NativeFeature
  {
    public float x;
    public feature_index weight_index;
  }

  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern feature_index GetShiftedWeightIndex(IntPtr vw, IntPtr ex, ulong weight_index_unshifted);

    [DllImport("vw.net.native")]
    public static extern float GetWeight(IntPtr vw, IntPtr ex, ulong weight_index_unshifted);

    [DllImport("vw.net.native")]
    public static extern float GetAuditWeight(IntPtr vw, IntPtr ex, ulong weight_index_unshifted);
  }
}

namespace VW
{
  using Vw.Net.Native;
  using feature_value = Single; // float = System.Single

  [System.Diagnostics.DebuggerDisplay("{WeightIndex}:{X}")]
  public struct VowpalWabbitFeature
  {
    private VowpalWabbit vw;
    private VowpalWabbitExample example;
    private ulong weightIndexUnshifted;

    public VowpalWabbitFeature(VowpalWabbitExample example, feature_value x, ulong weight_index)
    {
      this.vw = example.Owner.Native;
      this.example = example;

      this.X = x;
      this.weightIndexUnshifted = weight_index;
    }

    internal VowpalWabbitFeature(VowpalWabbitExample example, NativeFeature feature)
      : this(example, feature.x, feature.weight_index)
    {
    }

    public VowpalWabbitFeature(VowpalWabbit vw, feature_value x, ulong weight_index)
    {
      this.vw = vw;
      this.example = null;

      this.X = x;
      this.weightIndexUnshifted = weight_index;
    }

    internal VowpalWabbitFeature(VowpalWabbit vw, NativeFeature feature_data)
      : this(vw, feature_data.x, feature_data.weight_index)
    {
    }

    public feature_value X { get; private set; }

    public ulong FeatureIndex => this.WeightIndex;

    public ulong WeightIndex
    {
      get
      {
        ulong result = NativeMethods.GetShiftedWeightIndex(this.vw.DangerousGetHandle(), this.example.DangerousGetNativeHandle(), this.weightIndexUnshifted);
        GC.KeepAlive(this.vw);
        this.example.KeepAliveNative();

        return result;
      }
    }

    public float Weight
    {
      get
      {
        float result = NativeMethods.GetWeight(this.vw.DangerousGetHandle(), this.example.DangerousGetNativeHandle(), this.weightIndexUnshifted);
        GC.KeepAlive(this.vw);
        this.example.KeepAliveNative();

        return result;
      }
    }

    public float AuditWeight
    {
      get
      {
        float result = NativeMethods.GetAuditWeight(this.vw.DangerousGetHandle(), this.example.DangerousGetNativeHandle(), this.weightIndexUnshifted);
        GC.KeepAlive(this.vw);
        this.example.KeepAliveNative();

        return result;
      }
    }

    public override bool Equals(object obj)
    {
      VowpalWabbitFeature? other = obj as VowpalWabbitFeature?;
      if (other == null)
      {
        return false;
      }

      // TODO: Should this be checking that we are operating in the same
      // VW workspace?
      return this.X == other.Value.X && this.WeightIndex == other.Value.WeightIndex;
    }
    
    public override int GetHashCode()
    {
      // ORIGINAL IMPLEMENTATION
      //return this.X.GetHashCode() + this.WeightIndex.GetHashCode();

      // Note very specifically that we are not keeping consistent with the pre-DotNet Core 
      // implementation of VowpalWabbitFeature.GetHashCode(). Avoiding using operator+ here 
      // should result in a better balanced hash. This is not a breaking change because hash
      // values are not defined to be consistent across versions.
      return this.X.GetHashCode() ^ this.WeightIndex.GetHashCode();
    }
  }
}