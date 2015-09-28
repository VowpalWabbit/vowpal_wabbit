package vw;

/**
 * A JNI layer for submitting examples to VW and getting predictions back.  It should be noted
 * that at this time VW is NOT thread safe, and therefore neither is the JNI layer.  It should be noted
 * that this was originally written with a bulk interface that was later removed because of benchmarking
 * data found <a href="https://microbenchmarks.appspot.com/runs/817d246a-5f90-478a-bc27-d5912d2ff874#r:scenario.benchmarkSpec.methodName,scenario.benchmarkSpec.parameters.loss,scenario.benchmarkSpec.parameters.mutabilityPolicy,scenario.benchmarkSpec.parameters.nExamples">here</a>.
 * Please note that close MUST be called in order to free up the memory on the C side.
 */
public final class VW extends VWGenericBase<Float> {

    /**
     * This main method only exists to test the library implementation.  To test it just run
     * java -cp target/vw-jni-*-SNAPSHOT.jar vw.VW
     * @param args No args needed.
     */
    public static void main(String[] args) {
        new VW("").close();
        new VW("--quiet").close();
    }

    public VW(String command) {
        super(command);
    }

    public Float predict(final String example, final boolean learn, final long nativePointer) {
        return predict_specialized(example, learn, nativePointer);
    }

    private float learnOrPredict(final String example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return predict_specialized(example, learn, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }

    /**
     * Runs prediction on <code>example</code> and returns the prediction output.
     *
     * @param example a single vw example string
     * @return A prediction
     */
    public float predictSpecialized(final String example) {
        return learnOrPredict(example, false);
    }

    /**
     * Runs learning on <code>example</code> and returns the prediction output.
     *
     * @param example a single vw example string
     * @return A prediction
     */
    public float learnSpecialized(final String example) {
        return learnOrPredict(example, true);
    }

    public static native String version();
    private static native float predict_specialized(String example, boolean learn, long nativePointer);
}
