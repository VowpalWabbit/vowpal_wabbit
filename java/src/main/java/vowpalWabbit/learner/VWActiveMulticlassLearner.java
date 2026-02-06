package vowpalWabbit.learner;

import vowpalWabbit.responses.ActiveMulticlass;

/**
 * Learner for cs_active reductions that return an active multiclass prediction.
 */
public final class VWActiveMulticlassLearner extends VWLearnerBase<ActiveMulticlass> {
    VWActiveMulticlassLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native ActiveMulticlass predict(String example, boolean learn, long nativePointer);

    @Override
    protected native ActiveMulticlass predictMultiline(String[] example, boolean learn, long nativePointer);
}
