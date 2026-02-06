package vowpalWabbit.learner;

/**
 * Learner for VW reductions that produce no prediction output (e.g. cbzo).
 */
final public class VWNoPredLearner extends VWLearnerBase<Object> {
    VWNoPredLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native Object predict(String example, boolean learn, long nativePointer);

    @Override
    protected native Object predictMultiline(String[] example, boolean learn, long nativePointer);
}
