using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Globalization;
using System.Text;

using Newtonsoft.Json;

using VW.Labels.Interop;

namespace Vw.Net.Native
{
  internal static partial class NativeMethods
  {
    // Similar to prediction_type_t, but it was never exposed as part of the .NET API
    public enum label_type_t
    {
        simple,
        cb,       // contextual-bandit
        cb_eval,  // contextual-bandit evaluation
        cs,       // cost-sensitive
        multilabel,
        multiclass,
        ccb,  // conditional contextual-bandit
        slates,
        nolabel,
        continuous  // continuous actions
    }

    [DllImport("vw.net.native")]
    public static extern IntPtr CbLabelReadFromExampleDangerous(IntPtr example);

    [DllImport("vw.net.native")]
    public static extern void CbLabelUpdateExample(IntPtr example, ref CbClassData labelData);

    [DllImport("vw.net.native")]
    public static extern label_type_t WorkspaceGetLabelType(IntPtr workspace);
  }
}

namespace VW.Labels
{
  using Vw.Net.Native;

  public interface ILabel
  {
    void UpdateExample(VowpalWabbit vw, VowpalWabbitExample ex);
    void ReadFromExample(VowpalWabbitExample ex);
  }

  public sealed class ContextualBanditLabel : ILabel
  {
    private CbClassData labelData = new CbClassData();

    public ContextualBanditLabel()
    {
    }

    public ContextualBanditLabel(uint action, float cost, float probability)
    {
      this.labelData.action = action;
      this.labelData.cost = cost;
      this.labelData.probability = probability;
    }

    [JsonProperty]
    public uint Action 
    {
      get => this.labelData.action;
      set => this.labelData.action = value;
    }

    [JsonProperty]
    public float Probability
    {
      get => this.labelData.probability;
      set => this.labelData.probability = value;
    }

    [JsonProperty]
    public float Cost
    {
      get => this.labelData.cost;
      set => this.labelData.cost = value;
    }

    [JsonProperty]
    public float Prediction
    {
      get => this.labelData.partial_prediction;
      set => this.labelData.partial_prediction = value;
    }

    [JsonIgnore]
    public bool IsShared => SharedLabel.IsShared(this);

    void ILabel.UpdateExample(VowpalWabbit vw, VowpalWabbitExample example)
    {
      VowpalWabbit workspace = example.Owner.Native;

      NativeMethods.CbLabelUpdateExample(example.DangerousGetNativeHandle(), ref this.labelData);

      example.KeepAliveNative();
      GC.KeepAlive(workspace);
    }

    private unsafe CbClassData ReadLabelDataFromExample(VowpalWabbitExample example)
    {
      IntPtr labelDataPtr = NativeMethods.CbLabelReadFromExampleDangerous(example.DangerousGetNativeHandle());
      CbClassData labelData = default(CbClassData);
      if (labelDataPtr != IntPtr.Zero)
      {
        labelData = Marshal.PtrToStructure<CbClassData>(labelDataPtr);
      }

      // BUGBUG!
      // This needs to happen after the Marshal.PtrToStructure call, because the pointer
      // references data owned by the example, so we need to keep the example object valid
      // until after we copy out the label data.
      example.KeepAliveNative();

      return labelData;
    }

    void ILabel.ReadFromExample(VowpalWabbitExample example)
    {
      this.labelData = ReadLabelDataFromExample(example);
    }

    public override string ToString()
    {
      return FormattableString.Invariant($"{this.Action}:{this.Cost}:{this.Probability}");
    }
  }

  public sealed class SharedLabel : ILabel
  {
    [DllImport("vw.net.native")]
    private static extern float SharedLabelGetCostConstant();

    private static readonly string SharedLabelString = "shared";

    private static readonly float SharedLabelCost = SharedLabelGetCostConstant();
    private const float SharedLabelProbability = -1f;

    private readonly ContextualBanditLabel labelTemplate;

    public static readonly SharedLabel Instance = new SharedLabel();

    internal static bool IsShared(ILabel label)
    {
      ContextualBanditLabel cbLabel = label as ContextualBanditLabel;
      if (cbLabel != null)
      {
        return cbLabel.Probability == SharedLabelProbability && cbLabel.Cost == SharedLabelCost;
      }
      else 
      {
        return label is SharedLabel;
      }
    }

