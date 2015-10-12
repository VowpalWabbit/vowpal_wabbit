package vw.learner;

public final class VWFloatArrayLearner extends VWGenericBase<float[]> {
    public VWFloatArrayLearner(String command) {
        super(command);
    }

    protected native float[] predict(String example, boolean learn, long nativePointer);
}
