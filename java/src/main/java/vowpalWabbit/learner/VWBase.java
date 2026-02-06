package vowpalWabbit.learner;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.Callable;
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
abstract class VWBase implements VWLearner {
    private volatile boolean isOpen;

    /**
     * Load tests have shown that a Lock is faster than synchronized (this).
     * It was originally hypothesized that {@link java.util.concurrent.locks.ReadWriteLock} would be a better
     * alternative, but at this time this is not possible cause of <a href="https://mail.google.com/mail/u/0/?ui=2&ik=cdb4bef19b&view=lg&msg=14dfe18a4f82a199#14dfe18a4f82a199_5a">this</a>.
     */
    final Lock lock;
    protected final long nativePointer;

    // It would appear that performing multiple passes from the JNI layer is not thread safe even across multiple models.
    // Because of this we need a GLOBAL lock to do mulitiple passes.
    private final static Lock globalLock = new ReentrantLock();

    /**
     * Create a new VW instance that is ready to either create predictions or learn based on examples.
     * This allows the user to instead of using the prepackaged JNI layer to load their own external JNI layer.
     * This means that if a user wants to use this code with an OS that is not supported they would follow the following steps:<br>
     * 1.  Download VW<br>
     * 2.  Build VW for the OS they wish to support<br>
     * 3.  Call either {@link System#load(String)} or {@link System#loadLibrary(String)}<br>
     * If a user wishes to use the prepackaged JNI libraries (which is encouraged) then no additional steps need to be taken.
     */
    VWBase(final long nativePointer) {
        isOpen = true;
        lock = new ReentrantLock();
        this.nativePointer = nativePointer;
    }

    /**
     * Close the VW instance.  close or closeAsync MUST be called in order to free up the native memory.
     * After this is called no future calls to this object are permitted.  Calling any combination of
     * <code>closer().call()</code> and <code>close()</code> multiple times should have no effect after
     * the first call is made.  This is consistent with the guarantees in the Closable interface.
     */
    @Override
    final public void close() throws IOException {
        try {
            closer().call();
        }
        catch (Exception e) {
            throw new IOException("An exception occurred while attempting to close VW Model with native pointer " +
                                  nativePointer, e);
        }
    }

    @Override
    final public Callable<Boolean> closer() {
        return new Closer();
    }

    final boolean isOpen() {
        return isOpen;
    }

    /**
     * Save the model in the VW instance.
     */
    public void saveModel(File filename) {
        lock.lock();
        try {
            if (isOpen()) {
                VWLearners.saveModel(nativePointer, filename.getPath());
            } else {
                throw new IllegalStateException("Already closed.");
            }
        }
        finally {
            lock.unlock();
        }
    }

    @Override
    public boolean isMultiline() {
        lock.lock();
        try {
            if (isOpen()) {
                return VWLearners.isMultiline(nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }

    @Override
    public void runDriver() {
        lock.lock();
        try {
            if (isOpen()) {
                VWBase.globalLock.lock();
                try {
                    VWLearners.runDriver(nativePointer);
                }
                finally {
                    globalLock.unlock();
                }
            } else {
                throw new IllegalStateException("Already closed.");
            }
        }
        finally {
            lock.unlock();
        }
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

    private class Closer implements Callable<Boolean> {
        private Closer() {}

        /**
         * <p>
         * Close the underlying VW model.
         * </p>
         *
         * <p>
         * May throw an unchecked exception if the underlying native code throws an Exception.
         * </p>
         * @return <code>true</code> if model was open and an attempt is made to close the model.
         *         A return value of <code>false</code> indicates an attempt to close the model was
         *         made previously.  This is consistent with other languages where a <code>true</code>
         *         return value indicates success.
         */
        @Override
        public Boolean call() {
            lock.lock();
            try {
                final boolean attemptingToClose = isOpen;
                if (isOpen) {
                    isOpen = false;
                    VWBase.globalLock.lock();
                    try {
                        VWLearners.performRemainingPasses(nativePointer);
                    }
                    finally {
                        globalLock.unlock();
                    }
                    VWLearners.closeInstance(nativePointer);
                }
                return attemptingToClose;
            }
            finally {
                lock.unlock();
            }
        }
    }
}
