package vowpalWabbit.learner;

/**
 * @author deak
 */
final public class VWMulticlassLearner extends VWIntLearner {
    VWMulticlassLearner(final long nativePointer) {
        super(nativePointer);
    }

    /**
     * Runs prediction on <code>example</code> and returns the string predictions output for each line.
     *
     * @param example a multiline vw example string
     * @return predictions for each line in example
     */
    public String[] predictMultipleNamedLabels(final String[] example) {
        return learnOrPredictMultipleNamedLabels(example, false);
    }

    /**
     * Runs prediction on <code>example</code> and returns the predictions output for each line.
     *
     * @param example a multiline vw example string
     * @return predictions for each line in example
     */
    public int[] predictMultiple(final String[] example) {
        return learnOrPredictMultiple(example, false);
    }

    /**
     * <code>learnOrPredict</code> returns an unboxed prediction. This will reduce the overhead
     * of this function call.
     * @param example an example
     * @param learn whether to call the learn or predict VW functions.
     * @return an <em>UNBOXED</em> string prediction.
     */
    private String[] learnOrPredictMultipleNamedLabels(final String[] example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return predictNamedLabelsForAllLines(example, learn, nativePointer);
            }
            throw new IllegalStateException("Already closed");
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
    private int[] learnOrPredictMultiple(final String[] example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return predictForAllLines(example, learn, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }

    @Override
    protected native int predict(String example, boolean learn, long nativePointer);

    @Override
    protected native int predictMultiline(String[] example, boolean learn, long nativePointer);

    // add-on functionaility to provide all label int index predictions for multiline examples
    // author: zhilians@
    protected native int[] predictForAllLines(String[] example, boolean learn, long nativePointer);

    // add-on functionaility to provide all label named predictions for multiline examples.
    // if name labels were not provided in model file, retrun string of int index prediction instead
    // TODO: remove this assumption and throw an exception if invoked without name labels defined in the model
    // author: zhilians@
    protected native String[] predictNamedLabelsForAllLines(String[] example, boolean learn, long nativePointer);
}
