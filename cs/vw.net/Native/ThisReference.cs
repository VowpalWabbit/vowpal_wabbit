using System;

namespace Vw.Net.Native
{
  internal sealed class ThisReference<T> where T : NativeObject<T>
  {
    public ThisReference()
    {
    }

    public T This
    {
        get;
        set;
    }
  }
}