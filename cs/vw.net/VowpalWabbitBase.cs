using System;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    public delegate void trace_message_t(IntPtr trace_context, IntPtr message);

    [DllImport("vw.net.native")]
    public static extern IntPtr CreateWorkspaceWithSeedVwModel(IntPtr seed_workspace, IntPtr arguments, UIntPtr argument_size, trace_message_t trace_message, IntPtr trace_context, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern IntPtr CreateWorkspaceWithModelData(IntPtr arguments, UIntPtr argument_size, IOReaderAdapter.VTable model_reader, trace_message_t trace_listener, IntPtr trace_context, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern IntPtr CreateWorkspace(IntPtr arguments, UIntPtr argument_size, trace_message_t trace_listener, IntPtr trace_context, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int DeleteWorkspace(IntPtr workspace, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern VW.prediction_type_t WorkspaceGetOutputPredictionType(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceReload(IntPtr workspace, IntPtr arguments, UIntPtr argument_size, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceSavePredictorToFile(IntPtr workspace, IntPtr filename, UIntPtr filename_size, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceSavePredictorToWriter(IntPtr workspace, IOWriterAdapter.VTable model_writer, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern IntPtr WorkspaceGetIdDup(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern void WorkspaceSetId(IntPtr workspace, IntPtr id, UIntPtr id_length);
  }
}

namespace VW {
  using Vw.Net;
  using Vw.Net.Native;

  // Do not subclass this directly? Maybe expose a VWWorkspace class instead?
  public abstract class VowpalWabbitBase : NativeObject<VowpalWabbitBase>
  {
    private GCHandle traceListenerHandle;
    private VowpalWabbitModel seedModel;
    private bool seedModelNeedsRelease;

    private static void PipeTraceCallback(IntPtr trace_context, IntPtr messagePtr)
    {
      Action<string> traceListener = GCHandle.FromIntPtr(trace_context).Target as Action<string>;
      if (traceListener != null)
      {
        ulong messageSizeLong = NativeMethods.StdStringGetLength(messagePtr).ToUInt64();
        if (messageSizeLong >= Int32.MaxValue)
        {
          // TODO: This should never actually happen
          messageSizeLong = Int32.MaxValue;
        }

        byte[] messageBuffer = new byte[(int)messageSizeLong];
        unsafe
        {
          fixed (byte* messageBufferPtr = messageBuffer)
          {
            int returned = NativeMethods.StdStringCopyToBuffer(messagePtr, new IntPtr(messageBufferPtr), (int)messageSizeLong);

            Debug.Assert(returned >= 0, "The size returned by StdStringGetLength is insufficient to hold the messagePtr. This is a bug.");
            Debug.Assert(returned == messageBuffer.Length, "Returned byte count does not match requested count. This is a bug.");
          }
        }

        // TODO: There may be a way to avoid copying the string.
        string message = NativeMethods.StringEncoding.GetString(messageBuffer);

        traceListener(message);
      }
    }

    private unsafe static New<VowpalWabbitBase> BindConstructorArguments(VowpalWabbitSettings settings, out IntPtr traceContext, out ApiStatus status)
    {
      ApiStatus localStatus = status = new ApiStatus();
      NativeMethods.trace_message_t traceListener = null;

      IntPtr localTraceContext = IntPtr.Zero;
      // This is a little convoluted, but there is no good clean way to do this without
      // either exposing implementation details or making the contract of NativeObject 
      // less clean/safe.
      // The goal is to make sure that if there is a trace listener Action, then we pass
      // it to the native code. That means it needs to have a GCHandle allocated for it,
      // so it does not get deallocated while the native side is still holding on to it.
      // In turn, that means we need to have access to the GCHandle instance on Dispose,
      // so we need to pass it out of the BindConstructorArguments method (static) and
      // into the instance via the constructor. This necessitates the out-param.
      if (settings.TraceListener != null)
      {
          GCHandle traceListenerHandle = GCHandle.Alloc(settings.TraceListener);
          localTraceContext = GCHandle.ToIntPtr(traceListenerHandle);
          traceListener = PipeTraceCallback;
      }

      traceContext = localTraceContext;

      return new New<VowpalWabbitBase>(() =>
      {
        string arguments = settings.Arguments ?? string.Empty;
        
        EnsureArguments(ref arguments, settings);

        byte[] argumentsBytes = Vw.Net.Native.NativeMethods.StringEncoding.GetBytes(arguments);
        fixed (byte* argumentsPtr = argumentsBytes)
        {
          var argumentsSize = (UIntPtr)argumentsBytes.Length;

          if (settings.Model != null)
          {
            IntPtr result = NativeMethods.CreateWorkspaceWithSeedVwModel(settings.Model.DangerousGetHandle(), new IntPtr(argumentsPtr), argumentsSize, traceListener, localTraceContext, localStatus.ToNativeHandleOrNullptrDangerous());
            GC.KeepAlive(settings.Model);

            if (result == IntPtr.Zero || localStatus.ErrorCode != NativeMethods.SuccessStatus)
            {
              throw new VWException(localStatus);
            }

            return result;
          }
          else if (settings.ModelStream != null)
          {
            // IOReaderAdapter takes ownership of the ModelStream, and will Dispose() it when the
            // native side invokes the Release() method out of the VTable.
            IOReaderAdapter modelReader = new IOReaderAdapter(settings.ModelStream);

            IntPtr result = NativeMethods.CreateWorkspaceWithModelData(new IntPtr(argumentsPtr), argumentsSize, modelReader.GetVTable(), traceListener, localTraceContext, localStatus.ToNativeHandleOrNullptrDangerous());

            if (result == IntPtr.Zero || localStatus.ErrorCode != NativeMethods.SuccessStatus)
            {
              throw new VWException(localStatus);
            }

            return result;
          }
          else
          {
            IntPtr result = NativeMethods.CreateWorkspace(new IntPtr(argumentsPtr), argumentsSize, traceListener, localTraceContext, localStatus.ToNativeHandleOrNullptrDangerous());
            if (result == IntPtr.Zero || localStatus.ErrorCode != NativeMethods.SuccessStatus)
            {
              throw new VWException(localStatus);
            }

            return result;
          }
        }
      });

      bool ArgumentStringHasQuiet(string arguments)
      {
        return arguments.Contains("--quiet ") || arguments.EndsWith("--quiet");
      }

      bool ArgumentStringHasNoStdIn(string arguments)
      {
        return arguments.Contains("--no_stdin ") || arguments.EndsWith("--no_stdin");
      }

      void EnsureArguments(ref string arguments, VowpalWabbitSettings vwSettings)
      {
        if (vwSettings.Model != null)
        {
          if (!vwSettings.Verbose && !ArgumentStringHasQuiet(arguments) && !ArgumentStringHasQuiet(vwSettings.Model.Arguments.CommandLine))
          {
            arguments += " --quiet";
          }
        }
        else
        {
          if (!ArgumentStringHasNoStdIn(arguments))
          {
            arguments += " --no_stdin";
          }

          if (!vwSettings.Verbose && vwSettings.ModelStream == null && !ArgumentStringHasQuiet(arguments))
          {
            arguments += " --quiet";
          }
        }
      }
    }

    private static Delete<VowpalWabbitBase> BindOperatorDelete(out ThisReference<VowpalWabbitBase> thisSlot)
    {
      ThisReference<VowpalWabbitBase> localTarget = thisSlot = new ThisReference<VowpalWabbitBase>();
      return (IntPtr handle) => localTarget.This.OperatorDelete(handle);
    }

    // TODO: Is it actually accurate to say that state will not be corrupted on a failure to delete a
    // workspace?
    [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
    private void OperatorDelete(IntPtr workspace)
    {
      using (ApiStatus status = new ApiStatus())
      {
        // It is so messed up that we can throw while shutting down.
        if (NativeMethods.DeleteWorkspace(workspace, status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
        {
          throw new VWException(status);
        }
      }

      // Release the seed model reference AFTER deleting this workspace. The seeded
      // workspace shares weights and shared_data with the seed model via shallow_copy/
      // shared_ptr. If we release before deleting, the seed model may be destroyed
      // while this workspace still needs the shared resources, causing an
      // AccessViolationException.
      if (this.seedModel != null &&
          this.seedModelNeedsRelease)
      {
        this.seedModel.DangerousRelease();
      }
    }

    protected VowpalWabbitBase(VowpalWabbitSettings settings)
      : base(BindConstructorArguments(settings, out IntPtr traceContext, out ApiStatus status), 
             BindOperatorDelete(out ThisReference<VowpalWabbitBase> thisSlot))
    {
      thisSlot.This = this;

      // In theory, we should never return here, because our OperatorNew should throw, but keep this around for
      // defense-in-depth.
      if (status.ErrorCode != NativeMethods.SuccessStatus)
      {
        // memory leak, potentially, but better than leaving a half-initialized object around
        this.SetHandleAsInvalid();
        throw new VWException(status);
      }

      if (settings.EnableThreadSafeExamplePooling)
      {
        this.m_examples = Bag.CreateLockFree<VowpalWabbitExample>(settings.MaxExamples);
      }
      else
      {
        this.m_examples = Bag.Create<VowpalWabbitExample>(settings.MaxExamples);
      }

      if (settings.Model != null)
      {
        this.seedModel = settings.Model;
        this.seedModel.DangerousAddRef(ref this.seedModelNeedsRelease);
        if (!this.seedModelNeedsRelease)
        {
          throw new VowpalWabbitException("Unable to maintain seed model reference count.");
        }
      }

      this.Settings = settings;

      this.traceListenerHandle = traceContext != IntPtr.Zero ? GCHandle.FromIntPtr(traceContext) : default(GCHandle);
    }
    
    //internal int m_instanceCount;

    [Obsolete("Use .DangerousGetHandle() instead.")]
    internal IntPtr m_vw => this.DangerousGetHandle();

    // Ideally, this should only be checking IsClosed.
    protected bool m_isDisposed => this.IsClosed || this.IsInvalid;

    //~VowpalWabbitBase()
    //{
    //  this.Dispose(false);
    //}

    [Obsolete("Calling InternalDispose() from consumers of the binding layer is undefined behaviour.")]
    protected void InternalDispose()
    {
      //TODO: Should we throw here?
    }

    [Obsolete("Calling DisposeExample() from consumers of the binding layer is undefined behaviour.")]
    protected void DisposeExample(VowpalWabbitExample ex)
    {
      //TODO: Should we throw here?
    }

    public VowpalWabbitSettings Settings
    {
      get;
      private set;
    }

    public VowpalWabbitArguments Arguments => new VowpalWabbitArguments(this);

    public string ID
    {
      get
      {
        IntPtr idPtr = NativeMethods.WorkspaceGetIdDup(this.DangerousGetHandle());
        GC.KeepAlive(this);

        try
        {
          return NativeMethods.StringMarshallingFunc(idPtr);
        }
        finally
        {
          NativeMethods.FreeDupString(idPtr);
        }

      }
      set
      {
        byte[] valueBytes = NativeMethods.StringEncoding.GetBytes(value);
        unsafe
        {
          fixed (byte* valuePtr = valueBytes)
          {
            NativeMethods.WorkspaceSetId(this.DangerousGetHandle(), new IntPtr(valuePtr), new UIntPtr((uint)valueBytes.Length));
          }
        }
      }
    }

    public void Reload(string args = null)
    {
      if (this.Settings.ParallelOptions != null)
      {
        throw new NotSupportedException("Cannot reaload model if AllReduce is enabled.");
      }

      if (args == null) args = String.Empty;

      byte[] argumentsBytes = Vw.Net.Native.NativeMethods.StringEncoding.GetBytes(args);
      unsafe
      { 
        fixed (byte* argumentsPtr = argumentsBytes)
        {
          ApiStatus status = new ApiStatus();
          NativeMethods.WorkspaceReload(this.DangerousGetHandle(), new IntPtr(argumentsPtr), new UIntPtr((uint)argumentsBytes.Length), status.ToNativeHandleOrNullptrDangerous());
          
          if (status.ErrorCode != NativeMethods.SuccessStatus)
          {
            // It is not clear what state we are left in after this exception.
            throw new VWException(status);
          }
        }
      }
    }

    public string AreFeaturesCompatible(VowpalWabbitBase other)
    {
      throw new NotImplementedException("AreFeaturesCompatible");
    }

    public void SaveModel() 
    {
      string finalRegressor = this.Arguments.FinalRegressor;
      if (finalRegressor == null) return;

      this.SaveModel(finalRegressor);
    }

    public void SaveModel(string filename) 
    {
      string directoryName = Path.GetDirectoryName(filename);
      if (!string.IsNullOrWhiteSpace(directoryName) &&
          !Directory.Exists(directoryName))
      {
        Directory.CreateDirectory(directoryName);
      }

      byte[] filenameBytes = Vw.Net.Native.NativeMethods.StringEncoding.GetBytes(filename);
      unsafe
      {
        fixed (byte* filenamePtr = filenameBytes)
        {
          ApiStatus status = new ApiStatus();
          NativeMethods.WorkspaceSavePredictorToFile(this.DangerousGetHandle(), new IntPtr(filenamePtr), new UIntPtr((uint)filenameBytes.Length), status.ToNativeHandleOrNullptrDangerous());
          GC.KeepAlive(this);
        
          if (status.ErrorCode != NativeMethods.SuccessStatus)
          {
            // It is not clear what state we are left in after this exception.
            throw new VWException(status);
          }
        }
      }
    }

    public void SaveModel(Stream stream)
    {
      if (stream == null)
      {
        throw new ArgumentNullException("stream");
      }

      ApiStatus status = new ApiStatus();
      IOWriterAdapter writer = new IOWriterAdapter(stream);
      NativeMethods.WorkspaceSavePredictorToWriter(this.DangerousGetHandle(), writer.GetVTable(), status.ToNativeHandleOrNullptrDangerous());
      GC.KeepAlive(this);

      if (status.ErrorCode != NativeMethods.SuccessStatus)
      {
        // It is not clear what state we are left in after this exception.
        throw new VWException(status);
      }
    }

    // This does not get used by the base class at all, so not sure why it is here. On the flip side,
    // if other implementors of VowpalWabbitBase (why?) need to access this, it will prevent them from
    // needing to deeply now about the different types of IBag implementations in Common.
    protected IBag<VowpalWabbitExample> m_examples;

    internal prediction_type_t GetOutputPredictionType()
    {
      prediction_type_t result = NativeMethods.WorkspaceGetOutputPredictionType(this.DangerousGetHandle());
      GC.KeepAlive(this);

      return result;
    }
  }
}