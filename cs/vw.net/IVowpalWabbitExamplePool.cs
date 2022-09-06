using System;

namespace VW
{
  public interface IVowpalWabbitExamplePool : IDisposable
  {
    VowpalWabbitExample GetOrCreateNativeExample();

    VowpalWabbit Native { get; }

    void ReturnExampleToPool(VowpalWabbitExample example);
  }

  internal static class VowpalWabbitExamplePoolExtensions
  {
    public static IntPtr DangerousGetNativeHandle(this IVowpalWabbitExamplePool pool)
    {
      return pool.Native.DangerousGetHandle();
    }

    public static void KeepAlive(this IVowpalWabbitExamplePool pool)
    {
      GC.KeepAlive(pool.Native);
    }
  }
}