package vw;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.Closeable;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by jmorra on 11/23/14.
 */
public class VWScorer implements Closeable {
    private static final Logger logger = LoggerFactory.getLogger(VWScorer.class);

    static {
        try {
            NativeUtils.loadOSDependentLibrary("/vw_jni", ".lib");
        }
        catch (IOException ioe) {
            logger.error("Cannot load JNI libraries!!");
            logger.error(ioe.getMessage(), ioe);
        }
    }

    private final AtomicBoolean isClosed;
    private final long nativePointer;

    public VWScorer(String command) {
        isClosed = new AtomicBoolean(false);
        nativePointer = initialize(command);
    }

    /**
     * Runs prediction on <code>example</code> and returns the prediction output.  Note that
     * this only works for "simple" VW predictions.
     *
     * @param example a single vw example string
     * @return prediction output
     */
    public float getPrediction(String example) {
        return getPrediction(example, false, nativePointer);
    }

    /**
     * Runs learning on <code>example</code> and returns the prediction output.  Note that
     * this only works for "simple" VW predictions.
     *
     * @param example a single vw example string
     * @return prediction output
     */
    public float doLearnAndGetPrediction(String example) {
        return getPrediction(example, true, nativePointer);
    }

    /**
     * Run prediction on <code>examples</code> and returns all of the predictions.  Note that
     * this only works for "simple" VW predictions.
     * @param examples an array of vw example strings
     * @return predictions
     */
    public float[] getPredictions(String[] examples) {
        return getPredictions(examples, false, nativePointer);
    }

    /**
     * Run learning on <code>examples</code> and returns all of the predictions.  Note that
     * this only works for "simple" VW predictions.
     * @param examples an array of vw example strings
     * @return predictions
     */
    public float[] doLearnAndGetPredictions(String[] examples) {
        return getPredictions(examples, true, nativePointer);
    }

    public void close() {
        if (!isClosed.getAndSet(true)) {
            logger.info("Shutting down VW");
            closeInstance(nativePointer);
        }
    }

    private native long initialize(String command);
    private native float getPrediction(String example, boolean learn, long nativePointer);
    private native float[] getPredictions(String[] examples, boolean learn, long nativePointer);
    private native void closeInstance(long nativePointer);
}
