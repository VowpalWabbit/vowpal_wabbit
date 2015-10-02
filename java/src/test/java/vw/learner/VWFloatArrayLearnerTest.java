package vw.learner;

import org.junit.BeforeClass;
import org.junit.Test;
import vw.VWTestHelper;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWFloatArrayLearnerTest {
    private static final int REPS = 100;

    @BeforeClass
    public static void globalSetup() throws IOException {
        VWTestHelper.loadLibrary();
    }

    private static class ModelInfo {
        String model;
        String readableModel;
        public ModelInfo(String model, String readableModel) {
            this.model = model;
            this.readableModel = readableModel;
        }
    };

    final static String[] data = new String[] {
            "| 0 2",
            "| 0 3",
            "| 0 4",
            "| 0 5",
            "| 0 6",
            "| 0 7",
            "| 1 2",
            "| 1 3",
            "| 1 4",
            "| 1 5",
            "| 1 6",
            "| 1 7"
    };

    @Test
    public void testLdaTrainModel() throws Exception {
        trainNewModel();
    }

    @Test
    public void testLdaTrainAndLoadModel() throws Exception {
        assertTrue(rehydratedModelOk(trainNewModel()));
    }

    @Test
    public void testLdaPredict() throws Exception {
        VWFloatArrayLearner v = rehydrateModel(trainNewModel());
        float[] vector = v.predict("| 1:1 2:2 3:3");
        assertNotNull(vector);
    }

    private static ModelInfo trainNewModel() throws Exception {
        ModelInfo mi = new ModelInfo(getModelPath(false), getModelPath(true));
        writeVwModelToDisk(mi);
        return mi;
    }

    private static void writeVwModelToDisk(ModelInfo mi) throws Exception {
        final VWFloatArrayLearner vwModel = new VWFloatArrayLearner(String.format("--quiet -b 3 --lda 2 -f %s --readable_model %s",
                mi.model, mi.readableModel));

        for (int i = 0; i < REPS; ++i) {
            for (String d : data) {
                vwModel.learn(d);
            }
        }

        vwModel.close();
    }

    /**
     * Instantiate model to see if it's possible or whether we get an exception.
     * @param mi path to binary VW model.
     */
    private static boolean rehydratedModelOk(ModelInfo mi) {
        VWFloatArrayLearner v = null;
        try {
            v = rehydrateModel(mi);
            return true;
        } catch (Throwable e) {
            return false;
        } finally {
            try {
                v.close();
            } catch (Throwable ignored) {}
        }
    }

    private static VWFloatArrayLearner rehydrateModel(ModelInfo mi) {
        return new VWFloatArrayLearner("-i " + mi.model + " -t --quiet");
    }

    private static String getModelPath(final boolean readable) throws Exception {
        File f = File.createTempFile("vwtest", (readable ? ".readable" : "") + ".lda.model");
        String p = f.getAbsolutePath();
        f.deleteOnExit();
        f.delete();
        return p;
    }
}
