package vw;

import vw.jni.NativeUtils;

import java.io.Closeable;
import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * A JNI layer for submitting examples to VW and getting predictions back.  It should be noted
 * that at this time VW is NOT thread safe, and therefore neither is the JNI layer.  It should be noted
 * that this was originally written with a bulk interface that was later removed because of benchmarking
 * data found <a href="https://microbenchmarks.appspot.com/runs/817d246a-5f90-478a-bc27-d5912d2ff874#r:scenario.benchmarkSpec.methodName,scenario.benchmarkSpec.parameters.loss,scenario.benchmarkSpec.parameters.mutabilityPolicy,scenario.benchmarkSpec.parameters.nExamples">here</a>.
 * Please note that close MUST be called in order to free up the memory on the C side.
 */
public class VW implements Closeable {

    /**
     * This main method only exists to test the library implementation.  To test it just run
     * java -cp target/vw-jni-*-SNAPSHOT.jar vw.VW
     * @param args No args needed.
     */
    public static void main(String[] args) {
        new VW("").close();
        new VW("--quiet").close();
    }

    private volatile static boolean loadedNativeLibrary = false;
    private static final Lock STATIC_LOCK = new ReentrantLock();
    private boolean isOpen;

    /**
     * Load tests have shown that a Lock is faster than synchronized (this).
     * It was originally hypothesized that {@link java.util.concurrent.locks.ReadWriteLock} would be a better
     * alternative, but at this time this is not possible cause of <a href="https://mail.google.com/mail/u/0/?ui=2&ik=cdb4bef19b&view=lg&msg=14dfe18a4f82a199#14dfe18a4f82a199_5a">this</a>.
     */
    private final Lock lock;
    private final long nativePointer;
    private final String command;

    /**
     * Create a new VW instance that is ready to either create predictions or learn based on examples.
     * This allows the user to instead of using the prepackaged JNI layer to load their own external JNI layer.
     * This means that if a user wants to use this code with an OS that is not supported they would follow the following steps:<br/>
     * 1.  Download VW<br/>
     * 2.  Build VW for the OS they wish to support<br/>
     * 3.  Call either {@link System#load(String)} or {@link System#loadLibrary(String)}<br/>
     * If a user wishes to use the prepackaged JNI libraries (which is encouraged) then no additional steps need to be taken.
     * @param command The same string that is passed to VW, see
     *                <a href="https://github.com/JohnLangford/vowpal_wabbit/wiki/Command-line-arguments">here</a>
     *                for more information
     */
    public VW(String command) {
        isOpen = true;
        lock = new ReentrantLock();
        long currentNativePointer;
        try {
            currentNativePointer = initialize(command);
            loadedNativeLibrary = true;
        }
        catch (UnsatisfiedLinkError e) {
            loadNativeLibrary();
            currentNativePointer = initialize(command);
        }
        this.command = command;
        nativePointer = currentNativePointer;
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

    /**
     * Gets the command this instance was initialized with.
     * @return The initialization command.
     */
    public String getCommand() {
        return command;
    }

    /**
     * Runs prediction on <code>example</code> and returns the prediction output.
     *
     * @param example a single vw example string
     * @return A prediction
     */
    public float predict(String example) {
        lock.lock();
        try {
            if (isOpen) {
                return predict(example, false, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }

    /**
     * Runs learning on <code>example</code> and returns the prediction output.
     *
     * @param example a single vw example string
     * @return A prediction
     */
    public float learn(String example) {
        lock.lock();
        try {
            if (isOpen) {
                return predict(example, true, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }

    /**
     * Close the VW instance.  This MUST be called in order to free up the native memory.
     * After this is called no future calls to this object are permitted.
     */
    public void close() {
        lock.lock();
        try {
            if (isOpen) {
                isOpen = false;
                closeInstance(nativePointer);
            }
        }
        finally {
            lock.unlock();
        }
    }

    public static native String version();
    private native long initialize(String command);
    private native float predict(String example, boolean learn, long nativePointer);
    private native void closeInstance(long nativePointer);
}
