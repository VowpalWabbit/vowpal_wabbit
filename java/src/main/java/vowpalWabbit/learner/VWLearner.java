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
     * @return a Callable responsible for closing the model.  The Callable should return <code>true</code> if
     *         model was open and an attempt is made to close the model.  A return value of <code>false</code>
     *         indicates an attempt to close the model was made previously.  This is consistent with other
     *         languages where a <code>true</code> return value indicates success.
     */
    Callable<Boolean> closer();

    void saveModel(File filename);

    /**
     * @return true if the underlying VW learner requires multiline examples.
     */
    boolean isMultiline();
}
