using BenchmarkDotNet.Running;
using BenchmarkDotNet.Configs;
using BenchmarkDotNet.Exporters;


public class Program
{
    static void Main(string[] args)
        => BenchmarkSwitcher.FromAssembly(typeof(Program).Assembly).Run(args);
}

public class VWBenchmarkConfig : ManualConfig
{
    public VWBenchmarkConfig()
    {
        AddExporter(PlainExporter.Default);
        AddExporter(RPlotExporter.Default);
    }
}

