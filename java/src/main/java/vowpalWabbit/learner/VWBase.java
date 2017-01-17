package vowpalWabbit.learner;

import java.io.Closeable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * The base class for VW predictors.  This class is responsible for:
 *
 * <ol>
 * <li>Keeping track of VW on the <em>C</em> side.</li>
 * <li>Closing the VW predictor and cleaning up the memory on the <em>C</em> side.</li>
 * <li>Bookkeeping, such as keeping the <em>C</em> memory pointer.</li>
 * </ol>
 *
 *
 */
abstract class VWBase implements Closeable {
    private boolean isOpen;

    /**
     * Load tests have shown that a Lock is faster than synchronized (this).
     * It was originally hypothesized that {@link java.util.concurrent.locks.ReadWriteLock} would be a better
     * alternative, but at this time this is not possible cause of <a href="https://mail.google.com/mail/u/0/?ui=2&ik=cdb4bef19b&view=lg&msg=14dfe18a4f82a199#14dfe18a4f82a199_5a">this</a>.
     */
    protected final Lock lock;
    protected final long nativePointer;

    /**
     * Create a new VW instance that is ready to either create predictions or learn based on examples.
     * This allows the user to instead of using the prepackaged JNI layer to load their own external JNI layer.
     * This means that if a user wants to use this code with an OS that is not supported they would follow the following steps:<br>
     * 1.  Download VW<br>
     * 2.  Build VW for the OS they wish to support<br>
     * 3.  Call either {@link System#load(String)} or {@link System#loadLibrary(String)}<br>
     * If a user wishes to use the prepackaged JNI libraries (which is encouraged) then no additional steps need to be taken.
     */
    protected VWBase(final long nativePointer) {
        isOpen = true;
        lock = new ReentrantLock();
        this.nativePointer = nativePointer;
    }

    /**
     * Close the VW instance.  This MUST be called in order to free up the native memory.
     * After this is called no future calls to this object are permitted.
     */
    @Override
    public void close() {
        lock.lock();
        try {
            if (isOpen) {
                isOpen = false;
                VWLearners.closeInstance(nativePointer);
            }
        }
        finally {
            lock.unlock();
        }
    }

    final boolean isOpen() {
        return isOpen;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        VWBase vwBase = (VWBase) o;

        return nativePointer == vwBase.nativePointer;

    }

    @Override
    public int hashCode() {
        return (int) (nativePointer ^ (nativePointer >>> 32));
    }
}
