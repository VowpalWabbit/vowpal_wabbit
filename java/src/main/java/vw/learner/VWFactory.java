package vw.learner;

import vw.jni.NativeUtils;

import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Created by jmorra on 10/28/15.
 */
public class VWFactory {
    private volatile static boolean loadedNativeLibrary = false;
    private static final Lock STATIC_LOCK = new ReentrantLock();

    private VWFactory() {}

    public static VWGeneric getVWLeaner(final String command) {
        long nativePointer = initializeVWJni(command);
        VWReturnType returnType = getReturnType(nativePointer);

        switch (returnType) {
            case VWFloatType: return new VWFloatLearner(nativePointer);
            case VWIntType: return new VWIntLearner(nativePointer);
            case VWFloatArrayType: return new VWFloatArrayLearner(nativePointer);
            case VWIntArrayType: return new VWIntArrayLearner(nativePointer);
            default: throw new IllegalArgumentException("Unknown VW return type");
        }
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
    static native VWReturnType getReturnType(long nativePointer);
}
