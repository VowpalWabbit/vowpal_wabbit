package vw.learner;

public final class VWFloatArrayLearner extends VWGenericBase<float[]> {
    protected VWFloatArrayLearner(final long nativePointer) {
        super(nativePointer);
    }

    protected native float[] predict(String example, boolean learn, long nativePointer);
}
