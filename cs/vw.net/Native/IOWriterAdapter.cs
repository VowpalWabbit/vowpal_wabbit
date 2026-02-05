using System;
using System.IO;
using System.Runtime;
using System.Threading;
using System.Runtime.InteropServices;

/* Rationale for why this implementation is safe (w.r.t. lifetimes of the managed objects/delegates), while relying
 * on NativeOwnedObject's unpinned GCHandle is in IOReaderAdapter (to prevent duplicating it). In short: the addresses
 * of the functions do not need to be pinned because P/Invoke adds a fixed-location native-only stub.
*/

namespace Vw.Net.Native
{
  internal unsafe delegate long IOAdapterWriteFn(void* buffer, int num_bytes);
  internal delegate void IOAdapterFlushFn();

  internal class IOWriterAdapter : NativeOwnedObject
  {
    [StructLayout(LayoutKind.Sequential)]
    internal struct VTable
    {
      [MarshalAs(UnmanagedType.FunctionPtr)]
      public ReleaseFn Release;
      [MarshalAs(UnmanagedType.FunctionPtr)]
      public IOAdapterWriteFn Write;
      [MarshalAs(UnmanagedType.FunctionPtr)]
      public IOAdapterFlushFn Flush;
    }

    private Stream stream;
    private bool ownsStream;
    private VTable vtable;

    public unsafe IOWriterAdapter(Stream stream, bool ownsStream = true)
    {
      this.stream = stream;
      this.ownsStream = ownsStream;

      this.vtable = new VTable
      {
        Release = this.DangerousGetReleaseHandle(),
        Write = this.Write,
        Flush = this.Flush
      };
    }

    private unsafe long Write(void* buffer, int num_bytes)
    {
      try
      {
        // .NET Standard 2.0 doesn't have Stream.Write(Span<byte>), so use a temp buffer
        byte[] temp = new byte[num_bytes];
        Marshal.Copy((IntPtr)buffer, temp, 0, num_bytes);
        this.stream.Write(temp, 0, num_bytes);
        return num_bytes;
      }
      catch (IOException)
      {
        return -1; // TODO: We really should have a better error raising mechanism in the io stack.
      }
    }

    public VTable GetVTable()
    {
      return this.vtable;
    }

    private void Flush()
    {
      this.stream.Flush();
    }

    protected override void DisposeInternal(bool disposing)
    {
      // This is guaranteed to be called only after the native side (or someone else)
      // has called into the Release() codepath.
      Stream localStream = Interlocked.Exchange(ref this.stream, null);
      if (ownsStream && localStream != null)
      {
        localStream.Dispose();
      }
    }
  }
}