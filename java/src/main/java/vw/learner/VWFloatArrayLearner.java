package vw.learner;

public final class VWFloatArrayLearner extends VWLearnerBase<float[]> {
    VWFloatArrayLearner(final long nativePointer, final long predictionFunctionPointer) {
        super(nativePointer, predictionFunctionPointer);
    }

    protected native float[] predict(String example, boolean learn, long nativePointer, long predictionFunctionPointer);
    protected native float[] predictMultiline(String[] example, boolean learn, long nativePointer, long predictionFunctionPointer);
}
