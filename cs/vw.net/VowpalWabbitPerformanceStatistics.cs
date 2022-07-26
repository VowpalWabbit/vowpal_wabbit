using System;
using System.Runtime.InteropServices;

namespace Vw.Net.Native
{
  
  [StructLayout(LayoutKind.Sequential)]
  public struct performance_statistics_t
  {
    public UIntPtr total_features;
    public UIntPtr examples_per_pass;
    public double weighted_examples;
    public double weighted_labels;
    public double average_loss;
    public float best_constant;
    public float best_constant_loss;
  }

  internal static partial class NativeMethods
  {

    [DllImport("vw.net.native")]
    public static extern void WorkspaceGetPerformanceStatistics(IntPtr workspace, ref performance_statistics_t statistics);
  }
}

namespace VW
{
  using Vw.Net;
  using Vw.Net.Native;

  public class VowpalWabbitPerformanceStatistics
  {
    private performance_statistics_t performance_statistics;

    public VowpalWabbitPerformanceStatistics(VowpalWabbitBase workspace)
    {
      this.performance_statistics = new performance_statistics_t();
      NativeMethods.WorkspaceGetPerformanceStatistics(workspace.DangerousGetHandle(), ref this.performance_statistics);
      GC.KeepAlive(workspace);
    }

    public ulong TotalNumberOfFeatures
    {
      get => performance_statistics.total_features.ToUInt64();
      set => performance_statistics.total_features = new UIntPtr(value);
    }

    public double WeightedExampleSum
    {
      get => performance_statistics.weighted_examples;
      set => performance_statistics.weighted_examples = value;
    }

    public ulong NumberOfExamplesPerPass
    {
      get => performance_statistics.examples_per_pass.ToUInt64();
      set => performance_statistics.examples_per_pass = new UIntPtr(value);
    }

    public double WeightedLabelSum
    {
      get => performance_statistics.weighted_labels;
      set => performance_statistics.weighted_labels = value;
    }

    public double AverageLoss
    {
      get => performance_statistics.average_loss;
      set => performance_statistics.average_loss = value;
    }

    public double BestConstant
    {
      get => performance_statistics.best_constant;
      set => performance_statistics.best_constant = (float)value;
    }

    public double BestConstantLoss
    {
      get => performance_statistics.best_constant_loss;
      set => performance_statistics.best_constant_loss = (float)value;
    }
  }
}