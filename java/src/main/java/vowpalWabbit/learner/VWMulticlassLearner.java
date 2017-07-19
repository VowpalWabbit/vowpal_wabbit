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
     * Get raw prediction output.
     *
     * @param example a single vw example string
     * @return Raw prediction
     */
    public float[] rawPredict(final String example) {
        lock.lock();
        try {
            if (isOpen()) {
                return rawPredict(example, false, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        } finally {
            lock.unlock();
        }
    }
}
