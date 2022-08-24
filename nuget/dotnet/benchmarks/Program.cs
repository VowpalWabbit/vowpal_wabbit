// See https://aka.ms/new-console-template for more information

using BenchmarkDotNet.Running;
using BenchmarkDotNet.Configs;


public class Program
{
    static void Main(string[] args)
        => BenchmarkSwitcher.FromAssembly(typeof(Program).Assembly).Run(args);
}

public class VWBenchmarkConfig : ManualConfig
{
    public VWBenchmarkConfig()
    {
        this.WithOptions(ConfigOptions.DisableOptimizationsValidator);
    }
}

