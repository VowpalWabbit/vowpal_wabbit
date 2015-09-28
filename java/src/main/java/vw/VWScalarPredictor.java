package vw;

/**
 * Created by deak on 9/28/15.
 *
 */
public class VWScalarPredictor extends VWGenericBase<Float> {

    public VWScalarPredictor(String command) {
        super(command);
    }

    public Float predict(final String example, final boolean learn, final long nativePointer) {
        return predictFloat(example, learn, nativePointer);
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
                return predictFloat(example, learn, nativePointer);
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

    private native float predictFloat(String example, boolean learn, long nativePointer);
}
