# Vowpal Wabbit Java Bindings

The Java layer for Vowpal Wabbit uses the [Java Native Interface](https://en.wikipedia.org/wiki/Java_Native_Interface) to communicate between Java and the native Vowpal Wabbit C code. The philosophy of this layer is to expose a simple type-safe, thread-safe interface so Java programmers can use Vowpal Wabbit for learning and scoring.

## Quick Start

```java
import vowpalWabbit.learner.VWLearners;
import vowpalWabbit.learner.VWScalarLearner;

public class Example {
    public static void main(String[] args) {
        // Create the model (type is checked at runtime)
        try (VWScalarLearner learner = VWLearners.create("-f model.vw --quiet")) {
            // Learn some data
            learner.learn("0 | price:.23 sqft:.25 age:.05 2006");
            learner.learn("1 2 'second_house | price:.18 sqft:.15 age:.35 1976");
            learner.learn("0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924");
        } // Model is automatically closed and saved

        // Load model for prediction
        try (VWScalarLearner scorer = VWLearners.create("-i model.vw -t --quiet")) {
            float prediction = scorer.predict("| price:0.23 sqft:0.25 age:0.05 2006");
            System.out.println("Prediction: " + prediction);
        }
    }
}
```

More examples can be found in the [Java tests](src/test/java/vowpalWabbit/learner).

## Important Notes

1. **Type Safety**: The type returned from `VWLearners.create()` is checked on the C side based on the VW arguments. While this doesn't provide full compile-time safety, it fails as early as possible at runtime.

2. **Memory Management**: The Java garbage collector will NOT clean up native C memory. Models MUST be closed explicitly by calling `close()` or using try-with-resources. Failure to close models will leak memory.

3. **Thread Safety**: Each learner instance uses a `ReentrantLock` for learn/predict operations. Multi-pass operations require a global lock.

4. **Command Line Options**: Most standard VW command line options are supported. If you find options that work from the command line but not via JNI, please file a bug.

## Installation

### Maven Dependency

The Java artifacts are released to [Maven Central](https://mvnrepository.com/artifact/com.github.vowpalwabbit/vw-jni):

```xml
<dependency>
    <groupId>com.github.vowpalwabbit</groupId>
    <artifactId>vw-jni</artifactId>
    <version>9.10.0</version>
</dependency>
```

### Native Library Requirements

The JNI layer requires a platform-specific native library. The library loader attempts to:

1. Load from `java.library.path` using `System.loadLibrary("vw_jni")`
2. If that fails, extract from the JAR file (if bundled)

#### Supported Platforms

| Platform | Directory | Library Name |
|----------|-----------|--------------|
| Linux x64 | `natives/linux_64/` | `libvw_jni.so` |
| Linux ARM64 | `natives/linux_arm64/` | `libvw_jni.so` |
| macOS x64 | `natives/macos_x64/` | `libvw_jni.dylib` |
| macOS ARM64 | `natives/macos_arm64/` | `libvw_jni.dylib` |
| Windows x64 | `natives/windows_64/` | `vw_jni.dll` |

#### Building the Native Library

```bash
# From the vowpal_wabbit root directory
mkdir build && cd build
cmake .. -DBUILD_JAVA=ON
make vw_jni
```

The native library will be created at `java/target/bin/natives/<platform>/`.

#### Installing the Native Library

Copy the library to a location on your `java.library.path`:

- **Linux**: `/usr/lib` or `/usr/local/lib`
- **macOS**: `/Library/Java/Extensions`
- **Windows**: Add to `PATH` or application directory

Alternatively, specify the path at runtime:

```bash
java -Djava.library.path=/path/to/natives/linux_64 -jar myapp.jar
```

## API Overview

### Learner Types

| Class | Use Case | Return Type |
|-------|----------|-------------|
| `VWScalarLearner` | Regression, binary classification | `float` |
| `VWMulticlassLearner` | Multiclass classification | `int` |
| `VWMultilabelsLearner` | Multilabel classification | `int[]` |
| `VWActionScoresLearner` | Contextual bandits | `ActionScores` |
| `VWActionProbsLearner` | CB with exploration | `ActionProbs` |
| `VWCCBLearner` | Conditional contextual bandits | `DecisionScores` |
| `VWProbLearner` | Continuous actions | `PDF` |

### Core Methods

```java
// Learning
learner.learn("label | features");
learner.learn("label | features", "tag");

// Prediction
T prediction = learner.predict("| features");

// Model persistence
learner.saveModel("model.vw");
```

## Known Limitations

### File-based Training (`-d` option)

The Java API is designed for programmatic example feeding. The `-d` (data file) option is not supported because the JNI layer initializes VW but does not run the full training driver loop.

**Workaround**: Read the file in Java and feed examples programmatically:

```java
try (VWScalarLearner learner = VWLearners.create("--quiet");
     BufferedReader reader = new BufferedReader(new FileReader("data.vw"))) {
    String line;
    while ((line = reader.readLine()) != null) {
        learner.learn(line);
    }
}
```

For large-scale file-based training, use the VW command-line tool directly.

### Other Limitations

- Some advanced options may not be fully supported
- Multi-pass learning requires careful memory management
- The Spark layer only supports simple labels (classification/regression)

## Version Compatibility

| VW Version | Git Commit Hash |
|------------|-----------------|
| 9.10.0 | (current) |
| 8.4.1 | 10bd09ab06f59291e04ad7805e88fd3e693b7159 |
| 8.1.0 | 9e5831a72d5b0a124c845dcaec75879f498b355f |

## Spark Integration

For improved performance when hosting VW in Spark, an optimized layer is available in `org.vowpalwabbit.spark.*`. The full VW/Spark integration is available through [SynapseML](https://github.com/microsoft/SynapseML) (formerly MMLSpark).

### Spark Layer Features

1. Native dependencies bundled in JAR
2. Pre-hashed feature support
3. Multi-pass learning support

### Spark Layer Limitations

1. Only simple labels supported (classification/regression)

## Troubleshooting

### UnsatisfiedLinkError

```
java.lang.UnsatisfiedLinkError: no vw_jni in java.library.path
```

**Solution**: Ensure the native library is on your `java.library.path` or bundled in the JAR.

### Platform Not Supported

```
UnsupportedOperationException: No native library found for platform: <platform>
```

**Solution**: Build the native library for your platform or use a supported platform.

### Memory Leaks

If you see increasing memory usage, ensure all learners are closed:

```java
// BAD - leaks memory
VWScalarLearner learner = VWLearners.create("--quiet");
learner.learn("...");
// forgot to close!

// GOOD - properly cleaned up
try (VWScalarLearner learner = VWLearners.create("--quiet")) {
    learner.learn("...");
}
```

## Contributing

See the main [VowpalWabbit contributing guide](https://github.com/VowpalWabbit/vowpal_wabbit/blob/master/CONTRIBUTING.md).
