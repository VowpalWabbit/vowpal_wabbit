using System;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public extern static IntPtr ComputeDiffDescriptionSimpleLabels(IntPtr ex1, IntPtr ex2);

    [DllImport("vw.net.native")]
    internal extern static IntPtr ComputeDiffDescriptionCbLabels(IntPtr ex1, IntPtr ex2);
  }
}

namespace VW
{
  using Vw.Net.Native;

  public interface IVowpalWabbitLabelComparator
  {
    string Diff(VowpalWabbitExample ex1, VowpalWabbitExample ex2);
  }

  public sealed class VowpalWabbitSimpleLabelComparator : IVowpalWabbitLabelComparator
  {
    

    public string Diff(VowpalWabbitExample ex1, VowpalWabbitExample ex2)
    {
      IntPtr diffStringPtr = NativeMethods.ComputeDiffDescriptionSimpleLabels(ex1.DangerousGetNativeHandle(), ex2.DangerousGetNativeHandle());
      ex1.KeepAliveNative();
      ex2.KeepAliveNative();

      if (diffStringPtr == IntPtr.Zero)
      {
        return null;
      }

      // It is somewhat unfortunate that this copy exists here, since it is a double-copy.
      // It would really be handy if the .NET framework could treat std::string as a .NET
      // String equivalent.
      string diffString = NativeMethods.StringMarshallingFunc(diffStringPtr);
      NativeMethods.FreeDupString(diffStringPtr);

      return diffString;
    }
  }

  public sealed class VowpalWabbitContextualBanditLabelComparator : IVowpalWabbitLabelComparator
  {
    public string Diff(VowpalWabbitExample ex1, VowpalWabbitExample ex2)
    {
      IntPtr diffStringPtr = NativeMethods.ComputeDiffDescriptionSimpleLabels(ex1.DangerousGetNativeHandle(), ex2.DangerousGetNativeHandle());
      ex1.KeepAliveNative();
      ex2.KeepAliveNative();

      if (diffStringPtr == IntPtr.Zero)
      {
        return null;
      }

      // It is somewhat unfortunate that this copy exists here, since it is a double-copy.
      // It would really be handy if the .NET framework could treat std::string as a .NET
      // String equivalent.
      string diffString = NativeMethods.StringMarshallingFunc(diffStringPtr);
      NativeMethods.FreeDupString(diffStringPtr);

      return diffString;
    }
  }

  public static class VowpalWabbitLabelComparator
  {
    public static IVowpalWabbitLabelComparator Simple = new VowpalWabbitSimpleLabelComparator();
    public static IVowpalWabbitLabelComparator ContextualBandit = new VowpalWabbitContextualBanditLabelComparator();
  }
}