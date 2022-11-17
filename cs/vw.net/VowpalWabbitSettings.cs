using System;
using System.IO;
using System.Collections.Generic;
using System.Threading.Tasks;

using VW.Serializer;

namespace VW
{
  public enum VowpalWabbitExampleDistribution
  {
    UniformRandom = 0,
    RoundRobin = 1
  }

  public interface ITypeInspector
  {
    Schema CreateSchema(VowpalWabbitSettings settings, Type type);
  }

  public class VowpalWabbitSettings : ICloneable
  {
    public VowpalWabbitSettings()
    {
      this.Arguments = string.Empty;
      this.ExampleCountPerRun = 1000;

      this.MaxExamples = Int32.MaxValue;
      this.MaxExampleCacheSize = UInt32.MaxValue;
      this.MaxExampleQueueLengthPerInstance = UInt32.MaxValue;
      
      this.EnableExampleCaching = false;
      this.EnableStringExampleGeneration = false;
      this.EnableStringFloatCompact = false;
      this.EnableThreadSafeExamplePooling = false;
      
      this.Verbose = false;

      this.PropertyConfiguration = PropertyConfiguration.Default;
      
      // default to the statistically more safe option
      this.ExampleDistribution = VowpalWabbitExampleDistribution.UniformRandom;
    }

    public VowpalWabbitSettings(string arguments) : this()
    {
      if (arguments != null)
      {
        this.Arguments = arguments;
      }
    }

    public string Arguments { get; set; }
    public Stream ModelStream { get; set; }
    public VowpalWabbitModel Model { get; set; }
    public ParallelOptions ParallelOptions { get; set; }
    public bool EnableExampleCaching { get; set; }
    public uint MaxExampleCacheSize { get; set; }
    public uint MaxExampleQueueLengthPerInstance { get; set; }
    public uint Node { get; set; }
    public VowpalWabbit Root { get; set; }
    public VowpalWabbitExampleDistribution ExampleDistribution { get; set; }

    public int ExampleCountPerRun { get; set; }

    public bool EnableStringExampleGeneration { get; set; }
    public bool EnableStringFloatCompact { get; set; }
    public Schema Schema { get; set; }
    public Schema ActionDependentSchema { get; set; }

    public List<Type> CustomFeaturizer   { get; set; }

    public ITypeInspector TypeInspector { get; set; }

    public PropertyConfiguration PropertyConfiguration { get; set; }

    public bool EnableThreadSafeExamplePooling { get; set; }

    public int MaxExamples { get; set; }

    public bool Verbose { get; set; }

    public Action<string> TraceListener { get; set; }

    public object Clone() => this.MemberwiseClone();
  }
}