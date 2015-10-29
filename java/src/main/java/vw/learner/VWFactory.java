package vw.learner;

import vw.jni.NativeUtils;

import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * This is the only entrance point to create a VWLearner.  It is the responsibility of the user to supply the type they want
 * given the VW command.  If that type is incorrect a {@link java.lang.ClassCastException} is thrown.
 * @author jmorra
 */
final public class VWFactory {
    private volatile static boolean loadedNativeLibrary = false;
    private static final Lock STATIC_LOCK = new ReentrantLock();

    enum VWReturnType {
        Unknown, VWFloatType, VWIntType, VWIntArrayType, VWFloatArrayType
    }

    private VWFactory() {}

    /**
     * This is the only way to construct a VW Predictor.  The goal here is to provide a typesafe way of getting an predictor
     * which will return the correct output type given the command specified.
     * <pre>
     *  {@code
     *      VWIntLearner vw = VWFactory.getVWLearner("--cb 4");
     *  }
     * </pre>
     * @param command The VW initialization command.
     * @param <T> The type of learner expected.  Note that this type implicitly specifies the output type of the learner.
     * @throws ClassCastException If the specified type T is not a super type of the returned learner given the command.
     * @return A VW Learner
     */
    @SuppressWarnings("unchecked")
    public static <T extends VWLearner> T getVWLeaner(final String command) {
        long nativePointer = initializeVWJni(command);
        VWReturnType returnType = getReturnType(nativePointer);
        while (true) {
            getReturnType(nativePointer);
        }
/*
        VWLearner baseLearner;
        switch (returnType) {
            case VWFloatType:
                baseLearner = new VWFloatLearner(nativePointer);
                break;
            case VWIntType:
                baseLearner = new VWIntLearner(nativePointer);
                break;
            case VWFloatArrayType:
                baseLearner = new VWFloatArrayLearner(nativePointer);
                break;
            case VWIntArrayType:
                baseLearner = new VWIntArrayLearner(nativePointer);
                break;
            case Unknown:
            default:
                // Doing this will allow for all cases when a C object is made to be closed.
                closeInstance(nativePointer);
                throw new IllegalArgumentException("Unknown VW return type using command: " + command);
        }

        // In the case that we have a ClassCastException the C object was still created and must be closed.
        // This will ensure that that closing happens
        try {
            return (T)baseLearner;
        }
        catch (ClassCastException e) {
            try {
                baseLearner.close();
            }
            catch (IOException e1) {
                // Ignored, closing a VWLearner cannot fail
            }
            throw e;
        }*/
    }

    /**
     * @param command The same string that is passed to VW, see
     *                <a href="https://github.com/JohnLangford/vowpal_wabbit/wiki/Command-line-arguments">here</a>
     *                for more information
     * @return
     */
    private static long initializeVWJni(final String command) {
        long nativePointer;
        try {
            nativePointer = initialize(command);
            loadedNativeLibrary = true;
        }
        catch (UnsatisfiedLinkError e) {
            loadNativeLibrary();
            nativePointer = initialize(command);
        }
        return nativePointer;
    }

    private static void loadNativeLibrary() {
        // By making use of a static lock here we make sure this code is only executed once globally.
        if (!loadedNativeLibrary) {
            STATIC_LOCK.lock();
            try {
                if (!loadedNativeLibrary) {
                    NativeUtils.loadOSDependentLibrary("/vw_jni", ".lib");
                    loadedNativeLibrary = true;
                }
            }
            catch (IOException e) {
                // Here I've chosen to rethrow the exception as an unchecked exception because if the native
                // library cannot be loaded then the exception is not recoverable from.
                throw new RuntimeException(e);
            }
            finally {
                STATIC_LOCK.unlock();
            }
        }
    }

    private static native long initialize(String command);
    private static native VWReturnType getReturnType(long nativePointer);

    // Closing needs to be done here when initialization fails and by VWBase
    static native void closeInstance(long nativePointer);
}
