package vw.learner;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vw.VWTestHelper;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author jmorra
 * @author marko asplund
 */
public class VWFloatArrayLearnerTest extends VWTestHelper {

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    private String model;
    private String readableModel;

    private String[][] cbADFTrain = new String[][]{
        new String[] {"| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"},
        new String[] {"shared | s_1 s_2", "0:1.0:0.5 | a:1 b:1 c:1", "| a:0.5 b:2 c:1"},
        new String[] {"| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"},
        new String[] {"shared | s_1 s_2", "0:1.0:0.5 | a:1 b:1 c:1", "| a:0.5 b:2 c:1"}
    };

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
          sb.append(dict.get(w) + " ");
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
          sb.append(":"+w[1]);
        }
        sb.append(" ");
      }
      return sb.toString().trim();
    }

    @Test
    public void testFloatArrayLearnerPredict() {
        writeVwModelToDisk();
        VWFloatArrayLearner v = rehydrateModel();
        float[] vector = v.predict(convertQuery("| wondering we look since"));
        assertNotNull(vector);
        assertEquals(3, vector.length);
    }

    private void writeVwModelToDisk() {
        final VWFloatArrayLearner vwModel =  VWLearners.create(String.format("--quiet -b 4 --lda 3 -f %s --readable_model %s",
                model, readableModel));

        for (String d : data) {
            vwModel.learn(d);
        }

        vwModel.close();
    }

    private VWFloatArrayLearner rehydrateModel() {
        return VWLearners.create("-i " + model + " -t --quiet");
    }

    @Test
    public void testCBExplore() throws IOException {
        String[] cbTrain = new String[]{
            "1:2:0.4 | a c",
            "3:0.5:0.2 | b d",
            "4:1.2:0.5 | a b c",
            "2:1:0.3 | b c",
            "3:1.5:0.7 | a d"
        };

        VWFloatArrayLearner vw = VWLearners.create("--quiet --cb_explore 4");
        float[][] trainPreds = new float[cbTrain.length][];
        for (int i=0; i<cbTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbTrain[i]);
        }
        float[][] expectedTrainPreds = new float[][]{
            new float[]{0.962500f, 0.012500f, 0.012500f, 0.012500f},
            new float[]{0.012500f, 0.962500f, 0.012500f, 0.012500f},
            new float[]{0.012500f, 0.962500f, 0.012500f, 0.012500f},
            new float[]{0.012500f, 0.962500f, 0.012500f, 0.012500f},
            new float[]{0.012500f, 0.962500f, 0.012500f, 0.012500f},
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);
    }

    @Test
    public void testCBADFWithRank() throws IOException {
        VWFloatArrayLearner vw = VWLearners.create("--quiet --cb_adf --rank_all");
        float[][] trainPreds = new float[cbADFTrain.length][];
        for (int i=0; i<cbADFTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbADFTrain[i]);
        }
        float[][] expectedTrainPreds = new float[][]{
            new float[]{0, 0},
            new float[]{0.14991696f, 0.14991696f},
            new float[]{0.27180168f, 0.31980497f},
            new float[]{0.3869971f, 0.35295868f}
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);
    }

    @Test
    public void testCBADFExplore() throws IOException {
        VWFloatArrayLearner vw = VWLearners.create("--quiet --cb_explore_adf");
        float[][] trainPreds = new float[cbADFTrain.length][];
        for (int i=0; i<cbADFTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbADFTrain[i]);
        }
        float[][] expectedTrainPreds = new float[][]{
            new float[]{0.97499996f, 0.025f},
            new float[]{0.97499996f, 0.025f},
            new float[]{0.97499996f, 0.025f},
            new float[]{0.025f, 0.97499996f}
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);
    }
}
