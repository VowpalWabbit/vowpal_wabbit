package vowpalWabbit.learner.concurrent;

import java.util.concurrent.Callable;
import java.util.concurrent.CompletionService;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorCompletionService;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;

import vowpalWabbit.learner.VWLearners;
import vowpalWabbit.learner.VWMulticlassLearner;
import vowpalWabbit.learner.VWMultilabelsLearner;

/**
 * Factory responsible for creating a single concurrent predictor instance which behind the
 * scenes distributes the prediction work to a learner pool. One should ensure that number of threads
 * in the threadpool is atleast as big as the pool size otherwise you are wasting memory occupied by the
 * extra learner instances.
 * 
 * TODO: Currently if a model occupies X MB, and your learner pool size is 10 then your overall memory 
 * footprint in 10 * x MB since we initialize multiple instances of the same vw model. There is a 
 * seed_vw_model which helps in sharing memory across multiple instances of the same learner. To achieve this,
 * we need to modify the learner initialization code below to create a new instance just once and use seed_vw_model for
 * creating the rest.
 * 
 * @author atulvkamat
 */
public class VWConcurrentLearnerFactory {

    /**
     * Create a multilabels predictor that distributes the work of multilabel
     * prediction to backing threadpool specified as executor service. One can
     * use the predictor returned to predict with a timeout.
     * 
     * @param learnerExecutor
     * @param poolSize - for predictor pool created internally.
     * @param command
     * @return VWConcurrentMultilabelsPredictor instance.
     * @throws InterruptedException 
     */
    public static VWConcurrentMultilabelsPredictor createMultilabelsPredictor(
            final String predictorName,
            final ExecutorService learnerExecutor, final int poolSize,
            final String command) throws InterruptedException {

        final LinkedBlockingQueue<VWMultilabelsLearner> vwPredictorPool = createVWPredictorPool(
                learnerExecutor, poolSize, command);
        return new VWConcurrentMultilabelsPredictor(predictorName, learnerExecutor,
                vwPredictorPool);
    }
    
    /**
     * Create a multilclass multiline predictor that distributes the work of multiclass multiline prediction to backing
     * threadpool specified as executor service. One can use the predictor returned to predict with a timeout.
     * 
     * @param learnerExecutor
     * @param poolSize
     *            - for predictor pool created internally.
     * @param command
     * @param predictNamedLabels
     *            - produce named labels at best effort if set to true
     * @return VWConcurrentMultilabelsPredictor instance.
     * @throws InterruptedException 
     */
    public static VWConcurrentMulticlassMultilinePredictor createMulticlassMultilinePredictorBase(
            final String predictorName,
            final ExecutorService learnerExecutor, final int poolSize,
            final String command,
            final boolean predictNamedLabels) throws InterruptedException {
        final LinkedBlockingQueue<VWMulticlassLearner> vwPredictorPool = createVWPredictorPool(
                learnerExecutor, poolSize, command);
        return new VWConcurrentMulticlassMultilinePredictor(predictorName, learnerExecutor, vwPredictorPool, predictNamedLabels);
    }

    /**
     * Create a predictor pool based on the poolSize. This method waits till all the predictors are
     * initialized and depending on the size of the predictor, this could be matter of few seconds.
     * 
     * @param learnerExecutor
     * @param poolSize
     * @param command
     * @throws InterruptedException 
     */
    private static <P> LinkedBlockingQueue<P> createVWPredictorPool(
            final ExecutorService learnerExecutor, final int poolSize,
            final String command) throws InterruptedException {
        final CompletionService<P> cs = new ExecutorCompletionService<>(
                learnerExecutor);

        final LinkedBlockingQueue<P> vwPredictorPool = new LinkedBlockingQueue<>(
                poolSize);

        // we want to do initialization in parallel as depending on the model
        // size, this could take some time.
        for (int i = 0; i < poolSize; i++) {
            cs.submit(new Callable<P>() {
                @Override
                public P call() throws Exception {
                    return VWLearners.create(command);
                }

            });
        }

        // This loop makes sure we block until learners are initialized in each
        // of the threads.
        for (int i = 0; i < poolSize; i++) {
            try {
                vwPredictorPool.add(cs.take().get());
            } catch (ExecutionException e) {
                throw new RuntimeException(String.format(
                        "Unable to load concurrent learners using "
                                + "command: %s, numThreads: %d",
                        command, poolSize), e);
            }
        }

        return vwPredictorPool;
    }
}