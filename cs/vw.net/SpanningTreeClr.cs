using System;
using System.Threading;
using System.Runtime.InteropServices;

using Vw.Net.Native;

namespace VW {
  public sealed class SpanningTreeClr : NativeObject<SpanningTreeClr>
  {
    [DllImport("vw.net.native")]
    private static extern IntPtr CreateSpanningTree();

    [DllImport("vw.net.native")]
    private static extern void DeleteSpanningTree(IntPtr handle);

    [DllImport("vw.net.native")]
    private static extern ushort GetSpanningTreePort(IntPtr handle);

    [DllImport("vw.net.native")]
    private static extern void StartSpanningTree(IntPtr handle);

    [DllImport("vw.net.native")]
    private static extern void StopSpanningTree(IntPtr handle);

    [DllImport("vw.net.native")]
    private static extern void RunSpanningTree(IntPtr handle);

    public SpanningTreeClr() : base(new New<SpanningTreeClr>(CreateSpanningTree), new Delete<SpanningTreeClr>(DeleteSpanningTree))
    {
    }

    // TODO: This is not actually projected pre-VW 9
    // public ushort Port
    // {
    //   get
    //   {
    //     return GetSpanningTreePort(Handle);
    //   }
    // }

    public void Start()
    {
      StartSpanningTree(this.DangerousGetHandle());
      GC.KeepAlive(this);
    }

    public void Run()
    {
      RunSpanningTree(this.DangerousGetHandle());
      GC.KeepAlive(this);
    }

    public void Stop()
    {
      StopSpanningTree(this.DangerousGetHandle());
      GC.KeepAlive(this);
    }
  }
}