package vw.learner;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vw.VWTestHelper;

import java.io.IOException;
import java.util.*;

import static org.junit.Assert.*;

/**
 * @author jmorra
 * @author marko asplund
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
        final VWFloatArrayLearner vwModel =  VWFactory.getVWLeaner(String.format("--quiet -b 4 --lda 3 -f %s --readable_model %s",
                model, readableModel));

        for (String d : data) {
            vwModel.learn(d);
        }

        vwModel.close();
    }

    private VWFloatArrayLearner rehydrateModel() {
        return VWFactory.getVWLeaner("-i " + model + " -t --quiet");
    }
}
