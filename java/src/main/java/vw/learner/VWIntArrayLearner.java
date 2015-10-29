package vw.learner;

public final class VWIntArrayLearner extends VWLearnerBase<int[]> {
    VWIntArrayLearner(final long nativePointer) {
        super(nativePointer);
    }

    protected native int[] predict(String example, boolean learn, long nativePointer);
}
