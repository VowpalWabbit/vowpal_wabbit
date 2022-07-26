using System;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern IntPtr CreateBuilder(IntPtr vw, IntPtr ex, byte feature_group);

    [DllImport("vw.net.native")]
    public static extern void DeleteBuilder(IntPtr builder);

    [DllImport("vw.net.native")]
    public static extern void BuilderPreallocate(IntPtr builder, int size);

    [DllImport("vw.net.native")]
    public static extern void BuilderAddFeature(IntPtr builder, ulong weight_index, float x);

    [DllImport("vw.net.native")]
    public static extern void BuilderAddFeaturesUnchecked(IntPtr builder, ulong weight_index_base, IntPtr begin, IntPtr end);

    [DllImport("vw.net.native")]
    public static extern UIntPtr BuilderGetFeatureCount(IntPtr builder);
  }
}

namespace VW
{
  using Vw.Net;
  using Vw.Net.Native;

  public sealed class VowpalWabbitNamespaceBuilder : NativeObject<VowpalWabbitNamespaceBuilder>
  {
    private VowpalWabbitExample example;
    private readonly byte featureGroup;

    private static New<VowpalWabbitNamespaceBuilder> BindConstructorArguments(VowpalWabbitExample example, byte namespaceIndex)
    {
      return new New<VowpalWabbitNamespaceBuilder>(() =>
      {
        IntPtr result = NativeMethods.CreateBuilder(example.Owner.DangerousGetNativeHandle(), example.DangerousGetNativeHandle(), namespaceIndex);
        example.Owner.KeepAlive();
        example.KeepAliveNative();

        return result;
      });
    }

    // TODO: This does not expose the new string-namespaces feature.
    internal VowpalWabbitNamespaceBuilder(VowpalWabbitExample example, byte namespaceIndex) : base(BindConstructorArguments(example, namespaceIndex), NativeMethods.DeleteBuilder)
    {
      this.example = example;
      this.featureGroup = namespaceIndex;
    }

    public void AddFeature(ulong weight_index, float x)
    {
      NativeMethods.BuilderAddFeature(this.DangerousGetHandle(), weight_index, x);
      GC.KeepAlive(this);
    }

    public void AddFeaturesUnchecked(ulong weight_index_base, IntPtr begin, IntPtr end)
    {
      NativeMethods.BuilderAddFeaturesUnchecked(this.DangerousGetHandle(), weight_index_base, begin, end);
      GC.KeepAlive(this);
    }

    unsafe public void AddFeaturesUnchecked(ulong weight_index_base, float* begin, float* end)
    {
      this.AddFeaturesUnchecked(weight_index_base, new IntPtr(begin), new IntPtr(end));
    }

    public void PreAllocate(int size)
    {
      NativeMethods.BuilderPreallocate(this.DangerousGetHandle(), size);
      GC.KeepAlive(this);
    }

    public ulong FeatureCount
    {
      get 
      {
        ulong result = NativeMethods.BuilderGetFeatureCount(this.DangerousGetHandle()).ToUInt64();
        GC.KeepAlive(this);

        return result;
      }
    }
  }
}