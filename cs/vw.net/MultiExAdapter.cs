using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime;
using System.Threading;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  
  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native.dll")]
    public static extern IntPtr CreateMultiEx();

    [DllImport("vw.net.native.dll")]
    public static extern void DeleteMultiEx(IntPtr multi_ex);

    [DllImport("vw.net.native.dll")]
    public static extern void MultiExAddExample(IntPtr multi_ex, IntPtr example);
  }
}

namespace VW
{
  using Vw.Net;
  using Vw.Net.Native;

  // VW currently rely on passing in std::vector<example*> for multi-line  examples. On the .NET
  // side, the existing implementation uses List<VowpalWabbitExample>, which is not marshallable
  // to the native side. Since these get used in the core of learning loops, we want to avoid an
  // extra array allocation, copies, and pinning, so we populate the multi_ex using the adapter.

  internal class MultiExAdapter : NativeObject<MultiExAdapter>
  {
    private List<VowpalWabbitExample> exampleCollection;

    public MultiExAdapter() : base(new New<MultiExAdapter>(NativeMethods.CreateMultiEx), new Delete<MultiExAdapter>(NativeMethods.DeleteMultiEx))
    {
      this.exampleCollection = new List<VowpalWabbitExample>();
    }

    public MultiExAdapter(List<VowpalWabbitExample> exampleCollection) : this()
    {
      this.exampleCollection = exampleCollection;
      foreach (VowpalWabbitExample example in this.exampleCollection)
      {
        NativeMethods.MultiExAddExample(this.DangerousGetHandle(), example.DangerousGetHandle());
        // GC.KeepAlive(this); is not needed because this is the constructor
        // GC.KeepAlive(example) is unneeded because all of the examples are kept in
        // the locally-owned exampleCollection. This means that as long as this is kept
        // alive, so is every example.
      }
    }

    public void AddExample(VowpalWabbitExample example)
    {
      NativeMethods.MultiExAddExample(this.DangerousGetHandle(), example.DangerousGetHandle());
      GC.KeepAlive(this);
      // GC.KeepAlive(example) is unneeded because all of the examples are kept in
      // the locally-owned exampleCollection. This means that as long as this is kept
      // alive, so is every example.

      this.exampleCollection.Add(example);
    }

    public int Count => this.exampleCollection.Count;

    public VowpalWabbitExample this[int index] => this.exampleCollection[index];
  }
}