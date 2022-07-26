using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Runtime.InteropServices;

using VW.Labels;

namespace Vw.Net.Native
{
  using namespace_index = Byte;
  using feature_index = UInt64;

  internal static partial class NativeMethods
  {
    [DllImport("vw.net.native")]
    public static extern IntPtr CreateExample(IntPtr vw);

    [DllImport("vw.net.native")]
    public static extern void DeleteExample(IntPtr example);

    [DllImport("vw.net.native")]
    public static extern bool IsRingExample(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern bool IsExampleNewline(IntPtr example);

    [DllImport("vw.net.native")]
    public static extern IntPtr ComputeDiffDescriptionExample(IntPtr vw, IntPtr ex1, IntPtr ex2);

    [DllImport("vw.net.native")]
    public static extern ulong GetExampleNumberOfFeatures(IntPtr example);

    [DllImport("vw.net.native")]
    public static extern void EmptyExampleData(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern void MakeIntoNewlineExample(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern void MakeLabelDefault(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native")]
    public static extern void UpdateExampleWeight(IntPtr vw, IntPtr example);

    #region Example Enumerator
    
    [DllImport("vw.net.native")]
    public static extern IntPtr CreateNamespaceEnumerator(IntPtr vw, IntPtr ex);

    [DllImport("vw.net.native")]
    public static extern void DeleteNamespaceEnumerator(IntPtr enumerator);

    [DllImport("vw.net.native")]
    public static extern bool NamespaceEnumeratorMoveNext(IntPtr enumerator);

    [DllImport("vw.net.native")]
    public static extern void NamespaceEnumeratorReset(IntPtr enumerator);

    [DllImport("vw.net.native")]
    public static extern namespace_index NamespaceEnumeratorGetNamespace(IntPtr enumerator);

    #endregion
  }
}

namespace VW
{
    using System.Runtime.ConstrainedExecution;
    using Vw.Net.Native;

  using namespace_index = Byte;
  using ctor = Func<IntPtr>;
  using dtor = Action<IntPtr>;

  [DebuggerDisplay("{DangerousGetNativeHandle()}: '{VowpalWabbitString}'")]
  public sealed class VowpalWabbitExample : IDisposable, IEnumerable<VowpalWabbitNamespace>
  {
    private readonly IVowpalWabbitExamplePool owner;
    private NativeHandle exampleHandle;
    private readonly VowpalWabbit vw;

    internal class NamespaceEnumerator : NativeObject<NamespaceEnumerator>, IEnumerator<VowpalWabbitNamespace>
    {
      private VowpalWabbitExample example;

      internal static New<NamespaceEnumerator> BindConstructorArguments(VowpalWabbitExample example)
      {
        return new New<NamespaceEnumerator>(() =>
        {
          IntPtr result = NativeMethods.CreateNamespaceEnumerator(example.vw.DangerousGetHandle(), example.exampleHandle.DangerousGetHandle());
          
          GC.KeepAlive(example.vw);
          GC.KeepAlive(example.exampleHandle);
          
          return result;
        });
      }

      public NamespaceEnumerator(VowpalWabbitExample example)
        : base(BindConstructorArguments(example), NativeMethods.DeleteNamespaceEnumerator)
      {
        this.example = example;
      }

      public VowpalWabbitNamespace Current
      {
        get
        {
          namespace_index index = NativeMethods.NamespaceEnumeratorGetNamespace(this.DangerousGetHandle());
          GC.KeepAlive(this);

          return new VowpalWabbitNamespace(this.example, index);
        }
      }

      public bool MoveNext()
      {
        bool result = NativeMethods.NamespaceEnumeratorMoveNext(this.DangerousGetHandle());
        GC.KeepAlive(this);

        return result;
      }

      public void Reset()
      {
        NativeMethods.NamespaceEnumeratorReset(this.DangerousGetHandle());
        GC.KeepAlive(this);
      }

      object IEnumerator.Current => this.Current;
    }

    private static NativeHandle AllocateNativeExample(VowpalWabbitBase vw)
    {
      ctor createExample = () => NativeMethods.CreateExample(vw.DangerousGetHandle());
      NativeHandle result = NativeHandle.MakeNative(createExample, NativeMethods.DeleteExample);
      GC.KeepAlive(vw);

      return result;
    }

    internal VowpalWabbitExample(IVowpalWabbitExamplePool owner)
    {
      this.owner = owner;
      this.vw = owner.Native;

      this.exampleHandle = AllocateNativeExample(this.vw);
    }

    public VowpalWabbitExample(IVowpalWabbitExamplePool owner, VowpalWabbitExample example)
    {
      this.owner = owner;
      this.vw = owner.Native;

      this.InnerExample = example;
      this.exampleHandle = example.exampleHandle.DangerousMakeNonOwningCopy();
    }

    public void Dispose()
    {
      if (this.owner != null)
      {
        this.owner.ReturnExampleToPool(this);
      }
    }

    internal void KeepAliveNative()
    {
      GC.KeepAlive(this.exampleHandle);
    }

    [Obsolete("Accessing the internal native example handle is deprecated.")]
    internal IntPtr m_example => this.exampleHandle.DangerousGetHandle();

    [Obsolete("Use the public Owner property instead.")]
    internal IVowpalWabbitExamplePool m_owner => this.owner;

    [Obsolete("Use the public VowpalWabbitString property instead.")]
    internal string m_string => this.VowpalWabbitString;

    public T GetPrediction<T>(VowpalWabbit vw, IVowpalWabbitPredictionFactory<T> factory)
    {
      if (vw == null)
      {
        throw new ArgumentNullException(nameof(vw));
      }

      return factory.Create(vw, this);
    }

    public VowpalWabbitExample InnerExample
    {
      get;
      private set;
    }

    public IVowpalWabbitExamplePool Owner => this.owner;

    public string VowpalWabbitString
    {
      get;
      set;
    }

    public IntPtr DangerousGetNativeHandle()
    {
      return this.exampleHandle.DangerousGetHandle();
    }

    public bool IsNewLine
    {
      get
      {
        bool result = NativeMethods.IsExampleNewline(this.exampleHandle.DangerousGetHandle());
        GC.KeepAlive(this.exampleHandle);

        return result;
      }
    }

    public string Diff(VowpalWabbit vw, VowpalWabbitExample other, IVowpalWabbitLabelComparator comparator)
    {
      IntPtr diffResultPtr = NativeMethods.ComputeDiffDescriptionExample(vw.DangerousGetHandle(), this.exampleHandle.DangerousGetHandle(), other.exampleHandle.DangerousGetHandle());
      GC.KeepAlive(vw);
      GC.KeepAlive(this.exampleHandle);
      GC.KeepAlive(other.exampleHandle);

      if (diffResultPtr != IntPtr.Zero)
      {
        // Marshall the string back using the marshalling function.
        string diffResult = NativeMethods.StringMarshallingFunc(diffResultPtr);
        NativeMethods.FreeDupString(diffResultPtr);

        return diffResult;
      }

      if (comparator != null)
      {
        return comparator.Diff(this, other);
      }

      return null;
    }

    internal void EmptyExampleData(VowpalWabbit vw)
    {
      // The equivalent method is called empty_example() in the .NET Framework bindings, but this leads
      // to potential confusion with MakeEmpty().
      NativeMethods.EmptyExampleData(vw.DangerousGetHandle(), this.exampleHandle.DangerousGetHandle());
      GC.KeepAlive(vw);
      GC.KeepAlive(this.exampleHandle);
    }

    public void MakeEmpty(VowpalWabbit vw)
    {
      // Is there a reason why we pass in VowpalWabbit here? Why not use the VowpalWabbit instance 
      // from the owner?
      NativeMethods.MakeIntoNewlineExample(vw.DangerousGetHandle(), this.exampleHandle.DangerousGetHandle());
      GC.KeepAlive(vw);
      GC.KeepAlive(this.exampleHandle);
    }

    public void MakeLabelDefault(VowpalWabbit vw)
    {
      NativeMethods.MakeLabelDefault(vw.DangerousGetHandle(), this.exampleHandle.DangerousGetHandle());
      GC.KeepAlive(vw);
      GC.KeepAlive(this.exampleHandle);
    }

    public IEnumerator<VowpalWabbitNamespace> GetEnumerator()
    {
      return new NamespaceEnumerator(this);
    }

    IEnumerator IEnumerable.GetEnumerator()
    {
      return this.GetEnumerator();
    }

    // TODO: In the native layer, this is given as a size_t, but that is architecture-dependent.
    // The correct type to use for that is UIntPtr, but we expect to do math on it, so we will use
    // UInt64 for now.
    public ulong NumberOfFeatures
    {
      get
      {
        ulong result = NativeMethods.GetExampleNumberOfFeatures(this.exampleHandle.DangerousGetHandle());
        GC.KeepAlive(this);

        return result;
      }
    }

    public ILabel Label
    {
      get 
      {
        ILabel label = null;
        NativeMethods.label_type_t labelType = NativeMethods.WorkspaceGetLabelType(vw.DangerousGetHandle());
        GC.KeepAlive(vw);

        ContextualBanditLabel cbLabel = null;
        switch (labelType)
        {
          case NativeMethods.label_type_t.simple:
          case NativeMethods.label_type_t.cb_eval:
          case NativeMethods.label_type_t.cs:
            label = new SimpleLabel();
            break;
          case NativeMethods.label_type_t.cb:
            label = cbLabel = new ContextualBanditLabel();
            break;
          default:
            break;
          // TODO: These label types are missing from C#/CLI implementation.
          // case NativeMethods.label_type_t.multilabel:
          // case NativeMethods.label_type_t.multiclass:
          // case NativeMethods.label_type_t.ccb:
          // case NativeMethods.label_type_t.slates:
          // case NativeMethods.label_type_t.continuous: 
          // case NativeMethods.label_type_t.nolabel: // What is this for?
        }

        label.ReadFromExample(this);
        if (cbLabel != null && cbLabel.IsShared)
        {
          return SharedLabel.Instance;
        }

        return label;
      }
      set
      {
        if (value == null)
        {
          //TODO: Is the correct thing to do here to clear the label?
          return; 
        }

        value.UpdateExample(this.vw, this);

        // we need to update the example weight as setup_example() can be called prior to this call.
        NativeMethods.UpdateExampleWeight(this.Owner.DangerousGetNativeHandle(), this.exampleHandle.DangerousGetHandle());
        GC.KeepAlive(this.Owner);
        GC.KeepAlive(this);
      }
    }

    internal prediction_type_t PredictionType => this.Owner.Native.GetOutputPredictionType();
  }
}