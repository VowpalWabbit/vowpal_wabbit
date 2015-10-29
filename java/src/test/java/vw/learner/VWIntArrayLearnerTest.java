package vw.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vw.VWTestHelper;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWIntArrayLearnerTest extends VWTestHelper {
    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Test
    public void testMultiLabel() throws IOException {
        // Note that the expected values in this test were obtained by running
        // vw from the command line as follows
        // echo -e "1 | a\n2 | a b\n3 | a c\n2 | a b\n3 | b c\n1 | a c\n2 | d" | ../vowpalwabbit/vw --multilabel_oaa 4 -f multilabel.model -p multilabel.train.out
        // echo -e "| a b c d\n| b d" | ../vowpalwabbit/vw -t -i multilabel.model -p multilabel.test.out
        String[] train = new String[]{
                "1 | a",
                "2 | a b",
                "3 | a c",
                "2 | a b",
                "3 | b c",
                "1 | a c",
                "2 | d"
        };
        String model = temporaryFolder.newFile().getAbsolutePath();
        VWIntArrayLearner vw = VWLearners.create(VWIntArrayLearner.class, "--quiet --multilabel_oaa 4 -f " + model);
        int[][] trainPreds = new int[train.length][];
        for (int i=0; i<train.length; ++i) {
            trainPreds[i] = vw.learn(train[i]);
        }
        int[][] expectedTrainPreds = new int[][]{
                new int[]{},
                new int[]{1},
                new int[]{1},
                new int[]{},
                new int[]{2},
                new int[]{3},
                new int[]{}};
        vw.close();

        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create(VWIntArrayLearner.class, "--quiet -t -i " + model);
        String[] test = new String[]{
                "| a b c d",
                "| b d"
        };

        int[][] testPreds = new int[test.length][1];
        for (int i=0; i<testPreds.length; ++i) {
            testPreds[i] = vw.predict(test[i]);
        }
        int[][] expectedTestPreds = new int[][]{new int[]{}, new int[]{2}};
        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }
}
