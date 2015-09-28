package vw;

import java.io.IOException;

/**
 * Created by deak on 9/27/15.
 */
abstract class VWGenericBase<T> extends VWBase implements VWGeneric<T> {
    protected VWGenericBase(final String command) {
        super(command);
    }

    @Override
    public final T learn(String example) {
        return learnOrPredict(example, true);
    }

    @Override
    public final T predict(String example) {
        return learnOrPredict(example, false);
    }

    protected abstract T predict(String example, boolean learn, long nativePointer);

    private T learnOrPredict(final String example, final boolean learn) {
        lock.lock();
        try {
            if (isOpen()) {
                return predict(example, learn, nativePointer);
            }
            throw new IllegalStateException("Already closed.");
        }
        finally {
            lock.unlock();
        }
    }
}
