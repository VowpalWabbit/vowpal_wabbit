using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

using VW.Labels;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern int SetupExample(IntPtr vw, IntPtr ex, IntPtr status);

    [DllImport("vw.net.native")]
    public static extern int CopyExample(IntPtr vw, IntPtr dst, IntPtr src, IntPtr status);
  }
}

namespace VW
{
  using Vw.Net;
  using Vw.Net.Native;

  public sealed class VowpalWabbitExampleBuilder : IDisposable
  {
    private VowpalWabbitExample ex;
    private IVowpalWabbitExamplePool owner;

    public VowpalWabbitExampleBuilder(IVowpalWabbitExamplePool vw)
    {
      if (vw == null)
      {
        throw new ArgumentNullException(nameof(vw));
      }

      this.owner = vw;
      this.ex = this.owner.GetOrCreateNativeExample();
    }

    private VowpalWabbitExampleBuilder(IVowpalWabbitExamplePool vw, VowpalWabbitExample ex)
    {
      this.owner = vw;
      this.ex = ex;
    }

    // Deep-copies the example under construction into a new builder. The copy has not been through
    // VW::setup_example either, so shared namespaces can be built once and stamped into any number of
    // examples, each of which is finalized exactly once by its own CreateExample call.
    public VowpalWabbitExampleBuilder Clone()
    {
      if (this.ex == null)
      {
        throw new ObjectDisposedException(nameof(VowpalWabbitExampleBuilder));
      }

      VowpalWabbitExample copy = this.owner.GetOrCreateNativeExample();

      try
      {
        ApiStatus status = new ApiStatus();
        if (NativeMethods.CopyExample(this.owner.DangerousGetNativeHandle(), copy.DangerousGetNativeHandle(), this.ex.DangerousGetNativeHandle(), status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
        {
          throw new VWException(status);
        }

        this.owner.KeepAlive();
        this.ex.KeepAliveNative();
        copy.KeepAliveNative();
        GC.KeepAlive(status);
      }
      catch
      {
        copy.Dispose();
        throw;
      }

      return new VowpalWabbitExampleBuilder(this.owner, copy);
    }

    public VowpalWabbitExample CreateExample()
    {
      if (this.ex != null)
      {
        ApiStatus status = new ApiStatus();
        if (NativeMethods.SetupExample(this.owner.DangerousGetNativeHandle(), this.ex.DangerousGetNativeHandle(), status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
        {
          throw new VWException(status);
        }

        this.owner.KeepAlive();
        this.ex.KeepAliveNative();
        GC.KeepAlive(status);

        VowpalWabbitExample result = Interlocked.Exchange(ref this.ex, null);

        return result;
      }

      return null;
    }

    public void ApplyLabel(ILabel label)
    {
      label.UpdateExample(this.owner.Native, this.ex);
    }

    public VowpalWabbitNamespaceBuilder AddNamespace(byte featureGroup)
    {
      return new VowpalWabbitNamespaceBuilder(this.ex, featureGroup);
    }

    public VowpalWabbitNamespaceBuilder AddNamespace(char featureGroup)
    {
      Debug.Assert(featureGroup < 256, $"{nameof(featureGroup)} must be between 0 and 255");

      return this.AddNamespace((byte)featureGroup);
    }

    // TODO: Enable string-addressed namespaces
    // public VowpalWabbitNamespaceBuilder AddNamespace(string featureGroup)
    // {
    //   throw new NotImplementedException();
    // }

    public void Dispose()
    {
      // This replicates the behaviour from the C#/CLI bindings, but I am not sure
      // that we do not want to return the example to the underlying pool, just
      // because it was partially built. Could there be a better "clear this example"
      // mechanism?
      VowpalWabbitExample localExample = Interlocked.Exchange(ref this.ex, null);
      if (localExample != null)
      {
        localExample.Dispose();
      }
    }
  }
}