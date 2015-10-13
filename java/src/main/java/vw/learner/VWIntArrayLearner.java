package vw.learner;

public final class VWIntArrayLearner extends VWGenericBase<int[]> {
    public VWIntArrayLearner(String command) {
        super(command);
    }

    protected native int[] predict(String example, boolean learn, long nativePointer);
}
