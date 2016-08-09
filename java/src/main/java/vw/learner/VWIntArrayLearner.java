package vw.learner;

public final class VWIntArrayLearner extends VWLearnerBase<int[]> {
    VWIntArrayLearner(final long nativePointer, final long predictionFunctionPointer) {
        super(nativePointer, predictionFunctionPointer);
    }

    protected native int[] predict(String example, boolean learn, long nativePointer, long predictionFunctionPointer);
    protected native int[] predictMultiline(String[] example, boolean learn, long nativePointer, long predictionFunctionPointer);
}
