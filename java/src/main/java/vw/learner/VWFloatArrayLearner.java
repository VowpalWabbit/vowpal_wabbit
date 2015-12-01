package vw.learner;

public final class VWFloatArrayLearner extends VWLearnerBase<float[]> {
    VWFloatArrayLearner(final long nativePointer) {
        super(nativePointer);
    }

    protected native float[] predict(String example, boolean learn, long nativePointer);
}
