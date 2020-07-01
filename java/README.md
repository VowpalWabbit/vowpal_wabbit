# Summary
The Java layer for Vowpal Wabbit uses the [Java Native Interface](https://en.wikipedia.org/wiki/Java_Native_Interface) to communicate between Java and the native Vowpal Wabbit C code.  The philosophy of this layer is that we want to expose a simple type safe, thread safe interface so a Java programmer can use Vowpal Wabbit for either learning or scoring.  An example usage is shown below

```java
class Foo {
    static {
        // Create the model.  The model type is checked here
        VWScalarLearner learner = VWLearners.create("-f model.vw");

        // Learn some data
        learner.learn("0 | price:.23 sqft:.25 age:.05 2006");
        learner.learn("1 2 'second_house | price:.18 sqft:.15 age:.35 1976");
        learner.learn("0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924");

        // Closing finalizes the model and frees up the native memory
        learner.close();

        VWScalarLearner learner = VWLearners.create("-i model.vw -t --quiet");
        // Get a prediction.
        float prediction = learner.predict("| price:0.23 sqft:0.25 age:0.05 2006");
        learner.close();
    }
}
```

More examples can be found in the [Java tests](src/test/java/vowpalWabbit/learner).

## Important Notes

1.  Most standard Vowpal Wabbit command line options are supported when calling create.  If you find some that work from the regular command line and not within the JNI please file a bug.
2.  The type returned from the create function is checked on the C side.  This means that the type will vary depending on the arguments supplied to create.  While this doesn't give full compile time safety it does fail as early as possible at runtime.  This also means that the expected output type can be safely used and checked at compile time.
3.  There is only a small amount of memory used on the Java side including a pointer to the VW model on the C side.  Because of this the Java base interface implements `Closeable` and models MUST BE CLOSED or else you will leak memory.  The Java garbage collector will not clean up the C memory.

# Installation
The Java artifacts are periodically released to [Maven Central](https://mvnrepository.com/artifact/com.github.johnlangford/vw-jni) and can be included like any other Java dependency.  Prior to version 8.4.1 some precompiled native libraries were included in the jar.  This made usage easier for users on supported platforms but became a nightmare to manage as the number of platforms grew.  It also added problems as these precompiled libraries used specific boost versions that had to be matched.  This is no longer the case as from 8.4.1 onwards the jars are much slimmer and only contain Java code.  The following is called within the Java code

```java
final public class VWLearners {
    static {
        System.loadLibrary("vw_jni");
    }
}
```

The implication here is that platform specific library must be on your `java.library.path` with the [appropriate name](https://stackoverflow.com/questions/37203247/while-loading-jni-library-how-the-mapping-happens-with-the-actual-library-name).  Creating this file is already part of the standard build process of VW, therefore you only have to execute `make java` to create the shared library.  After executing this a file will be created under `java/target/*vw_jni*` where the `*` are platform specific.  This file then needs to be put somewhere on the `java.library.path` of the machine you wish to use the JNI library from.

It should also be noted that Vowpal Wabbit makes all attempts at compatibility between versions, but the only true way to guarantee compatibility to use the exact same git hash that was used when creating the jar.  I will try and keep this list up to date as new versions are released.

| VW Version | Git Commit Hash                          |
| ---------- | ---------------------------------------- |
| 8.4.1      | 10bd09ab06f59291e04ad7805e88fd3e693b7159 |
| 8.1.0      | 9e5831a72d5b0a124c845dcaec75879f498b355f |

# Spark Layer
To improve performance when hosting VW in Spark an additional optimized layer can be found in org.vowpalwabbit.spark.*. The actual VW/Spark integration will be available through [MMLSpark](https://github.com/Azure/mmlspark).

## Features

1. Native dependencies are included in the JAR file.
2. Features are expected to be already hashed.
3. Multi-pass support.

## Limitations

1. Only simple label is supported for now (e.g. classification/regression).
