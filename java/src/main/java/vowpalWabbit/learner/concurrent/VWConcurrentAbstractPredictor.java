package vowpalWabbit.learner.concurrent;

import java.util.Optional;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Abstracts out timeout logic associated with predict api.
 * 
 * @author atulvkamat
 *
 * @param <P>
 *            is the predictor. Eg: VWMultilabelsLearner.
 * @param <O>
 *            is the prediction output. Eg: Multilabels.
 * @param <E>
 *            is the example type. Eg: String.
 */
public abstract class VWConcurrentAbstractPredictor<P, O, E> {

    private final ExecutorService learnerExecutor;
    private final LinkedBlockingQueue<P> vwPredictorPool;

    public VWConcurrentAbstractPredictor(final String predictorName,
            final ExecutorService learnerExecutor,
            final LinkedBlockingQueue<P> vwPredictorPool) {
        this.learnerExecutor = learnerExecutor;
        this.vwPredictorPool = vwPredictorPool;
    }

    private ExecutorService getLearnerExecutor() {
        return learnerExecutor;
    }

    private P borrowPredictorFromPool() throws InterruptedException {
        return vwPredictorPool.take();
    }

    private void returnPredictorToPool(P learner) {
        try {
            vwPredictorPool.add(learner);
        } catch (Exception e) {
            throw new RuntimeException(
                    "Error returning the predictor back to pool.", e);
        }
    }

    /**
     * Its the concrete classes responsibility to trigger prediction based on
     * the predictor and the example.
     * 
     * @param predictor
     * @param example
     * @return prediction output
     */
    protected abstract O predict(final P predictor, final E example);

    /**
     * Makes prediction against the example by queuing the prediction request
     * and timeboxing it to the timeout window specified.
     * 
     * @param example
     * @param timeoutInMillis
     * @return Optional prediction with the result being empty if not returned
     *         within the timeout window.
     * @throws InterruptedException
     *             - expecting owner of the thread to deal with this interrupt
     *             exception.
     */
    public Optional<O> predict(final E example, final long timeoutInMillis)
            throws InterruptedException {
        Future<O> future = null;
        try {
            future = getLearnerExecutor().submit(new Callable<O>() {
                @Override
                public O call() throws Exception {
                    P predictor = borrowPredictorFromPool();
                    try {
                        return predict(predictor, example);
                    } finally {
                        returnPredictorToPool(predictor);
                    }
                }
            });

            Optional<O> result = Optional
                    .of(future.get(timeoutInMillis, TimeUnit.MILLISECONDS));

            return result;
        } catch (ExecutionException e) {
            throw new RuntimeException(
                    String.format("Exception while predicting for example: %s",
                            example),
                    e.getCause());
        } catch (TimeoutException e) {
            future.cancel(true/* interrupt */);
            return Optional.empty();
        }
    }
}