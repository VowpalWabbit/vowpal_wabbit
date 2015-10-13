package vw.learner;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vw.VWTestHelper;

import java.io.IOException;

import static org.junit.Assert.assertNotNull;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWFloatArrayLearnerTest {

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @BeforeClass
    public static void globalSetup() throws IOException {
        VWTestHelper.loadLibrary();
    }

    private String model;
    private String readableModel;

    @Before
    public void setupFiles() throws IOException {
        model = temporaryFolder.newFile().getAbsolutePath();
        readableModel = temporaryFolder.newFile().getAbsolutePath();
    }

    private final String[] data = new String[] {
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
    public void testLdaPredict() {
        writeVwModelToDisk();
        VWFloatArrayLearner v = rehydrateModel();
        float[] vector = v.predict("| 1:1 2:2 3:3");
        assertNotNull(vector);
    }

    private void writeVwModelToDisk() {
        final VWFloatArrayLearner vwModel = new VWFloatArrayLearner(String.format("--quiet -b 3 --lda 2 -f %s --readable_model %s",
                model, readableModel));

        for (String d : data) {
            vwModel.learn(d);
        }

        vwModel.close();
    }

    private VWFloatArrayLearner rehydrateModel() {
        return new VWFloatArrayLearner("-i " + model + " -t --quiet");
    }
}
