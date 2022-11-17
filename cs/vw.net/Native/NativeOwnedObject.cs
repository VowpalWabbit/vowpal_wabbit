using System;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  internal delegate void ReleaseFn();

  internal abstract class NativeOwnedObject : IDisposable
  {
    private bool native_released;
    private GCHandle object_handle;

    protected NativeOwnedObject(GCHandleType type = GCHandleType.Normal)
    {
      this.native_released = false;
      this.object_handle = GCHandle.Alloc(this, type);
    }

    private void ReleaseImpl()
    {
      this.object_handle.Free();
      this.native_released = true;
      this.Dispose();
    }

    // TODO: Needed?
    protected void DangerousForceRelease()
    {
      // Do not call this unless you really know what you are doing. This only 
      // really exists to attempt to recover from a bad run which failed to properly
      // clean up the object, but can continue running the overall process.
      this.ReleaseImpl();
    }

    protected ReleaseFn DangerousGetReleaseHandle()
    {
      return this.ReleaseImpl;
    }

    protected virtual void DisposeInternal(bool disposing)
    {
    }

    private void Dispose(bool disposing)
    {
      if (!this.native_released)
      {
        // TODO: Is this the right exception type?
        throw new InvalidOperationException("NativeOwnedObject is .Disposing() before native side has Release()d it.");
      }

      this.DisposeInternal(disposing);
    }


    public void Dispose()
    {
      this.Dispose(disposing: true);
      GC.SuppressFinalize(this);
    }

    // This is reasonably safe, since we will never Finalize this object until the
    // GCHandle is Free()d, which means that we have received a callback to release
    // the underlying object.
    ~NativeOwnedObject()
    {
      this.Dispose(disposing: false);
    }
  }

}