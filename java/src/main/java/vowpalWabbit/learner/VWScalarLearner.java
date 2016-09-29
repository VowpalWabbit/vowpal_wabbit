package vowpalWabbit.learner;

/**
 * @author deak
 */
final public class VWScalarLearner extends VWFloatLearner {
    VWScalarLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native float predict(String example, boolean learn, long nativePointer);

    @Override
    protected native float predictMultiline(String[] example, boolean learn, long nativePointer);
}
