package vw;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;

/**
 * Created by jmorra on 11/23/14.
 */
public class VWScorer {
    private static final Logger logger = LoggerFactory.getLogger(VWScorer.class);

    static {
        try {
            NativeUtils.loadLibraryFromJar("/vw_jni.lib");
        }
        catch (IOException ioe) {
            logger.error("Cannot load JNI libraries!!");
            logger.error(ioe.getMessage(), ioe);
        }
    }

    public VWScorer(String command){
        initialize(command);
    }

    /**
     * Initialize vw instance with <code>command</code>
     *
     * @param command initialize vs instance with command
     */
    private native void initialize(String command);

    /**
     * Runs prediction on <code>example</code> and returns the prediction output.  Note that
     * this only works for "simple" VW predictions.
     *
     * @param example a single vw example string
     * @return prediction output
     */
    public native float getPrediction(String example);

      /**
     * Runs learning on <code>example</code> and returns the prediction output.  Note that
     * this only works for "simple" VW predictions.
     *
     * @param example a single vw example string
     * @return prediction output
     */
    public native float doLearnAndGetPrediction(String example);
   
    /**
     * Properly shutdown vw instance
     * Made this public and removed finalize call, as finalize is not guaranteed to be called
     * and it caused memory errors to rely on it. Instead explicitly call this method.
     */
    public native void closeInstance();
}
