package vowpalWabbit.learner;

import vowpalWabbit.responses.DecisionScores;

public final class VWCCBLearner extends VWLearnerBase<DecisionScores> {
    VWCCBLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native DecisionScores predict(String example, boolean learn, long nativePointer);

    @Override
    protected native DecisionScores predictMultiline(String[] example, boolean learn, long nativePointer);
}
