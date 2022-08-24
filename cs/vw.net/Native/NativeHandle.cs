using System;
using System.Collections.Generic;
using System.Text;
using Vw.Net.Native;

namespace Vw.Net.Native
{
  internal class NativeHandle : NativeObject<NativeHandle>
  {
    private NativeHandle(New<NativeHandle> operatorNew, Delete<NativeHandle> operatorDelete) : base(() => operatorNew(), (ptr) => operatorDelete(ptr))
    {
    }

    private NativeHandle(IntPtr sharedHandle, bool ownsHandle, Delete<NativeHandle> operatorDelete = null) : base(sharedHandle, ownsHandle, operatorDelete)
    {
    }

    public NativeHandle DangerousMakeNonOwningCopy()
    {
      NativeHandle result = new NativeHandle(this.DangerousGetHandle(), false);
      GC.KeepAlive(this);

      return result;
    }

    public static NativeHandle MakeNative(Func<IntPtr> operatorNew, Action<IntPtr> operatorDelete)
    {
      return new NativeHandle(new New<NativeHandle>(operatorNew), new Delete<NativeHandle>(operatorDelete));
    }
  }
}
