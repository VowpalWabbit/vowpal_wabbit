package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWMulticlassLearnerTest extends VWTestHelper {
    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Test
    public void testContextualBandits() throws IOException {
        // Note that the expected values in this test were obtained by running
        // vw from the command line as follows
        // echo -e "1:2:0.4 | a c\n3:0.5:0.2 | b d\n4:1.2:0.5 | a b c\n2:1:0.3 | b c\n3:1.5:0.7 | a d" | ../vowpalwabbit/vw --cb 4 -f cb.model -p cb.train.out
        // echo -e "1:2 3:5 4:1:0.6 | a c d\n1:0.5 2:1:0.4 3:2 4:1.5 | c d" | ../vowpalwabbit/vw -i cb.model -t -p cb.out
        String[] train = new String[]{
            "1:2:0.4 | a c",
            "3:0.5:0.2 | b d",
            "4:1.2:0.5 | a b c",
            "2:1:0.3 | b c",
            "3:1.5:0.7 | a d"
        };
        String cbModel = temporaryFolder.newFile().getAbsolutePath();
        VWMulticlassLearner vw = VWLearners.create("--quiet --cb 4 -f " + cbModel);
        int[] trainPreds = new int[train.length];
        for (int i=0; i<train.length; ++i) {
            trainPreds[i] = vw.learn(train[i]);
        }
        int[] expectedTrainPreds = new int[]{1, 2, 2, 2, 2};
        vw.close();

        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create("--quiet -t -i " + cbModel);
        String[] test = new String[]{
            "1:2 3:5 4:1:0.6 | a c d",
            "1:0.5 2:1:0.4 3:2 4:1.5 | c d"
        };

        int[] testPreds = new int[test.length];
        for (int i=0; i<testPreds.length; ++i) {
            testPreds[i] = vw.predict(test[i]);
        }
        int[] expectedTestPreds = new int[]{4, 4};
        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }

    @Test
    public void csoaa() throws IOException {
        // Note that the expected values in this test were obtained by running
        // vw from the command line as follows
        // echo -e "1 | a\n2 | a b\n3 | a c\n2 | a b\n3 | b c\n1 | a c\n2 | d" | ../vowpalwabbit/vw --multilabel_oaa 4 -f multilabel.model -p multilabel.train.out
        // echo -e "| a b c d\n| b d" | ../vowpalwabbit/vw -t -i multilabel.model -p multilabel.test.out
        String[] train = new String[]{
                "1:1.0 a1_expect_1| a",
                "2:1.0 b1_expect_2| b",
                "3:1.0 c1_expect_3| c",
                "1:2.0 2:1.0 ab1_expect_2| a b",
                "2:1.0 3:3.0 bc1_expect_2| b c",
                "1:3.0 3:1.0 ac1_expect_3| a c",
                "2:3.0 d1_expect_2| d"
        };
        String model = temporaryFolder.newFile().getAbsolutePath();
        VWMulticlassLearner vw = VWLearners.create("--quiet --csoaa 3 -f " + model);
        for (String aTrain : train) {
            vw.learn(aTrain);
        }
        vw.close();

        int[] expectedTestPreds = new int[]{1, 2, 3, 2, 2, 3, 2};
        vw = VWLearners.create("--quiet -t -i " + model);
        int[] testPreds = new int[expectedTestPreds.length];
        for (int i=0; i<train.length; ++i) {
            testPreds[i] = vw.predict(train[i]);
        }

        assertArrayEquals(expectedTestPreds, testPreds);
        vw.close();
    }
}
