package vowpalWabbit.learner;

import vowpalWabbit.responses.PDF;

/**
 * Learner for CATS reductions that return a continuous action probability density function.
 */
public final class VWPDFLearner extends VWLearnerBase<PDF> {
    VWPDFLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native PDF predict(String example, boolean learn, long nativePointer);

    @Override
    protected native PDF predictMultiline(String[] example, boolean learn, long nativePointer);
}
