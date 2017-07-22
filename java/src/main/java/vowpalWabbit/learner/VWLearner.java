package vowpalWabbit.learner;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.Callable;

/**
 * This is the super type of all different typed VW learners.  This type exists to ensure that the
 * {@link VWLearners#create(String)} method has a super type.
 */
public interface VWLearner extends Closeable {

    /**
     * This will close the underlying VW model after performing any
     * Close should be written in terms of closeAsync, followed by blocking for the result.
     */
    @Override void close() throws IOException;

    /**
     * <p>
     * Closing the underlying VW model may take a long time (for instance if it is to make multiple passes).
     * To address this, this method provides a strategy for closing the model asynchronously by executing
     * the returned Callable.
     * </p>
     *
     * <p>
     * It can be called synchronously or, if desired, can be passed to an ExecutorService to be executed
     * asynchronously, or even to a Guava ListeningExecutorService to produce a ListenableFuture to which
     * callbacks can be attached.
     * </p>
     *
     * <p>
     * <b>NOTE</b>: no locking should be necessary to produce the closer, but locking may be required when the
     * closer's <em>call</em> method is invoked.
     * </p>
     *
     * <p>
     * <b>NOTE</b>: If the Callable is interrupted once started, there's a good chance that both the underlying
     * VW model could corrupted and it may not be closable in the future (which could result in a memory leak).
     * </p>
     *
     * @return a Callable responsible for closing the model.  <em>As the signature indicates, the return value
     * from the Callable is Void and Void has no possible values since it is uninstantiable.  <b>Therefore,
     * do not attempt to use it.</b></em>
     */
    Callable<Void> closer();

    void saveModel(File filename);
}
