package vowpalWabbit.learner;

/**
 * @author deak
 */
abstract class VWFloatLearner extends VWBase implements VWLearner {
    VWFloatLearner(final long nativePointer) {
        super(nativePointer);
    }

    /**
     * <code>learnOrPredict</code> allows the ability to return an unboxed prediction.  This will reduce the overhead
     * of this function call.
     * @param example an example
     * @param learn whether to call the learn or predict VW functions.
     * @return an <em>UNBOXED</em> prediction.
     */
    private float learnOrPredict(final String example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return predict(example, learn, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }

    /**
     * <code>learnOrPredict</code> allows the ability to return an unboxed prediction.  This will reduce the overhead
     * of this function call.
     * @param example an example
     * @param learn whether to call the learn or predict VW functions.
     * @return an <em>UNBOXED</em> prediction.
     */
    private float learnOrPredict(final String[] example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return predictMultiline(example, learn, nativePointer);
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
    public float predict(final String example) {
        return learnOrPredict(example, false);
    }

    /**
     * Runs learning on <code>example</code> and returns the prediction output.
     *
     * @param example a single vw example string
     * @return A prediction
     */
    public float learn(final String example) {
        return learnOrPredict(example, true);
    }

    /**
     * Runs prediction on <code>example</code> and returns the prediction output.
     *
     * @param example a multiline vw example string
     * @return A prediction
     */
    public float predict(final String[] example) {
        return learnOrPredict(example, false);
    }

    /**
     * Runs learning on <code>example</code> and returns the prediction output.
     *
     * @param example a multiline vw example string
     * @return A prediction
     */
    public float learn(final String[] example) {
        return learnOrPredict(example, true);
    }

    protected abstract float predict(String example, boolean learn, long nativePointer);
    protected abstract float predictMultiline(String[] example, boolean learn, long nativePointer);
}
