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
    [DllImport("vw.net.native.dll")]
    public static extern IntPtr CreateExample(IntPtr vw);

    [DllImport("vw.net.native.dll")]
    public static extern void DeleteExample(IntPtr example);

    [DllImport("vw.net.native.dll")]
    public static extern bool IsRingExample(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native.dll")]
    public static extern bool IsExampleNewline(IntPtr example);

    [DllImport("vw.net.native.dll")]
    public static extern IntPtr ComputeDiffDescriptionExample(IntPtr vw, IntPtr ex1, IntPtr ex2);

    [DllImport("vw.net.native.dll")]
    public static extern ulong GetExampleNumberOfFeatures(IntPtr example);

    [DllImport("vw.net.native.dll")]
    public static extern void MakeEmpty(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native.dll")]
    public static extern void MakeLabelDefault(IntPtr vw, IntPtr example);

    [DllImport("vw.net.native.dll")]
    public static extern void UpdateExampleWeight(IntPtr vw, IntPtr example);

    #region Example Enumerator
    
    [DllImport("vw.net.native.dll")]
    public static extern IntPtr CreateNamespaceEnumerator(IntPtr vw, IntPtr ex);

    [DllImport("vw.net.native.dll")]
    public static extern void DeleteNamespaceEnumerator(IntPtr enumerator);

    [DllImport("vw.net.native.dll")]
    public static extern bool NamespaceEnumeratorMoveNext(IntPtr enumerator);

    [DllImport("vw.net.native.dll")]
    public static extern void NamespaceEnumeratorReset(IntPtr enumerator);

    [DllImport("vw.net.native.dll")]
    public static extern namespace_index NamespaceEnumeratorGetNamespace(IntPtr enumerator);

    #endregion
  }
}

namespace VW
{
  using Vw.Net.Native;

  using namespace_index = Byte;

  [DebuggerDisplay("{DangerousGetHandle()}: '{VowpalWabbitString}'")]
  public sealed class VowpalWabbitExample : NativeObject<VowpalWabbitExample>, IEnumerable<VowpalWabbitNamespace>
  {
    private readonly VowpalWabbit vw;

    

    internal class NamespaceEnumerator : NativeObject<NamespaceEnumerator>, IEnumerator<VowpalWabbitNamespace>
    {
      private VowpalWabbitExample example;

      internal static New<NamespaceEnumerator> BindConstructorArguments(VowpalWabbitExample example)
      {
        return new New<NamespaceEnumerator>(() =>
        {
          IntPtr result = NativeMethods.CreateNamespaceEnumerator(example.vw.DangerousGetHandle(), example.DangerousGetHandle());
          
          GC.KeepAlive(example.vw);
          GC.KeepAlive(example);
          
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



    internal static New<VowpalWabbitExample> BindConstructorArguments(VowpalWabbitBase vw)
    {
      return new New<VowpalWabbitExample>(() =>
      {
        IntPtr result = NativeMethods.CreateExample(vw.DangerousGetHandle());
        
        GC.KeepAlive(vw);
        
        return result;
      });
    }

    internal VowpalWabbitExample(IVowpalWabbitExamplePool owner)
      : base(BindConstructorArguments(owner.Native), NativeMethods.DeleteExample)
    {
      this.vw = owner.Native;
    }

    internal VowpalWabbitExample(IVowpalWabbitExamplePool owner, IntPtr example)
      : base(example, ownsHandle: true)
    {
      this.vw = owner.Native;
    }

    [Obsolete("Use the P/Invoke DangerousGetHandle() mechanism instead.")]
    internal IntPtr m_example => this.DangerousGetHandle();

    [Obsolete("Use the public Owner property instead.")]
    internal IVowpalWabbitExamplePool m_owner => this.vw;

    [Obsolete("Use the public VowpalWabbitString property instead.")]
    internal string m_string => this.VowpalWabbitString;

    public VowpalWabbitExample(IVowpalWabbitExamplePool owner, VowpalWabbitExample example)
      : base(example.DangerousGetHandle(), ownsHandle: false)
    {
      this.vw = owner.Native;
      this.InnerExample = example;
    }

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

    public IVowpalWabbitExamplePool Owner => this.vw;

    public string VowpalWabbitString
    {
      get;
      set;
    }

    public bool IsNewLine
    {
      get
      {
        bool result = NativeMethods.IsExampleNewline(this.DangerousGetHandle());
        GC.KeepAlive(this);

        return result;
      }
    }

    public string Diff(VowpalWabbit vw, VowpalWabbitExample other, IVowpalWabbitLabelComparator comparator)
    {
      IntPtr diffResultPtr = NativeMethods.ComputeDiffDescriptionExample(vw.DangerousGetHandle(), this.DangerousGetHandle(), other.DangerousGetHandle());
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

    public void MakeEmpty(VowpalWabbit vw)
    {
      // Is there a reason why we pass in VowpalWabbit here? Why not use the VowpalWabbit instance 
      // from the owner?
      NativeMethods.MakeEmpty(vw.DangerousGetHandle(), this.DangerousGetHandle());
      GC.KeepAlive(vw);
      GC.KeepAlive(this);
    }

    public void MakeLabelDefault(VowpalWabbit vw)
    {
      NativeMethods.MakeLabelDefault(vw.DangerousGetHandle(), this.DangerousGetHandle());
      GC.KeepAlive(vw);
      GC.KeepAlive(this);
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
        ulong result = NativeMethods.GetExampleNumberOfFeatures(this.DangerousGetHandle());
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

        value.UpdateExample(this);

        // we need to update the example weight as setup_example() can be called prior to this call.
        NativeMethods.UpdateExampleWeight(this.Owner.DangerousGetNativeHandle(), this.DangerousGetHandle());
        GC.KeepAlive(this.Owner);
        GC.KeepAlive(this);
      }
    }

    internal prediction_type_t PredictionType => this.Owner.Native.GetOutputPredictionType();
  }
}