package vowpalWabbit.learner;

import vowpalWabbit.responses.ActionProbs;

public final class VWActionProbsLearner extends VWLearnerBase<ActionProbs> {
    VWActionProbsLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native ActionProbs predict(String example, boolean learn, long nativePointer);

    @Override
    protected native ActionProbs predictMultiline(String[] example, boolean learn, long nativePointer);
}
