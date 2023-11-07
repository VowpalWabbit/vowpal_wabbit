package vowpalWabbit.learner;

import org.junit.Before;
import org.junit.Test;
import vowpalWabbit.VWTestHelper;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.junit.Assert.*;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScalarsLearnerTest extends VWTestHelper {
    private String model;
    private String readableModel;

    @Test
    public void probs() throws IOException {
        String[] data = new String[]{
                "1 | a",
                "2 | a b",
                "3 | c d e",
                "2 | b a",
                "1 | f g"
        };

        VWScalarsLearner vw = VWLearners.create("--quiet --oaa 3 --loss_function=logistic --probabilities");
        float[][] pred = new float[data.length][];
        for (int i=0; i<data.length; ++i) {
            pred[i] = vw.learn(data[i]);
        }
        vw.close();

        float[][] expected = new float[][]{
                new float[]{0.333333f, 0.333333f, 0.333333f},
                new float[]{0.475999f, 0.262000f, 0.262000f},
                new float[]{0.374730f, 0.344860f, 0.280410f},
                new float[]{0.365531f, 0.411037f, 0.223432f},
                new float[]{0.341800f, 0.354783f, 0.303417f}
        };
        assertEquals(expected.length, pred.length);
        for (int i=0; i<expected.length; ++i)
            assertArrayEquals(expected[i], pred[i], 0.001f);
    }

    @Before
    public void setupFiles() throws IOException {
        model = temporaryFolder.newFile().getAbsolutePath();
        readableModel = temporaryFolder.newFile().getAbsolutePath();
    }

    private static Map<String, Integer> createDictionaryFromDocuments(String[] documents) {
        Map<String, Integer> dict = new HashMap<String, Integer>();
        Integer count = 0;
        for(String doc : documents) {
            for(String w : doc.toLowerCase().split(" ")) {
                if (!dict.containsKey(w)) {
                    dict.put(w, count++);
                }
            }
        }
        return dict;
    }

    private static String[] documentsToTrainingData(Map<String, Integer> dict, String[] documents) {
        List<String> docs = new ArrayList<String>();
        for(String doc : documents) {
            StringBuilder sb = new StringBuilder("| ");
            for(String w : doc.toLowerCase().split(" ")) {
                sb.append(dict.get(w)).append(" ");
            }
            docs.add(sb.toString().trim());
        }
        return docs.toArray(new String[]{});
    }

    private final String[] trainingDocuments = new String[] {
            "printf sizeof char",
            "eof printlf argc std",
            "scanf std cout len",
            "img div width",
            "png color img",
            "0px jpg img",
            "good since say better",
            "wondering we look since",
            "computer really say we"
    };
    private final Map<String, Integer> dictionary = createDictionaryFromDocuments(trainingDocuments);
    private final String[] data = documentsToTrainingData(dictionary, trainingDocuments);

    private String convertQuery(String q) {
        String[] s = q.toLowerCase().split(" ");
        StringBuilder sb = new StringBuilder("| ");
        for(int i = 1; i < s.length; i++) {
            String[] w = s[i].split(":");
            sb.append(dictionary.get(w[0]));
            if (w.length == 2) {
                sb.append(":").append(w[1]);
            }
            sb.append(" ");
        }
        return sb.toString().trim();
    }

    @Test
    public void testLDALearnerPredict() throws IOException {
        writeVwModelToDisk();
        VWScalarsLearner v = rehydrateModel();
        float[] vector = v.predict(convertQuery("| wondering we look since"));
        assertNotNull(vector);
        assertEquals(3, vector.length);
    }

    @Test
    public void csoaaLdf() throws IOException {
        String[][] data = new String[][]{
            new String[]{"1:1.0 | a_1 b_1 c_1", "2:0.0 | a_2 b_2 c_2", "3:2.0 | a_3 b_3 c_3"},
            new String[]{"1:1.0 | b_1 c_1 d_1", "2:0.0 | b_2 c_2 d_2"},
            new String[]{"1:1.0 | a_1 b_1 c_1", "3:2.0 | a_3 b_3 c_3"}
        };

        VWScalarsLearner vw = VWLearners.create("--quiet --csoaa_ldf=mc --loss_function=logistic --probabilities");
        float[][] pred = new float[data.length][];
        for (int i=0; i<data.length; ++i) {
            pred[i] = vw.learn(data[i]);
        }
        vw.close();

        float[][] expected = new float[][]{
                new float[]{0.333333f, 0.33333334f, 0.33333334f},
                new float[]{0.38244691f, 0.61757493f},
                new float[]{0.4915067f, 0.50853986f},
        };
        assertEquals(expected.length, pred.length);
        for (int i=0; i<expected.length; ++i)
            assertArrayEquals(expected[i], pred[i], 0.001f);
    }

    private void writeVwModelToDisk() throws IOException {
        final VWScalarsLearner vwModel =  VWLearners.create(String.format("--quiet -b 4 --lda 3 -f %s --readable_model %s",
                model, readableModel));

        for (String d : data) {
            vwModel.learn(d);
        }

        vwModel.close();
    }

    private VWScalarsLearner rehydrateModel() {
        return VWLearners.create("-i " + model + " -t --quiet");
    }
}
