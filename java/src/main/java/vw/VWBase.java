package vw;

import vw.jni.NativeUtils;

import java.io.Closeable;
import java.io.IOException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

abstract class VWBase implements Closeable {
    private volatile static boolean loadedNativeLibrary = false;
    private static final Lock STATIC_LOCK = new ReentrantLock();

    private boolean isOpen;

    /**
     * Load tests have shown that a Lock is faster than synchronized (this).
     * It was originally hypothesized that {@link java.util.concurrent.locks.ReadWriteLock} would be a better
     * alternative, but at this time this is not possible cause of <a href="https://mail.google.com/mail/u/0/?ui=2&ik=cdb4bef19b&view=lg&msg=14dfe18a4f82a199#14dfe18a4f82a199_5a">this</a>.
     */
    protected final Lock lock;
    protected final long nativePointer;
    private final String command;

    /**
     * Create a new VW instance that is ready to either create predictions or learn based on examples.
     * This allows the user to instead of using the prepackaged JNI layer to load their own external JNI layer.
     * This means that if a user wants to use this code with an OS that is not supported they would follow the following steps:<br>
     * 1.  Download VW<br>
     * 2.  Build VW for the OS they wish to support<br>
     * 3.  Call either {@link System#load(String)} or {@link System#loadLibrary(String)}<br>
     * If a user wishes to use the prepackaged JNI libraries (which is encouraged) then no additional steps need to be taken.
     * @param command The same string that is passed to VW, see
     *                <a href="https://github.com/JohnLangford/vowpal_wabbit/wiki/Command-line-arguments">here</a>
     *                for more information
     */

    protected VWBase(final String command) {
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

    final protected boolean isOpen() {
        return isOpen;
    }

    private native long initialize(String command);
    private native void closeInstance(long nativePointer);
}
