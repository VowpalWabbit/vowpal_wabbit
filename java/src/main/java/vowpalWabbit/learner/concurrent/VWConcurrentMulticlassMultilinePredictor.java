package vowpalWabbit.learner.concurrent;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;

import vowpalWabbit.learner.VWMulticlassLearner;

/**
 * Concurrent predictor wrapper for multiclass multiline predictions
 * 
 * @author zhilians
 *
 */
public class VWConcurrentMulticlassMultilinePredictor
        extends VWConcurrentAbstractPredictor<VWMulticlassLearner, String[], String[]> {

    private final boolean predictNameLabels;

    /**
     * Default constructor to produce prediction class index strings
     * 
     * @param predictorName
     * @param learnerExecutor
     * @param vwPredictorPool
     */
    public VWConcurrentMulticlassMultilinePredictor(String predictorName, ExecutorService learnerExecutor,
            LinkedBlockingQueue<VWMulticlassLearner> vwPredictorPool) {
        this(predictorName, learnerExecutor, vwPredictorPool, false);
    }

    /**
     * Constructor to produce prediction specifying if named labels or string of indexes should be produced
     * 
     * @param predictorName
     * @param learnerExecutor
     * @param vwPredictorPool
     * @param predictNameLabels
     *            if True, <code>String[] predictMultipleNamedLabels(final String[] example)</code> will be invoked and
     *            named labels will be provided at best effort. See method comment for details.
     *            if False, <code>int[] predictMultiple(final String[] example)</code> will be invoked and results will
     *            be transformed from <code>int[]</code> to <code>String[]</code>
     */
    public VWConcurrentMulticlassMultilinePredictor(String predictorName, ExecutorService learnerExecutor,
            LinkedBlockingQueue<VWMulticlassLearner> vwPredictorPool, boolean predictNameLabels) {
        super(predictorName, learnerExecutor, vwPredictorPool);
        this.predictNameLabels = predictNameLabels;
    }

    @Override
    protected String[] predict(VWMulticlassLearner predictor, String[] example) {
        if (predictNameLabels) {
            return predictor.predictMultipleNamedLabels(example);
        } else {
            int[] predIdx = predictor.predictMultiple(example);
            String[] predStr = new String[predIdx.length];
            for (int i = 0; i < predIdx.length; ++i) {
                predStr[i] = String.valueOf(predIdx[i]);
            }
            return predStr;
        }
    }

}