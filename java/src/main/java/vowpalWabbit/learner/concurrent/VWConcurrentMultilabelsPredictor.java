package vowpalWabbit.learner.concurrent;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;

import vowpalWabbit.learner.VWMultilabelsLearner;
import vowpalWabbit.responses.Multilabels;

/**
 * Make multilabel predictions.
 * 
 * @author atulvkamat
 *
 */
public class VWConcurrentMultilabelsPredictor extends
        VWConcurrentAbstractPredictor<VWMultilabelsLearner, Multilabels, String> {

    public VWConcurrentMultilabelsPredictor(
            final String predictorName,
            final ExecutorService learnerExecutor,
            final LinkedBlockingQueue<VWMultilabelsLearner> vwPredictorPool) {
        super(predictorName, learnerExecutor, vwPredictorPool);
    }

    @Override
    protected Multilabels predict(VWMultilabelsLearner predictor, String example) {
        return predictor.predict(example);
    }
}