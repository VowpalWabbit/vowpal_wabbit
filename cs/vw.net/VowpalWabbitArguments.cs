using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    [StructLayout(LayoutKind.Sequential)]
    public struct vw_basic_arguments_t
    {
      [MarshalAs(UnmanagedType.U1)]
      public bool isTestOnly; 
      public int numPasses;
      public int cbNumberOfActions;
      public float learningRate;
      public float powerT;
    }

    [DllImport("vw.net.native")]
    public static extern void GetWorkspaceBasicArguments(IntPtr workspace, ref vw_basic_arguments_t arguments);

    [DllImport("vw.net.native")]
    public static extern IntPtr GetWorkspaceDataFilename(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern IntPtr GetFinalRegressorFilename(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern IntPtr SerializeCommandLine(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetInitialRegressorFilenamesCount(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern int GetInitialRegressorFilenames(IntPtr workspace, IntPtr filenamesPtr, int count);
  }
}

namespace VW {
  using Vw.Net.Native;

  public sealed class VowpalWabbitArguments
  {
    private VowpalWabbitBase vw;
    private readonly NativeMethods.vw_basic_arguments_t basicArguments;

    internal VowpalWabbitArguments(VowpalWabbitBase vw)
    {
      this.vw = vw;

      this.basicArguments = new NativeMethods.vw_basic_arguments_t();
      NativeMethods.GetWorkspaceBasicArguments(this.vw.DangerousGetHandle(), ref this.basicArguments);
      GC.KeepAlive(this.vw);

      this.data = new Lazy<string>(() => this.GetWorkspaceString(NativeMethods.GetWorkspaceDataFilename));
      this.finalRegressor = new Lazy<string>(() => this.GetWorkspaceString(NativeMethods.GetFinalRegressorFilename));
      this.commandLine = new Lazy<string>(() => this.GetWorkspaceString(NativeMethods.SerializeCommandLine));
      this.initialRegressors = new Lazy<IEnumerable<string>>(this.GetInitialRegressors);
    }

    private string GetWorkspaceString(Func<IntPtr, IntPtr> getter, bool needsFree = false)
    {
      IntPtr nativeString = getter(this.vw.DangerousGetHandle());
      GC.KeepAlive(this.vw);

      string result = NativeMethods.StringMarshallingFunc(nativeString);
      if (needsFree) NativeMethods.FreeDupString(nativeString);

      return result;
    }

    Lazy<string> data;
    public string Data
    {
      get => this.data.Value;
    }

    public bool TestOnly
    {
      get => this.basicArguments.isTestOnly;
    }

    public int NumPasses
    {
      get => this.basicArguments.numPasses;
    }

    private Lazy<string> finalRegressor;
    public string FinalRegressor
    {
      get => this.finalRegressor.Value;
    }

    private unsafe IEnumerable<string> GetInitialRegressors()
    {
      UIntPtr count = NativeMethods.GetInitialRegressorFilenamesCount(this.vw.DangerousGetHandle());
      GC.KeepAlive(this.vw);

      if (count.ToUInt64() == 0)
      {
        return new string[0];
      }

      if (count.ToUInt64() > Int32.MaxValue)
      {
        // This is inconsistent with what happens in the C#/CLI implementation, but that implementation
        // has a downcast from size_t to int (int32_t), and the behaviour when casting is implementation
        // specific. Thus, it is UB when the number of scalars is too large, and introducing an exception
        // should not be considered  a breaking change.
        // TODO: The exception type is not the best, but keep it for consistency with the others.
        throw new ArgumentOutOfRangeException("Number of initial regressors is too large.");
      }

      int dotnetCount = (int)count.ToUInt32();
      string[] filenames = new string[dotnetCount];
      IntPtr[] nativeFilenames = new IntPtr[dotnetCount];

      fixed (IntPtr* nativeFilenamesPtr = nativeFilenames)
      {
        int returned = NativeMethods.GetInitialRegressorFilenames(this.vw.DangerousGetHandle(), new IntPtr(nativeFilenamesPtr), filenames.Length);
        GC.KeepAlive(this.vw);

        Debug.Assert(returned >= 0, "The size returned by GetPredictionActiveMulticlassMoreInfoRequiredClassesCount is insufficient to hold the classes. This is a bug.");
        Debug.Assert(returned == filenames.Length, "Returned class count does not match requested count. This is a bug.");

        for (int i = 0; i < returned; i++)
        {
          // There is no need to Free here, because the strings are owned by a durable object on
          // VW::workspace, and thus did not need to be copied when returned here.
          filenames[i] = NativeMethods.StringMarshallingFunc(nativeFilenames[i]);
        }
      }

      return filenames;
    }

    private Lazy<IEnumerable<string>> initialRegressors;
    public IEnumerable<string> InitialRegressors
    {
      get => this.initialRegressors.Value;
    }

    private Lazy<string> commandLine;
    public string CommandLine
    {
      get => this.commandLine.Value;
    }

    public int ContextualBanditNumberOfActions
    {
      get => this.basicArguments.cbNumberOfActions;
    }

    public float LearningRate
    {
      get => this.basicArguments.learningRate;
    }

    public float PowerT
    {
      get => this.basicArguments.powerT;
    }
  }
}