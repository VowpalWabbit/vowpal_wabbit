package vowpalWabbit.learner;

/**
 * @author deak
 */
final public class VWMulticlassLearner extends VWIntLearner {
    VWMulticlassLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native int predict(String example, boolean learn, long nativePointer);

    @Override
    protected native int predictMultiline(String[] example, boolean learn, long nativePointer);
}
