package vowpalWabbit.learner;

/**
 * @author deak
 */
final public class VWProbLearner extends VWFloatLearner {
    VWProbLearner(final long nativePointer) {
        super(nativePointer);
    }

    @Override
    protected native float predict(String example, boolean learn, long nativePointer);

    @Override
    protected native float predictMultiline(String[] example, boolean learn, long nativePointer);

    public static native String version();
}
