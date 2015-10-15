package vw.learner;

/**
 * This abstract base class allows the authors of new model wrappers to just write
 * java code like the following:
 *
 * <pre>
 * {@code
 * public final class SomeLearner extends VWGenericBase<float[]> {
 *   public VWFloatArrayLearner(String command) { super(command); }
 *   protected native float[] predict(String example, boolean learn, long nativePointer);
 * }
 * }
 * </pre>
 *
 * Then the author can simply concentrate on writing the <em>JNI</em> C code.
 *
 * @author deak
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
