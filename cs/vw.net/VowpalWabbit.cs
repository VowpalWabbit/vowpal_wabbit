using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;
using VW;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern UIntPtr WorkspaceHashSpace(IntPtr workspace, IntPtr str, UIntPtr len);

    [DllImport("vw.net.native")]
    public static extern UIntPtr WorkspaceHashFeature(IntPtr workspace, IntPtr str, UIntPtr len, UIntPtr _base);

    [DllImport("vw.net.native")]
    public static extern void WorkspaceSetUpAllReduceThreadsRoot(IntPtr workspace, UIntPtr total, UIntPtr node);

    [DllImport("vw.net.native")]
    public static extern void WorkspaceSetUpAllReduceThreadsNode(IntPtr workspace, UIntPtr total, UIntPtr node, IntPtr rootWorkspace);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceRunMultiPass(IntPtr workspace, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceRunDriver(IntPtr workspace, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceRunDriverOneThread(IntPtr workspace, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceNotifyEndOfPass(IntPtr workspace, IntPtr api_status);

    internal unsafe delegate IntPtr ExamplePoolGetExampleFn(IntPtr context);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceParseJson(IntPtr workspace, IntPtr json, UIntPtr json_len, [MarshalAs(UnmanagedType.FunctionPtr)] ExamplePoolGetExampleFn get_example, IntPtr example_pool_context, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceParseDecisionServiceJson(IntPtr workspace, IntPtr json, UIntPtr json_len, UIntPtr offset, bool copy_json, [MarshalAs(UnmanagedType.FunctionPtr)] ExamplePoolGetExampleFn get_example, IntPtr example_pool_context, IntPtr interaction, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceGetTopicCount(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern ulong WorkspaceGetTopicSize(IntPtr workspace);

    [DllImport("vw.net.native")]
    public static extern long WorkspaceFillTopicAllocation(IntPtr workspace, IntPtr topic_weights_buffers, int topic_weights_buffer_size, int topic_weights_buffers_count);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceFillSingleTopicTopWeights(IntPtr workspace, int topic, IntPtr feature_buffer, int feature_buffer_size);

    internal delegate void CreatePredictionCallback();

    [DllImport("vw.net.native")]
    public static extern int WorkspaceParseSingleLine(IntPtr workspace, IntPtr example, IntPtr line, UIntPtr line_length, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceLearn(IntPtr workspace, IntPtr example, [MarshalAs(UnmanagedType.FunctionPtr)] CreatePredictionCallback create_prediction, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspacePredict(IntPtr workspace, IntPtr example, [MarshalAs(UnmanagedType.FunctionPtr)] CreatePredictionCallback create_prediction, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspaceLearnMulti(IntPtr workspace, IntPtr example_pool, [MarshalAs(UnmanagedType.FunctionPtr)] CreatePredictionCallback create_prediction, IntPtr api_status);

    [DllImport("vw.net.native")]
    public static extern int WorkspacePredictMulti(IntPtr workspace, IntPtr example_pool, [MarshalAs(UnmanagedType.FunctionPtr)] CreatePredictionCallback create_prediction, IntPtr api_status);
  }

  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern IntPtr CreateDecisionServiceInteraction();
        
    [DllImport("vw.net.native")]
    public static extern void DeleteDecisionServiceInteraction(IntPtr interaction);

    [DllImport("vw.net.native")]
    public static extern IntPtr GetDSInteractionEventIdDup(IntPtr interaction);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetDSInteractionActionsCount(IntPtr interaction);

    [DllImport("vw.net.native")]
    public static extern int GetDSInteractionActions(IntPtr interaction, IntPtr actions_buffer, int actions_buffer_size);

    [DllImport("vw.net.native")]
    public static extern UIntPtr GetDSInteractionProbabilitiesCount(IntPtr interaction);

    [DllImport("vw.net.native")]
    public static extern int GetDSInteractionProbabilities(IntPtr interaction, IntPtr probabilities_buffer, int probabilities_buffer_size);

    [DllImport("vw.net.native")]
    public static extern float GetDSInteractionProbabilityOfDrop(IntPtr interaction);

    [DllImport("vw.net.native")]
    public static extern bool GetDSInteractionSkipLearn(IntPtr interaction);
  }



  internal class DecisionServiceInteractionAdapter : NativeObject<DecisionServiceInteractionAdapter>
  {
    public DecisionServiceInteractionAdapter() :
      base(NativeMethods.CreateDecisionServiceInteraction, NativeMethods.DeleteDecisionServiceInteraction)
    {
    }

    public string EventId
    {
      get 
      {
        IntPtr eventIdPtr = NativeMethods.GetDSInteractionEventIdDup(this.DangerousGetHandle());
        GC.KeepAlive(this);

        string result = NativeMethods.StringMarshallingFunc(eventIdPtr);
        NativeMethods.FreeDupString(eventIdPtr);

        return result;
      }
    }

    public int[] Actions
    {
      get
      {
        UIntPtr count = NativeMethods.GetDSInteractionActionsCount(this.DangerousGetHandle());
        if (count.ToUInt64() > Int32.MaxValue)
        {
            throw new ArgumentOutOfRangeException("Action Set too large");
        }
        
        // TODO: The type here should really be uint (since it is it uint32_t on the other side), but 
        // the C#/CLI implementation is using int, and we need to match that. This is fine, unless we 
        // get over 2 billion actions...
        int[] result = new int[(int)count.ToUInt32()];
        
        unsafe
        {
          fixed (int* resultPtr = result)
          {
            int returned = NativeMethods.GetDSInteractionActions(this.DangerousGetHandle(), new IntPtr(resultPtr), result.Length);
            GC.KeepAlive(this);
            
            Debug.Assert(returned >= 0, "The size returned by GetDSInteractionActions is insufficient to hold the action set. This is a bug.");
            Debug.Assert(returned == result.Length, "Returned action set count does not match requested count. This is a bug.");
          }
        }
        
        return result;
      }
    }

    public float[] Probabilities
    {
      get
      {
        UIntPtr count = NativeMethods.GetDSInteractionProbabilitiesCount(this.DangerousGetHandle());
        if (count.ToUInt64() > Int32.MaxValue)
        {
            throw new ArgumentOutOfRangeException("PMF too large");
        }
        
        float[] result = new float[(int)count.ToUInt32()];
        
        unsafe
        {
          fixed (float* resultPtr = result)
          {
            int returned = NativeMethods.GetDSInteractionProbabilities(this.DangerousGetHandle(), new IntPtr(resultPtr), result.Length);
            GC.KeepAlive(this);
            
            Debug.Assert(returned >= 0, "The size returned by GetDSInteractionProbabilities is insufficient to hold the PMF. This is a bug.");
            Debug.Assert(returned == result.Length, "Returned probabilities count does not match requested count. This is a bug.");
          }
        }
        
        return result;
      }
    }

    public float ProbabilityOfDrop
    {
      get
      {
        float result = NativeMethods.GetDSInteractionProbabilityOfDrop(this.DangerousGetHandle());
        GC.KeepAlive(this);

        return result;
      }
    }

    public bool SkipLearn
    {
      get
      {
        bool result = NativeMethods.GetDSInteractionSkipLearn(this.DangerousGetHandle());
        GC.KeepAlive(this);

        return result;
      }
    }
  
    public VowpalWabbitDecisionServiceInteractionHeader ToInteractionHeader()
    {
      return new VowpalWabbitDecisionServiceInteractionHeader
      {
        EventId = this.EventId,
        Actions = this.Actions,
        Probabilities = this.Probabilities,
        ProbabilityOfDrop = this.ProbabilityOfDrop,
        SkipLearn = this.SkipLearn
      };
    }
  }
}

namespace VW {
    using System.Linq;
    using System.Runtime.CompilerServices;
    using Vw.Net;
  using Vw.Net.Native;

  public sealed class VowpalWabbit : VowpalWabbitBase, IVowpalWabbitExamplePool
  {
    

    public VowpalWabbit(string args) : this(new VowpalWabbitSettings(args))
    {
    }

    public VowpalWabbit(VowpalWabbitSettings settings) : base(settings)
    {
      if (settings.ParallelOptions != null)
      {
        UIntPtr total = new UIntPtr((uint)settings.ParallelOptions.MaxDegreeOfParallelism);

        if (settings.Root == null)
        {
          NativeMethods.WorkspaceSetUpAllReduceThreadsRoot(this.DangerousGetHandle(), total, new UIntPtr(settings.Node));
          GC.KeepAlive(this);
        }
        else
        {
          NativeMethods.WorkspaceSetUpAllReduceThreadsNode(this.DangerousGetHandle(), total, new UIntPtr(settings.Node), settings.Root.DangerousGetHandle());
          GC.KeepAlive(settings.Root);
          GC.KeepAlive(this);
        }
      }
    }

    public void RunMultiPass()
    {
      ApiStatus status = new ApiStatus();
      if (NativeMethods.WorkspaceRunMultiPass(this.DangerousGetHandle(), status.DangerousGetHandle()) != NativeMethods.SuccessStatus)
      {
        throw new VWException(status);
      }
    }

    public VowpalWabbitPerformanceStatistics PerformanceStatistics => new VowpalWabbitPerformanceStatistics(this);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void CheckLineInput(string line)
    {
      if (line == null)
      {
        throw new ArgumentNullException("line");
      }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void CheckLinesInput(IEnumerable<string> lines)
    {
      if (lines == null)
      {
        throw new ArgumentException("lines must not be empty. For multi-line examples use Learn/Predict(IEnumerable<string>) overload.", "line");
      }
    }

    public VowpalWabbitExample ParseLine(string line)
    {
      this.CheckLineInput(line);

      VowpalWabbitExample example = this.GetOrCreateNativeExample();

      try
      {
        unsafe
        {
          ApiStatus status = new ApiStatus();
          byte[] lineBytes = NativeMethods.StringEncoding.GetBytes(line);
          fixed (byte* linePtr = lineBytes)
          {
            if (NativeMethods.WorkspaceParseSingleLine(this.DangerousGetHandle(), example.DangerousGetNativeHandle(), new IntPtr(linePtr), new UIntPtr((uint)lineBytes.Length), status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
            {
              throw new VWException(status);
            }

            example.VowpalWabbitString = line;

            GC.KeepAlive(this);
            example.KeepAliveNative();
            GC.KeepAlive(status);
          }
        }
      }
      catch
      {
        example.Dispose();
        throw;
      }

      return example;
    }
    
    private IntPtr GetOrCreateExample(IntPtr resultPointer)
    {
      List<VowpalWabbitExample> result = (List<VowpalWabbitExample>)GCHandle.FromIntPtr(resultPointer).Target;

      VowpalWabbitExample example = this.GetOrCreateNativeExample();
      result.Add(example);

      return example.DangerousGetNativeHandle();
    }

    public List<VowpalWabbitExample> ParseJson(string line)
    {
      this.CheckLineInput(line);
      
      List<VowpalWabbitExample> result = new List<VowpalWabbitExample>();
      GCHandle resultHandle = GCHandle.Alloc(result, GCHandleType.Normal);

      try
      {
        ApiStatus status = new ApiStatus();
        byte[] lineBytes = NativeMethods.StringEncoding.GetBytes(line);

        unsafe
        {
          fixed (byte* linePtr = lineBytes)
          {
            if (NativeMethods.WorkspaceParseJson(this.DangerousGetHandle(), new IntPtr(linePtr), new UIntPtr((uint)lineBytes.Length), GetOrCreateExample, GCHandle.ToIntPtr(resultHandle), status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
            {
              throw new VWException(status);
            }
          }
        }
        

        // TODO: Is there a good reason why we do not do this for DSJson?
        result[0].VowpalWabbitString = line;
      }
      catch 
      {
        foreach (var example in result)
        {
          example.Dispose();
        }

        result.Clear();

        throw;
      }
      finally
      {
        if (resultHandle.IsAllocated)
        {
          resultHandle.Free();
        }
      }

      return result;
    }

    public List<VowpalWabbitExample> ParseDecisionServiceJson(byte[] json, int offset, int length, bool copyJson, out VowpalWabbitDecisionServiceInteractionHeader header)
    {
      if (json == null)
      {
        throw new ArgumentNullException("json");
      }
      
      List<VowpalWabbitExample> result = new List<VowpalWabbitExample>();
      GCHandle resultHandle = GCHandle.Alloc(result, GCHandleType.Normal);

      try
      {
        ApiStatus status = new ApiStatus();
        DecisionServiceInteractionAdapter interaction = new DecisionServiceInteractionAdapter();

        if (json.Length > 0)
        {
          unsafe 
          {
            fixed (byte* jsonPtr = json)
            {
              if (NativeMethods.WorkspaceParseDecisionServiceJson(this.DangerousGetHandle(), new IntPtr(jsonPtr), new UIntPtr((uint)length), new UIntPtr((uint)offset), copyJson, GetOrCreateExample, GCHandle.ToIntPtr(resultHandle), interaction.DangerousGetHandle(), status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
              {
                throw new VWException(status);
              }

            }
          }
        }
        

        header = interaction.ToInteractionHeader();
        GC.KeepAlive(header);
        GC.KeepAlive(this);
      }
      catch 
      {
        foreach (var example in result)
        {
          example.Dispose();
        }

        result.Clear();

        throw;
      }
      finally
      {
        if (resultHandle.IsAllocated)
        {
          resultHandle.Free();
        }
      }

      return result;
    }

    public ulong HashSpaceNative(string s)
    {
      byte[] sBytes = NativeMethods.StringEncoding.GetBytes(s);

      unsafe
      {
        fixed (byte* sBytesPtr = sBytes)
        {
          UIntPtr result = NativeMethods.WorkspaceHashSpace(this.DangerousGetHandle(), (IntPtr)sBytesPtr, new UIntPtr((uint)sBytes.Length));
          GC.KeepAlive(this);

          return result.ToUInt64();
        }
      }
    }

    public ulong HashSpace(string s)
    {
      return this.HashSpaceNative(s);
    }

    public ulong HashFeatureNative(string s, ulong u)
    {
      byte[] sBytes = NativeMethods.StringEncoding.GetBytes(s);

      unsafe
      {
        fixed (byte* sBytesPtr = sBytes)
        {
          UIntPtr result = NativeMethods.WorkspaceHashFeature(this.DangerousGetHandle(), (IntPtr)sBytesPtr, new UIntPtr((uint)sBytes.Length), new UIntPtr(u));
          GC.KeepAlive(this);

          return result.ToUInt64();
        }
      }
    }

    public ulong HashFeature(string s, ulong u)
    {
      return this.HashFeatureNative(s, u);
    }

    public float[][] GetTopicAllocation()
    {
      int topicCount = NativeMethods.WorkspaceGetTopicCount(this.DangerousGetHandle());
      
      ulong topicSize = NativeMethods.WorkspaceGetTopicSize(this.DangerousGetHandle());
      if (topicSize > Int32.MaxValue)
      {
        // What happened in the C#/CLI implementation is an unchecked c-style cast from 
        // uint64_t to int, which is then passed into the array constructor directly,
        // which will result in an incorrectly-sized array at best (which will throw) 
        // during the individual topic-fills, or during the array construction itself.

        // TODO: Should we simulate the old behaviour in terms of which exceptions are
        // thrown, or throw the new exception?
        throw new ArgumentOutOfRangeException("The number of weights is too large");
      }

      float[][] result = new float[topicCount][];
      GCHandle[] bufferHandles = new GCHandle[topicCount];
      IntPtr[] bufferPtrs = new IntPtr[topicCount];

      for (int k = 0; k < topicCount; k++)
      {
        result[k] = new float[(int)topicSize];
        bufferHandles[k] = GCHandle.Alloc(result[k], GCHandleType.Pinned);
        bufferPtrs[k] = bufferHandles[k].AddrOfPinnedObject();
      }

      try
      {
        unsafe
        {
          long expecting = (long)bufferPtrs.Length * (int)topicCount;

          fixed (IntPtr* bufferPtrsPtr = bufferPtrs)
          {
            long returned = NativeMethods.WorkspaceFillTopicAllocation(this.DangerousGetHandle(), new IntPtr(bufferPtrsPtr), (int)topicSize, bufferPtrs.Length);
            GC.KeepAlive(this);

            Debug.Assert(returned > 0, "The size of the buffers (or number thereof) is insufficient to contain the topic allocations. This is a bug.");
            Debug.Assert(returned == expecting, "Returned floats count does not match requested count. This is a bug.");
          }
        }
      }
      finally
      {
        for (int k = 0; k < topicCount; k++)
        {
          if (bufferHandles[k].IsAllocated)
          {
            bufferHandles[k].Free();
          }
        }
      }

      return result;
    }

    private NativeFeature[] GetTopWeights(int topic, int top)
    {
      ulong count = NativeMethods.WorkspaceGetTopicSize(this.DangerousGetHandle());
      int actualTopCount = Math.Min((int)count, top);
        
      NativeFeature[] result = new NativeFeature[actualTopCount];
        
      unsafe
      {
        fixed (NativeFeature* resultPtr = result)
        {
          int returned = NativeMethods.WorkspaceFillSingleTopicTopWeights(this.DangerousGetHandle(), topic, new IntPtr(resultPtr), result.Length);
          GC.KeepAlive(this);

          Debug.Assert(returned > 0, "The size requested is insufficient to contain the features. This is a bug.");
          Debug.Assert(returned == result.Length, "Returned features count does not match requested count. This is a bug.");
        }
      }
        
      return result;
    }

    public List<VowpalWabbitFeature>[] GetTopicAllocation(int top)
    {
      int topicCount = NativeMethods.WorkspaceGetTopicCount(this.DangerousGetHandle());

      List<VowpalWabbitFeature>[] result = new List<VowpalWabbitFeature>[topicCount];

      for (int topic = 0; topic < topicCount; topic++)
      {
        result[topic] = this.GetTopWeights(topic, top)
                            .Select(f => new VowpalWabbitFeature(this, f))
                            .ToList();
      }

      return result;
    }

    private void CheckExample(VowpalWabbitExample ex)
    {
      if (ex == null)
        throw new ArgumentNullException(nameof(ex));
    }

    private void CheckExamplePredictionFactory<T>(VowpalWabbitExample ex, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      this.CheckExample(ex);

      if (predictionFactory == null)
        throw new ArgumentNullException(nameof(predictionFactory));
    }

    private void InvokeLearner(Func<IntPtr, IntPtr, NativeMethods.CreatePredictionCallback, IntPtr, int> learnerCall, VowpalWabbitExample ex, NativeMethods.CreatePredictionCallback createPrediction = null)
    {
      // TODO: Needed?
      if (createPrediction == null) createPrediction = () => {};

      using (ApiStatus status = new ApiStatus())
      {
        if (learnerCall(this.DangerousGetHandle(), ex.DangerousGetNativeHandle(), createPrediction, status.ToNativeHandleOrNullptrDangerous())  != NativeMethods.SuccessStatus)
        {
          throw new VWException(status);
        }

        GC.KeepAlive(this);
        ex.KeepAliveNative();
        GC.KeepAlive(status);
      }
    }

    public T Learn<T>(VowpalWabbitExample ex, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      this.CheckExamplePredictionFactory(ex, predictionFactory);

      T resultSlot = default(T);
      NativeMethods.CreatePredictionCallback createPrediction = () => resultSlot = predictionFactory.Create(this, ex);

      InvokeLearner(NativeMethods.WorkspaceLearn, ex, createPrediction);

      return resultSlot;
    }

    public T Predict<T>(VowpalWabbitExample ex, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      this.CheckExamplePredictionFactory(ex, predictionFactory);

      T resultSlot = default(T);
      NativeMethods.CreatePredictionCallback createPrediction = () => resultSlot = predictionFactory.Create(this, ex);

      InvokeLearner(NativeMethods.WorkspacePredict, ex, createPrediction);

      return resultSlot;
    }

    public void Learn(VowpalWabbitExample ex)
    {
      InvokeLearner(NativeMethods.WorkspaceLearn, ex);
    }

    private void InvokeMultiLearner(Func<IntPtr, IntPtr, NativeMethods.CreatePredictionCallback, IntPtr, int> learnerCall, MultiExAdapter multiEx, NativeMethods.CreatePredictionCallback createPrediction = null)
    {
      // TODO: Needed?
      //createPrediction ??= () => {};

      using (ApiStatus status = new ApiStatus())
      {
        if (learnerCall(this.DangerousGetHandle(), multiEx.DangerousGetHandle(), createPrediction, status.ToNativeHandleOrNullptrDangerous())  != NativeMethods.SuccessStatus)
        {
          throw new VWException(status);
        }

        GC.KeepAlive(this);
        GC.KeepAlive(multiEx);
        GC.KeepAlive(status);
      }
    }

    public void Learn(List<VowpalWabbitExample> examples)
    {
      using (MultiExAdapter multiEx = new MultiExAdapter(examples))
      {
        this.InvokeMultiLearner(NativeMethods.WorkspaceLearnMulti, multiEx);
      }
    }

    public void Predict(VowpalWabbitExample ex)
    {
      this.InvokeLearner(NativeMethods.WorkspacePredict, ex);
    }

    public void Predict(List<VowpalWabbitExample> examples)
    {
      using (MultiExAdapter multiEx = new MultiExAdapter(examples))
      {
        this.InvokeMultiLearner(NativeMethods.WorkspacePredictMulti, multiEx);
      }
    }

    public void Learn(string line)
    {
      using (VowpalWabbitExample example = this.ParseLine(line))
      {
        this.Learn(example);
      }
    }

    public void Predict(string line)
    {
      using (VowpalWabbitExample example = this.ParseLine(line))
      {
        this.Predict(example);
      }
    }

    public T Learn<T>(string line, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      using (VowpalWabbitExample example = this.ParseLine(line))
      {
        return this.Learn(example, predictionFactory);
      }
    }

    public T Predict<T>(string line, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      using (VowpalWabbitExample example = this.ParseLine(line))
      {
        return this.Predict(example, predictionFactory);
      }
    }

    private void CacheEmptyLine()
    {
      VowpalWabbitExample empty = this.GetOrCreateNativeExample();
      empty.MakeEmpty(this);
      this.ReturnExampleToPool(empty);
    }

    private void ParseLinesToMultiEx(IEnumerable<string> lines, MultiExAdapter multiEx)
    {
      CheckLinesInput(lines);

      foreach (var line in lines)
      {
        multiEx.AddExample(this.ParseLine(line));
      }

      this.CacheEmptyLine();
    }

    public void Learn(IEnumerable<string> lines)
    {
      using (MultiExAdapter multiEx = new MultiExAdapter())
      {
        this.ParseLinesToMultiEx(lines, multiEx);
        this.InvokeMultiLearner(NativeMethods.WorkspaceLearnMulti, multiEx);
      }
    }

    public void Predict(IEnumerable<string> lines)
    {
      using (MultiExAdapter multiEx = new MultiExAdapter())
      {
        this.ParseLinesToMultiEx(lines, multiEx);
        this.InvokeMultiLearner(NativeMethods.WorkspacePredictMulti, multiEx);
      }
    }

    public T Learn<T>(IEnumerable<string> lines, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      T resultSlot = default(T);

      using (MultiExAdapter multiEx = new MultiExAdapter())
      {
        NativeMethods.CreatePredictionCallback createPrediction = () => resultSlot = predictionFactory.Create(this, multiEx[0]);

        this.ParseLinesToMultiEx(lines, multiEx);
        this.InvokeMultiLearner(NativeMethods.WorkspaceLearnMulti, multiEx, createPrediction);
        GC.KeepAlive(createPrediction);
      }

      return resultSlot;
    }

    public T Predict<T>(IEnumerable<string> lines, IVowpalWabbitPredictionFactory<T> predictionFactory)
    {
      T resultSlot = default(T);

      using (MultiExAdapter multiEx = new MultiExAdapter())
      {
        NativeMethods.CreatePredictionCallback createPrediction = () => resultSlot = predictionFactory.Create(this, multiEx[0]);

        this.ParseLinesToMultiEx(lines, multiEx);
        this.InvokeMultiLearner(NativeMethods.WorkspacePredictMulti, multiEx, createPrediction);
        GC.KeepAlive(createPrediction);
      }

      return resultSlot;
    }

    public void EndOfPass()
    {
      ApiStatus status = new ApiStatus();
      if (NativeMethods.WorkspaceNotifyEndOfPass(this.DangerousGetHandle(), status.DangerousGetHandle()) != NativeMethods.SuccessStatus)
      {
        throw new VWException(status);
      }
    }

    public void Driver()
    {
      ApiStatus status = new ApiStatus();
      if (NativeMethods.WorkspaceRunDriver(this.DangerousGetHandle(), status.DangerousGetHandle()) != NativeMethods.SuccessStatus)
      {
        throw new VWException(status);
      }
    }

    public void DriverOneThread()
    {
      ApiStatus status = new ApiStatus();
      if (NativeMethods.WorkspaceRunDriverOneThread(this.DangerousGetHandle(), status.DangerousGetHandle()) != NativeMethods.SuccessStatus)
      {
        throw new VWException(status);
      }
    }

    public VowpalWabbitExample GetOrCreateNativeExample()
    {
      VowpalWabbitExample pooledExample = this.m_examples.Remove();
      if (pooledExample == null)
      {
        pooledExample = new VowpalWabbitExample(this);
      }

      pooledExample.EmptyExampleData(this);
      pooledExample.MakeLabelDefault(this);

      return pooledExample;
    }

    private bool IsRingExample(VowpalWabbitExample example)
    {
      bool result = NativeMethods.IsRingExample(this.DangerousGetHandle(), example.DangerousGetNativeHandle());
      GC.KeepAlive(this);
      example.KeepAliveNative();

      return result;
    }

    public void ReturnExampleToPool(VowpalWabbitExample example)
    {
      Debug.Assert(!this.IsRingExample(example));

      // If the bag is full, just dispose it
      if (!this.m_examples.TryAdd(example))
      {
        example.Dispose();
      }
    }

    public VowpalWabbit Native { get => this; }
  }

}