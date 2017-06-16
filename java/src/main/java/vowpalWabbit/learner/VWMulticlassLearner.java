package vowpalWabbit.learner;

/**
 * @author deak
 */
final public class VWMulticlassLearner extends VWIntLearner {
    VWMulticlassLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native int predict(String example, boolean learn, long nativePointer);

    @Override
    protected native int predictMultiline(String[] example, boolean learn, long nativePointer);

    protected native float[] rawPredict(String example, boolean learn, long nativePointer);

    /**
     * Runs prediction on <code>example</code> and returns the prediction output.
     *
     * @param example a single vw example string
     * @return A prediction
     */
    public float[] rawPredict(final String example) {
        return rawLearnOrPredict(example, false);
    }

    /**
     * <code>learnOrPredict</code> allows the ability to return an unboxed prediction.  This will reduce the overhead
     * of this function call.
     * @param example an example
     * @param learn whether to call the learn or predict VW functions.
     * @return an <em>UNBOXED</em> prediction.
     */
    private float[] rawLearnOrPredict(final String example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return rawPredict(example, learn, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }
}
