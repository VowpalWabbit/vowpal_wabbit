package vowpalWabbit.learner;

import common.Native;

/**
 * This is the only entrance point to create a VWLearner.  It is the responsibility of the user to supply the type they want
 * given the VW command.  If that type is incorrect a {@link java.lang.ClassCastException} is thrown.  Refer to
 * {@link #create(String)} for more information.
 * @author jmorra
 */
final public class VWLearners {
    private enum VWReturnType {
        Unknown, ActionProbs, ActionScores, Multiclass, Multilabels, Prob, Scalar, Scalars, DecisionProbs,
        ActionPDFValue, PDF, ActiveMulticlass, NoPred
    }

    static {
        try {
            // Load from java.library.path
            System.loadLibrary("vw_jni");
        } catch (UnsatisfiedLinkError e) {
            // Load from JAR
            Native.load();
        }
    }

    private VWLearners() {}

    /**
     * This is the only way to construct a VW Predictor.  The goal here is to provide a typesafe way of getting an predictor
     * which will return the correct output type given the command specified.
     * <pre>
     * {@code
     *     VWMulticlassLearner vw = VWFactory.createVWLearner("--cb 4");
     * }
     * </pre>
     *
     * NOTE: It is very important to note that if this method results in a {@link java.lang.ClassCastException} then there
     * WILL be a memory leak as the exception occurs in the calling method not this method due to type erasures.  It is therefore
     * imperative that if the caller of this method is unsure of the type returned that it should specify <code>T</code>
     * as {@link VWBase} and do the casting on it's side so that closing the method can be guaranteed.
     * @param command The VW initialization command.
     * @param <T> The type of learner expected.  Note that this type implicitly specifies the output type of the learner.
     * @return A VW Learner
     */
    @SuppressWarnings("unchecked")
    public static <T extends VWLearner> T create(String command) {
        if(command.indexOf("--no_stdin") == -1)
            command += " --no_stdin";
        long nativePointer = initialize(command);
        VWReturnType returnType = getReturnType(nativePointer);

        switch (returnType) {
            case ActionProbs: return (T)new VWActionProbsLearner(nativePointer);
            case ActionScores: return (T)new VWActionScoresLearner(nativePointer);
            case Multiclass: return (T)new VWMulticlassLearner(nativePointer);
            case Multilabels: return (T)new VWMultilabelsLearner(nativePointer);
            case Prob: return (T)new VWProbLearner(nativePointer);
            case Scalar: return (T)new VWScalarLearner(nativePointer);
            case Scalars: return (T)new VWScalarsLearner(nativePointer);
            case DecisionProbs: return (T)new VWCCBLearner(nativePointer);
            case ActionPDFValue: return (T)new VWActionPDFValueLearner(nativePointer);
            case PDF: return (T)new VWPDFLearner(nativePointer);
            case ActiveMulticlass: return (T)new VWActiveMulticlassLearner(nativePointer);
            case NoPred: return (T)new VWNoPredLearner(nativePointer);
            case Unknown:
            default:
                // Doing this will allow for all cases when a C object is made to be closed.
                closeInstance(nativePointer);
                throw new IllegalArgumentException("Unknown VW return type using command: " + command);
        }
    }

    private static native long initialize(String command);
    private static native VWReturnType getReturnType(long nativePointer);

    // Closing needs to be done here when initialization fails and by VWBase
    static native void closeInstance(long nativePointer);

    // Closing needs to be done here when initialization fails and by VWBase
    static native void performRemainingPasses(long nativePointer);

    static native void saveModel(long nativePointer, String filename);

    static native boolean isMultiline(long nativePointer);
}
