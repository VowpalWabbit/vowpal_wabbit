package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

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

    @Test
    public void rawPredictions() throws IOException {
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
        float[] testPreds = new float[expectedTestPreds.length];
        for (int i=0; i<train.length; ++i) {
            Object f = vw.rawPredict(train[i]);
            System.out.println();
        }

        //assertArrayEquals(expectedTestPreds, testPreds);
        vw.close();
    }

    @Test
    public void testRawPredictions() throws IOException {
        VWMulticlassLearner vw_multi = VWLearners.create("--quiet -i /home/matt/workspace/activeFAQ/vowpal/multiclass.model -r /dev/stdout");
        String[] testData = new String[]{
            "product shipping return_and_exchange order payment settings conversation | After my first pair, can I change my trunk subscription to another color other than Parisian Blue or the Color of the Month?",
            "product shipping return_and_exchange order payment settings conversation | I ordered a pair of the French Terry sweatpants, and when they arrived, there were several small holes along the center seam. (I can send pictures if needed) I was wondering if I could send this pair back and receive a new pair. My order number is R871655120. Thank you",
            "product shipping return_and_exchange order payment settings conversation | When will my package arrive?",
            "product shipping return_and_exchange order payment settings conversation | Hi! I've lost the 20% off code for my first order. Can you help? I don't want to purchase without it. Thanks!",
            "product shipping return_and_exchange order payment settings conversation | I need to change the address for my subscription"
        };

        float[] expectedTestPreds1 = new float[]{0.755454F, 1.45879F, 0.721972F, 1.01461F, 1.41322F, 0.642049F, 1.87484F};
        float[] expectedTestPreds2 = new float[]{0.982791F, 1.66577F, -0.146446F, 1.29745F, 1.54971F, 1.25542F, 2.86067F};
        float[] expectedTestPreds3 = new float[]{0.922538F,0.920442F, 0.761036F, 0.480921F, 0.89217F, 0.867562F, 1.0385F};
        float[] expectedTestPreds4 = new float[]{1.03581F, 1.01326F, 1.34347F, 1.3972F, 0.542562F, 0.72871F, 1.23015F};
        float[] expectedTestPreds5 = new float[]{1.10763F, 1.09224F, 0.6842F, 0.717893F, 1.17313F, 0.236427F, 1.50063F};
        float[] testPreds1 = vw_multi.rawPredict(testData[0]);
        float[] testPreds2 = vw_multi.rawPredict(testData[1]);
        float[] testPreds3 = vw_multi.rawPredict(testData[2]);
        float[] testPreds4 = vw_multi.rawPredict(testData[3]);
        float[] testPreds5 = vw_multi.rawPredict(testData[4]);

        vw_multi.close();

        assertArrayEquals(expectedTestPreds1, testPreds1, 0.0001F);
        assertArrayEquals(expectedTestPreds2, testPreds2, 0.0001F);
        assertArrayEquals(expectedTestPreds3, testPreds3, 0.0001F);
        assertArrayEquals(expectedTestPreds4, testPreds4, 0.0001F);
        assertArrayEquals(expectedTestPreds5, testPreds5, 0.0001F);


    }
}
