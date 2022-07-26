using System;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Vw.Net.Native
{
  // TODO: This should ideally be modelled as a stack-only type
  internal class NativeException : NativeObject<NativeException>
  {
    [DllImport("vw.net.native")]
    private static extern IntPtr NativeExceptionWhat(IntPtr exception);

    [DllImport("vw.net.native")]
    private static extern IntPtr NativeVwExceptionWhere(IntPtr exception, out int lineNumber);

    internal enum ExceptionTypes
    {
      Std_exception,
      VW_vw_exception
    }

    internal NativeException(IntPtr exceptionHandle, ExceptionTypes typeInfo) : base(exceptionHandle, ownsHandle: false)
    {
      this.TypeInfo = typeInfo;

      IntPtr messagePtr = NativeExceptionWhat(this.DangerousGetHandle());
      this.Message = NativeMethods.StringMarshallingFunc(messagePtr);

      if (typeInfo == ExceptionTypes.VW_vw_exception)
      {
        int lineNumber;
        IntPtr filenamePtr = NativeVwExceptionWhere(this.DangerousGetHandle(), out lineNumber);

        this.LineNumber = lineNumber;
        this.Filename = NativeMethods.StringMarshallingFunc(filenamePtr);
      }

      // It is important for the exception handle to stay alive until after we grab marshal
      // the message and the filename string.
      GC.KeepAlive(this);
    }

    internal ExceptionTypes TypeInfo 
    { 
      get; 
      private set; 
    }

    public string Message
    {
      get;
      private set;
    }

    public string Filename
    {
      get;
      private set;
      
    }

    public int LineNumber
    {
      get;
      private set;
    }
  }
}