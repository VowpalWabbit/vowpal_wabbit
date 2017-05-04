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

    @Test
    public void searn() throws IOException {
        /**
         * Equivalent to:
         * 
         * echo -e
         * "1  |w a\n2  |w b\n3  |w c\n3  |w d\n\n2  |w e\n\n1  |w f\n1  |w g\n1  |w h\n\n1  |w i\n1  |w j\n\n2  |w k\n3  |w l\n\n1  |w m\n1  |w n\n1  |w o\n2  |w p\n\n1  |w i\n2  |w k\n3  |w l\n3  |w d"
         * | bin/vw --quiet -b 24 -c --search_task sequence --search 45
         * --search_neighbor_features -1:w,1:w --affix -1w,+1w -f
         * toy_searn.model
         * 
         * echo -e "  |w i\n  |w j\n  |w k\n\n" | bin/vw --quiet -t -i toy_searn.model -p stdout
         */
        String[][] train = new String[][] {
                new String[]{"1  |w a", "2  |w b", "3  |w c", "3  |w d"},
                new String[]{"2  |w e"},
                new String[]{"1  |w f", "1  |w g", "1  |w h"},
                new String[]{"1  |w i", "1  |w j"},
                new String[]{"2  |w k", "3  |w l"},
                new String[]{"1  |w m", "1  |w n", "1  |w o", "2  |w p"},
                new String[]{"1  |w i", "2  |w k", "3  |w l", "3  |w d"}
        };
        String model = temporaryFolder.newFile().getAbsolutePath();
        VWMulticlassLearner vw = VWLearners
                .create("--quiet -b 24 -c --search_task sequence --search 45 --search_neighbor_features -1:w,1:w --affix -1w,+1w -f "
                        + model);
        for (String[] aTrain : train) {
            vw.learn(aTrain);
        }
        vw.close();
        // expecting int index value of the classes if predictMultiple was called
        int[] expectedTestPreds = new int[]{1, 1, 2};
        // expecting string of int index value of the classes if predictMultipleNamedLabels was called, without name labels provided in the model
        String[] expectedTestPredStrs = new String[]{"1", "1", "2"};
        // testing examples
        String[] testInput = new String[]{"  |w i", "  |w j", "  |w k"};
        vw = VWLearners.create("--quiet -t -i " + model + " -p stdout");
        int[] testPreds = vw.predictMultiple(testInput);
        String[] testPredStrs = vw.predictMultipleNamedLabels(testInput);
        assertArrayEquals(expectedTestPreds, testPreds);
        assertArrayEquals(expectedTestPredStrs, testPredStrs);
    }

    @Test
    public void searnWithNamedLabels() throws IOException {
        /**
         * Equivalent to:
         * 
         * echo -e
         * "x  |w a\ny  |w b\nz  |w c\nz  |w d\n\ny  |w e\n\nx  |w f\nx  |w g\nx  |w h\n\nx  |w i\nx  |w j\n\ny  |w k\nz  |w l\n\nx  |w m\nx  |w n\nx  |w o\ny  |w p\n\nx  |w i\ny  |w k\nz  |w l\nz  |w d"
         * | bin/vw --quiet -b 24 -c --search_task sequence --search 45 --named_labels x,y,z 
         * --search_neighbor_features -1:w,1:w --affix -1w,+1w -f
         * toy_searn.model
         * 
         * echo -e "  |w i\n  |w j\n  |w k\n\n" | bin/vw --quiet -t -i toy_searn.model -p stdout
         */
        String[][] train = new String[][] {
                 new String[]{"x  |w a", "y  |w b", "z  |w c", "z  |w d"},
                 new String[]{"y  |w e"},
                 new String[]{"x  |w f", "x  |w g", "x  |w h"},
                 new String[]{"x  |w i", "x  |w j"},
                 new String[]{"y  |w k", "z  |w l"},
                 new String[]{"x  |w m", "x  |w n", "x  |w o", "y  |w p"},
                 new String[]{"x  |w i", "y  |w k", "z  |w l", "z  |w d"}
         };
         String model = temporaryFolder.newFile().getAbsolutePath();
         VWMulticlassLearner vw = VWLearners
                 .create("--quiet -b 24 -c --search_task sequence --search 45 --search_neighbor_features -1:w,1:w --affix -1w,+1w --named_labels x,y,z -f "
                         + model);
         for (String[] aTrain : train) {
             vw.learn(aTrain);
         }
         vw.close();
         // expecting the named labels of the classes if predictMultipleNamedLabels was called and if name labels were provided in the model
         String[] expectedTestPreds = new String[]{"x", "x", "y"};
         String[] testInput = new String[]{"  |w i", "  |w j", "  |w k"};
         vw = VWLearners.create("--quiet -t -i " + model + " -p stdout");
         String[] testPreds = vw.predictMultipleNamedLabels(testInput);
         assertArrayEquals(expectedTestPreds, testPreds);
    }
}
