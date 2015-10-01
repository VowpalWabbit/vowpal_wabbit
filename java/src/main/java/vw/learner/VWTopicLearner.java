package vw.learner;

public final class VWTopicLearner extends VWGenericBase<float[]> {
    public VWTopicLearner(String command) {
        super(command);
    }

    protected native float[] predict(String example, boolean learn, long nativePointer);
}
