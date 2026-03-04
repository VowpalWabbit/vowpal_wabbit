# Vowpal Wabbit

[Vowpal Wabbit](https://vowpalwabbit.org/) is an open-source machine learning library with a focus on online learning, contextual bandits, and reinforcement learning.

## Installation

```
dotnet add package VowpalWabbit
```

## Platform Support

| Runtime Package | Platform |
|---|---|
| `VowpalWabbit.runtime.win-x64` | Windows x64 |
| `VowpalWabbit.runtime.linux-x64` | Linux x64 |
| `VowpalWabbit.runtime.osx-x64` | macOS x64 |
| `VowpalWabbit.runtime.osx-arm64` | macOS Apple Silicon |

All runtime packages are pulled in automatically when you install the main `VowpalWabbit` package.

## Quick Start

### Basic Regression

```csharp
using VW;

using (var vw = new VowpalWabbit("--quiet"))
{
    vw.Learn("1 | feature1 feature2");
    vw.Learn("0 | feature3 feature4");

    var prediction = vw.Predict("| feature1 feature2");
}
```

### Contextual Bandits (CB ADF)

```csharp
using VW;

using (var vw = new VowpalWabbit("--cb_explore_adf --quiet"))
{
    var example = @"shared | user_age:25
        | action1 sport
        | action2 politics
        | action3 music";

    vw.Learn(example);
    var prediction = vw.Predict(example);
}
```

### Save and Load Models

```csharp
using (var vw = new VowpalWabbit("--quiet"))
{
    // ... train ...
    vw.SaveModel("model.vw");
}

using (var vw = new VowpalWabbit("--quiet -i model.vw"))
{
    var prediction = vw.Predict("| feature1 feature2");
}
```

## Documentation

- [C# Binding Wiki](https://github.com/VowpalWabbit/vowpal_wabbit/wiki/C%23-Binding)
- [VW Documentation](https://vowpalwabbit.org/)
- [GitHub Repository](https://github.com/VowpalWabbit/vowpal_wabbit)

## License

BSD-3-Clause
