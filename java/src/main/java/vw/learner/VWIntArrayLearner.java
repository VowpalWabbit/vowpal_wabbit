package vw.learner;

public final class VWIntArrayLearner extends VWGenericBase<int[]> {
    protected VWIntArrayLearner(final long nativePointer) {
        super(nativePointer);
    }

    protected native int[] predict(String example, boolean learn, long nativePointer);
}
