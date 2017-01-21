package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;
import vowpalWabbit.responses.Multilabels;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * @author jmorra
 */
public class VWMultilabelsLearnerTest extends VWTestHelper {

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
        VWMultilabelsLearner vw = VWLearners.create("--quiet --multilabel_oaa 4 -f " + model);
        Multilabels[] trainPreds = new Multilabels[train.length];
        for (int i=0; i<train.length; ++i) {
            trainPreds[i] = vw.learn(train[i]);
        }
        Multilabels[] expectedTrainPreds = new Multilabels[]{
                new Multilabels(new int[]{}),
                new Multilabels(new int[]{1}),
                new Multilabels(new int[]{1}),
                new Multilabels(new int[]{}),
                new Multilabels(new int[]{2}),
                new Multilabels(new int[]{3}),
                new Multilabels(new int[]{})
        };
        vw.close();

        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create("--quiet -t -i " + model);
        String[] test = new String[]{
                "| a b c d",
                "| b d"
        };

        Multilabels[] testPreds = new Multilabels[test.length];
        for (int i=0; i<testPreds.length; ++i) {
            testPreds[i] = vw.predict(test[i]);
        }
        Multilabels[] expectedTestPreds = new Multilabels[]{new Multilabels(new int[]{}), new Multilabels(new int[]{2})};
        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }
}
