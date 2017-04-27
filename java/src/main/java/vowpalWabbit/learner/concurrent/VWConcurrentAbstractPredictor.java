package vowpalWabbit.learner.concurrent;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Abstracts out synchronized and asynchronized prediction method for the concurrent predict API.
 * 
 * @author atulvkamat
 * @author zhilians
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
     * @return prediction result or null if timed out
     * @throws InterruptedException
     *             expecting owner of the thread to deal with this interrupt
     *             exception.
     */
    public O predict(final E example, final long timeoutInMillis) throws InterruptedException {
        Future<O> future = null;
        try {
            future = predict(example);
            O result = future.get(timeoutInMillis, TimeUnit.MILLISECONDS);
            return result;
        } catch (ExecutionException e) {
            throw new RuntimeException(String.format("Exception while predicting for example: %s", example),
                    e.getCause());
        } catch (TimeoutException e) {
            future.cancel(true/* interrupt */);
            return null;
        }
    }

    /**
     * Makes prediction against the example by queuing the prediction request
     * 
     * A Future will be returned so that further action can be taken asynchronously
     * 
     * @param example
     * @return A {@link Future} of the prediction output
     * 
     */
    public Future<O> predict(final E example) {
        Future<O> future = getLearnerExecutor().submit(new Callable<O>() {

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

        return future;
    }
}