using BenchmarkDotNet.Running;
using BenchmarkDotNet.Configs;
using BenchmarkDotNet.Exporters;
using BenchmarkDotNet.Exporters.Json;
using BenchmarkDotNet.Jobs;
using BenchmarkDotNet.Environments;

public class Program
{
    static void Main(string[] args)
        => BenchmarkSwitcher.FromAssembly(typeof(Program).Assembly).Run(args);
}

public class VWBenchmarkConfig : ManualConfig
{
    public VWBenchmarkConfig()
    {
        AddJob(Job.Default.AsBaseline().WithId(".NET 8.0").WithRuntime(CoreRuntime.Core80));
        AddJob(Job.Default.WithId(".NET Framework 4.8").WithRuntime(ClrRuntime.Net48));
        AddExporter(PlainExporter.Default);
        AddExporter(RPlotExporter.Default);
        AddExporter(JsonExporter.FullCompressed);
    }
}
