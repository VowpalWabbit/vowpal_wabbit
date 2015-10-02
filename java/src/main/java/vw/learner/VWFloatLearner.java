package vw.learner;

/**
 * Created by deak on 10/1/15.
 */
final public class VWFloatLearner extends VWBase {
    public VWFloatLearner(String command) {
        super(command);
    }

    /**
     * <code>learnOrPredict</code>, <code>predictSpecialized</code> and <code>learnSpecialized</code>
     * functions mimic the functionality in VWGenericBase<Float> but are required if we don't want to
     * box the floats like they are boxed in the <code>predict(example, learn, nativePointer)</code>.
     * @param example an example
     * @param learn whether the learn prior to returning the prediction.
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

    private native float predict(String example, boolean learn, long nativePointer);
}
