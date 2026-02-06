package vowpalWabbit.learner;

import vowpalWabbit.responses.PDFValue;

/**
 * Learner for CATS reductions that return a continuous action PDF value prediction.
 */
public final class VWActionPDFValueLearner extends VWLearnerBase<PDFValue> {
    VWActionPDFValueLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native PDFValue predict(String example, boolean learn, long nativePointer);

    @Override
    protected native PDFValue predictMultiline(String[] example, boolean learn, long nativePointer);
}
