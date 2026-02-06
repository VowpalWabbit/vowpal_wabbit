using System;
using System.IO;
using System.Runtime;
using System.Threading;
using System.Runtime.InteropServices;

/* 
 * Note that this also applies to the implementation of IOWriterAdapter.
 *
 * From: https://docs.microsoft.com/en-us/archive/blogs/cbrumme/asynchronous-operations-pinning
 *
 * Along the same lines, managed Delegates can be marshaled to unmanaged code, where they are exposed as unmanaged 
 * function pointers. Calls on those pointers will perform an unmanaged to managed transition; a change in calling 
 * convention; entry into the correct AppDomain; and any necessary argument marshaling. Clearly the unmanaged 
 * function pointer must refer to a fixed address. It would be a disaster if the GC were relocating that! This 
 * leads many applications to create a pinning handle for the delegate. This is completely unnecessary. The 
 * unmanaged function pointer actually refers to a native code stub that we dynamically generate to perform the 
 * transition & marshaling. This stub exists in fixed memory outside of the GC heap.
 *
 * However, the application is responsible for somehow extending the lifetime of the delegate until no more calls 
 * will occur from unmanaged code. The lifetime of the native code stub is directly related to the lifetime of the 
 * delegate. Once the delegate is collected, subsequent calls via the unmanaged function pointer will crash or 
 * otherwise corrupt the process.
 */

namespace Vw.Net.Native
{
  internal unsafe delegate long IOAdapterReadFn(void* buffer, int num_bytes);
  internal delegate void IOAdapterResetFn();

  internal class IOReaderAdapter : NativeOwnedObject
  {
    [StructLayout(LayoutKind.Sequential)]
    internal struct VTable
    {
      [MarshalAs(UnmanagedType.FunctionPtr)]
      public ReleaseFn Release;
      [MarshalAs(UnmanagedType.FunctionPtr)]
      public IOAdapterReadFn Read;
      [MarshalAs(UnmanagedType.FunctionPtr)]
      public IOAdapterResetFn Reset;
      public bool IsResettable;
    }

    private Stream stream;
    private VTable vtable;

    public unsafe IOReaderAdapter(Stream stream)
    {
      this.stream = stream;
      this.vtable = new VTable
      {
        Release = this.DangerousGetReleaseHandle(),
        Read = this.Read,
        Reset = this.Reset,
        IsResettable = this.stream.CanSeek
      };
    }

    public VTable GetVTable()
    {
      return this.vtable;
    }

    private unsafe long Read(void* buffer, int num_bytes)
    {
      try
      {
#if NETSTANDARD2_1_OR_GREATER
        Span<byte> bufferSpan = new Span<byte>((byte*)buffer, num_bytes);
        return this.stream.Read(bufferSpan);
#else
        byte[] temp = new byte[num_bytes];
        int bytesRead = this.stream.Read(temp, 0, num_bytes);
        if (bytesRead > 0)
        {
          Marshal.Copy(temp, 0, (IntPtr)buffer, bytesRead);
        }
        return bytesRead;
#endif
      }
      catch (IOException)
      {
        return -1; // TODO: We really should have a better error raising mechanism in the io stack.
      }
    }

    private void Reset()
    {
      this.stream.Seek(0, SeekOrigin.Begin);
    }

    protected override void DisposeInternal(bool disposing)
    {
      // This is guaranteed to be called only after the native side (or someone else)
      // has called into the Release() codepath.
      Stream localStream = Interlocked.Exchange(ref this.stream, null);
      if (localStream != null)
      {
        localStream.Dispose();
      }
    }
  }
}