    public SharedLabel()
    {
      this.labelTemplate = new ContextualBanditLabel();
      this.labelTemplate.Action = (uint)NativeMethods.VwUniformHash(SharedLabelString, 0);
      this.labelTemplate.Probability = -1f;
      this.labelTemplate.Cost = SharedLabelCost;
      this.labelTemplate.Prediction = 0f;
    }

    public override string ToString()
    {
      return SharedLabelString;
    }

    void ILabel.UpdateExample(VowpalWabbit vw, VowpalWabbitExample ex)
    {
      ((ILabel)this.labelTemplate).UpdateExample(vw, ex);
    }

    void ILabel.ReadFromExample(VowpalWabbitExample ex)
    {
    }
  }

  public sealed class SimpleLabel : ILabel
  {
    [DllImport("vw.net.native")]
    private static extern float SimpleLabelReadFromExample(IntPtr example, out float weight, out float initial);

    [DllImport("vw.net.native")]
    private static extern void SimpleLabelUpdateExample(IntPtr workspace, IntPtr example, float label, IntPtr maybe_weight, IntPtr maybe_initial);

    [JsonProperty]
    public float Label { get; set; }

    [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
    public float? Weight { get; set; }

    [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
    public float? Initial { get; set; }

    void ILabel.UpdateExample(VowpalWabbit vw, VowpalWabbitExample example)
    {
      float weight, initial;
      IntPtr maybe_weight = IntPtr.Zero;
      IntPtr maybe_initial = IntPtr.Zero;

      unsafe
      {
        if (this.Weight.HasValue)
        {
          weight = this.Weight.Value;
          maybe_weight = new IntPtr(&weight); // This is safe because weight is fixed on the stack
        }

        if (this.Initial.HasValue)
        {
          initial = this.Initial.Value;
          maybe_initial = new IntPtr(&initial); // This is safe because weight is fixed on the stack
        }

        SimpleLabelUpdateExample(vw.DangerousGetHandle(), example.DangerousGetNativeHandle(), this.Label, maybe_weight, maybe_initial);

        example.KeepAliveNative();
        GC.KeepAlive(vw);
      }
    }

    void ILabel.ReadFromExample(VowpalWabbitExample example)
    {
      float weight, initial;

      this.Label = SimpleLabelReadFromExample(example.DangerousGetNativeHandle(), out weight, out initial);
      example.KeepAliveNative();

      this.Weight = weight;
      this.Initial = initial;
    }

    public override string ToString()
    {
      StringBuilder sb = new StringBuilder();
      sb.Append(this.Label.ToString(CultureInfo.InvariantCulture));

      if (this.Weight.HasValue)
      {
        sb.Append(' ');
        sb.Append(this.Weight.Value.ToString(CultureInfo.InvariantCulture));

        if (this.Initial.HasValue)
        {
          sb.Append(' ');
          sb.Append(this.Initial.Value.ToString(CultureInfo.InvariantCulture));
        }
      }

      return sb.ToString();
    }
  }

  public sealed class MulticlassLabel : ILabel
  {
    //[JsonProperty] // this was not present in the .NET Framework bindings
    public class Label
    {
      public int Class
      {
        get;
        set;
      }

      [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
      public float? Weight
      {
        get;
        set;
      }
    }

    [JsonProperty]
    public List<Label> Classes
    {
      get;
      set;
    }

    void ILabel.UpdateExample(VowpalWabbit vw, VowpalWabbitExample example)
    {
      throw new NotImplementedException("to be done...");
    }

    void ILabel.ReadFromExample(VowpalWabbitExample example)
    {
      throw new NotImplementedException("to be done...");
    }

    public override string ToString()
    {
      StringBuilder sb = new StringBuilder();

      foreach (var label in this.Classes)
      {
        sb.Append(' ');
        sb.Append(label.Class.ToString(CultureInfo.InvariantCulture));

        if (label.Weight.HasValue)
        {
          sb.Append(' ');
          sb.Append(label.Weight.Value.ToString(CultureInfo.InvariantCulture));
        }
      }

      // This is a slightly awkward way to do this but it is slightly more
      // performant than generating the string first, then copying it to trim
      // the leading/trailing whitespace.
      if (sb.Length > 0)
        sb.Remove(0, 1);

      return sb.ToString();
    }
  }

  /// <summary>
  /// Label for continuous action (CATS) problems.
  /// Used with --cats option for continuous action tree search.
  /// </summary>
  public sealed class ContinuousActionLabel : ILabel
  {
    [DllImport("vw.net.native")]
    private extern static int StringLabelParseAndUpdateExample(IntPtr workspace, IntPtr example, IntPtr label, IntPtr label_len, IntPtr api_status);

    public ContinuousActionLabel()
    {
    }

    public ContinuousActionLabel(float action, float cost, float pdfValue)
    {
      this.Action = action;
      this.Cost = cost;
      this.PdfValue = pdfValue;
    }

    /// <summary>
    /// The continuous action value.
    /// </summary>
    [JsonProperty("action")]
    public float Action { get; set; }

    /// <summary>
    /// The cost of this action.
    /// </summary>
    [JsonProperty("cost")]
    public float Cost { get; set; }

    /// <summary>
    /// The PDF density of the chosen location, specifies the probability
    /// the data collection policy chose this action.
    /// </summary>
    [JsonProperty("pdf_value")]
    public float PdfValue { get; set; }

    unsafe private static void ParseLabelAndUpdateExample(VowpalWabbitExample example, string label, Vw.Net.ApiStatus status = null)
    {
      VowpalWabbit workspace = example.Owner.Native;

      fixed (byte* labelBytes = NativeMethods.StringEncoding.GetBytes(label))
      {
        IntPtr labelPtr = new IntPtr(labelBytes);
        IntPtr labelLen = new IntPtr(label.Length);

        if (StringLabelParseAndUpdateExample(workspace.DangerousGetHandle(), example.DangerousGetNativeHandle(), labelPtr, labelLen, status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
          throw new Vw.Net.VWException(status);

        example.KeepAliveNative();
        GC.KeepAlive(workspace);
      }
    }

    void ILabel.UpdateExample(VowpalWabbit vw, VowpalWabbitExample example)
    {
      ParseLabelAndUpdateExample(example, this.ToString());
    }

    void ILabel.ReadFromExample(VowpalWabbitExample example)
    {
      throw new NotImplementedException("Reading continuous action labels from examples is not yet implemented.");
    }

    public override string ToString()
    {
      // Format: ca action:cost:pdf_value
      return string.Format(CultureInfo.InvariantCulture, "ca {0}:{1}:{2}", this.Action, this.Cost, this.PdfValue);
    }
  }

  public sealed class StringLabel : ILabel
  {
    [DllImport("vw.net.native")]
    private extern static int StringLabelParseAndUpdateExample(IntPtr workspace, IntPtr example, IntPtr label, IntPtr label_len, IntPtr api_status);

    public StringLabel() 
    { }

    public StringLabel(string label)
    {
      this.Label = label;
    }

    public string Label 
    { 
      get; 
      set; 
    }

    unsafe private static void ParseLabelAndUpdateExample(VowpalWabbitExample example, string label, Vw.Net.ApiStatus status = null)
    {
      VowpalWabbit workspace = example.Owner.Native;

      fixed (byte* labelBytes = NativeMethods.StringEncoding.GetBytes(label))
      {
        IntPtr labelPtr = new IntPtr(labelBytes);
        IntPtr labelLen = new IntPtr(label.Length);
        IntPtr apiStatusPtr = IntPtr.Zero;

        if (StringLabelParseAndUpdateExample(workspace.DangerousGetHandle(), example.DangerousGetNativeHandle(), labelPtr, labelLen, status.ToNativeHandleOrNullptrDangerous()) != NativeMethods.SuccessStatus)
          throw new Vw.Net.VWException(status);

        example.KeepAliveNative();
        GC.KeepAlive(workspace);
      }
    }

    void ILabel.UpdateExample(VowpalWabbit vw, VowpalWabbitExample example)
    {
      ParseLabelAndUpdateExample(example, this.Label);
    }

    void ILabel.ReadFromExample(VowpalWabbitExample example)
    {
      throw new NotImplementedException("to be done...");
    }

    public override string ToString()
    {
      return this.Label;
    }
  }
}