using System;
using System.Runtime.InteropServices;

namespace VW.Labels.Interop
{
  using uint32_t = System.UInt32;

  [StructLayout(LayoutKind.Explicit)]
  internal struct SimpleLabelReductionFeatures
  {
    [FieldOffset(0)]
    float weight;

    [FieldOffset(4)]
    float initial;
  }

  [StructLayout(LayoutKind.Explicit)]
  internal struct MulticlassLabelData
  {
    [FieldOffset(0)]
    public uint32_t label;

    [FieldOffset(4)]
    public float weight;
  }

  [StructLayout(LayoutKind.Explicit)]
  internal struct CbClassData
  {
    [FieldOffset(0)]
    public float cost;

    [FieldOffset(4)]
    public uint32_t action;

    [FieldOffset(8)]
    public float probability;

    [FieldOffset(12)]
    public float partial_prediction;
  }
}