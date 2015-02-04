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

    private boolean isClosed;

    public VWScorer(String command){
        isClosed = false;
        initialize(command);
    }

    /**
     * Probably the only acceptable use of a finalizer.  Because this class
     * uses native code, we have to manually clean up the native code when this
     * object is garbage collected.
     */
    @Override
    public void finalize() {
        if (!isClosed) {
            closeInstance();
            isClosed = true;
        }
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
     * Properly shutdown vw instance
     */
    private native void closeInstance();
}
