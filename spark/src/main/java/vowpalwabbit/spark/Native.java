package vowpalwabbit.spark;

import org.scijava.nativelib.NativeLoader;

public class Native {
    static {
        try {
           NativeLoader.loadLibrary("vw_spark_jni");
        } catch (Exception e) {
            throw new RuntimeException("Unable to load native library 'vw_spark_jni'", e);
        }
    }

    public static void load() {
        // just execute the loader
    }
}