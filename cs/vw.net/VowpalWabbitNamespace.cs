using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


namespace Vw.Net.Native
{
  using namespace_index = Byte;  // unsigned char

  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern IntPtr CreateFeatureEnumerator(IntPtr vw, IntPtr ex, namespace_index ns);

    [DllImport("vw.net.native")]
    public static extern void DeleteFeatureEnumerator(IntPtr enumerator);

    [DllImport("vw.net.native")]
    public static extern bool FeatureEnumeratorMoveNext(IntPtr enumerator);

    [DllImport("vw.net.native")]
    public static extern void FeatureEnumeratorReset(IntPtr enumerator);

    [DllImport("vw.net.native")]
    public static extern void FeatureEnumeratorGetFeature(IntPtr enumerator, out NativeFeature feature);
  }
}

namespace VW
{
  using Vw.Net.Native;
  using namespace_index = Byte;  // unsigned char

  [System.Diagnostics.DebuggerDisplay("{Index} = '{(char)Index}'")]
  public struct VowpalWabbitNamespace : IEnumerable<VowpalWabbitFeature>
  {
    public VowpalWabbitNamespace(VowpalWabbitExample example, namespace_index index)
    {
      this.Example = example;
      this.Index = index;
    }

    internal class FeatureEnumerator : NativeObject<FeatureEnumerator>, IEnumerator<VowpalWabbitFeature>
    {
      private VowpalWabbitExample example;
      private readonly namespace_index ns;

      internal static New<FeatureEnumerator> BindConstructorArguments(VowpalWabbitExample example, namespace_index ns)
      {
        return new New<FeatureEnumerator>(() =>
        {
          IntPtr result = NativeMethods.CreateFeatureEnumerator(example.Owner.DangerousGetNativeHandle(), example.DangerousGetNativeHandle(), ns);

          example.Owner.KeepAlive();
          example.KeepAliveNative();

          return result;
        });
      }

      internal FeatureEnumerator(VowpalWabbitExample example, namespace_index ns)
        : base(BindConstructorArguments(example, ns), NativeMethods.DeleteFeatureEnumerator)
      {
        this.example = example;
        this.ns = ns;
      }

      public VowpalWabbitFeature Current
      {
        get
        {
          NativeMethods.FeatureEnumeratorGetFeature(this.DangerousGetHandle(), out NativeFeature feature);
          GC.KeepAlive(this);

          return new VowpalWabbitFeature(this.example, feature);
        }
      }

      public bool MoveNext()
      {
        bool result = NativeMethods.FeatureEnumeratorMoveNext(this.DangerousGetHandle());
        GC.KeepAlive(this);

        return result;
      }

      public void Reset()
      {
        NativeMethods.FeatureEnumeratorReset(this.DangerousGetHandle());
        GC.KeepAlive(this);
      }

      object IEnumerator.Current => this.Current;
    }

    private VowpalWabbitExample Example
    {
      get;
      set;
    }

    public namespace_index Index 
    {
      get;
      private set;
    }

    public IEnumerator<VowpalWabbitFeature> GetEnumerator()
    {
      return new FeatureEnumerator(this.Example, this.Index);
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
      return this.GetEnumerator();
    }
  }
}