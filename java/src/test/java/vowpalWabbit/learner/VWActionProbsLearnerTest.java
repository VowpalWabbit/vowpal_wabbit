package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;
import vowpalWabbit.responses.ActionProbs;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWActionProbsLearnerTest extends VWTestHelper {
    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Test
    public void testCBExplore() throws IOException {
        String model = temporaryFolder.newFile().getAbsolutePath();
        String[] cbTrain = new String[]{
            "1:2:0.4 | a c",
            "3:0.5:0.2 | b d",
            "4:1.2:0.5 | a b c",
            "2:1:0.3 | b c",
            "3:1.5:0.7 | a d"
        };

        VWActionProbsLearner vw = VWLearners.create("--quiet --cb_explore 4 -f " + model);
        ActionProbs[] trainPreds = new ActionProbs[cbTrain.length];
        for (int i=0; i<cbTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbTrain[i]);
        }
        ActionProbs[] expectedTrainPreds = new ActionProbs[]{
            actionProbs(
                actionProb(0, 0.9625f),
                actionProb(1, 0.0125f),
                actionProb(2, 0.0125f),
                actionProb(3, 0.0125f)
            ),
            actionProbs(
                actionProb(0, 0.0125f),
                actionProb(1, 0.9625f),
                actionProb(2, 0.0125f),
                actionProb(3, 0.0125f)
            ),
            actionProbs(
                actionProb(0, 0.0125f),
                actionProb(1, 0.9625f),
                actionProb(2, 0.0125f),
                actionProb(3, 0.0125f)
            ),
            actionProbs(
                actionProb(0, 0.0125f),
                actionProb(1, 0.9625f),
                actionProb(2, 0.0125f),
                actionProb(3, 0.0125f)
            ),
            actionProbs(
                actionProb(0, 0.0125f),
                actionProb(1, 0.9625f),
                actionProb(2, 0.0125f),
                actionProb(3, 0.0125f)
            )
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create("--quiet -t -i " + model);
        ActionProbs[] testPreds = new ActionProbs[]{vw.predict(cbTrain[0])};

        ActionProbs[] expectedTestPreds = new ActionProbs[]{
            actionProbs(
                actionProb(0, 0.0125f),
                actionProb(1, 0.0125f),
                actionProb(2, 0.9625f),
                actionProb(3, 0.0125f)
            )
        };

        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }

    @Test
    public void testCBADFExplore() throws IOException {
        String[][] cbADFTrain = new String[][]{
            new String[] {"| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"},
            new String[] {"shared | s_1 s_2", "0:1.0:0.5 | a:1 b:1 c:1", "| a:0.5 b:2 c:1"},
            new String[] {"| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"},
            new String[] {"shared | s_1 s_2", "0:1.0:0.5 | a:1 b:1 c:1", "| a:0.5 b:2 c:1"}
        };
        String model = temporaryFolder.newFile().getAbsolutePath();
        VWActionProbsLearner vw = VWLearners.create("--quiet --cb_explore_adf -f " + model);
        ActionProbs[] trainPreds = new ActionProbs[cbADFTrain.length];
        for (int i=0; i<cbADFTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbADFTrain[i]);
        }
        ActionProbs[] expectedTrainPreds = new ActionProbs[]{
                actionProbs(
                    actionProb(0, 0.5f),
                    actionProb(1, 0.5f)
                ),
                actionProbs(
                    actionProb(0, 0.5f),
                    actionProb(1, 0.5f)
                ),
                actionProbs(
                    actionProb(0, 0.97499996f),
                    actionProb(1, 0.025f)
                ),
                actionProbs(
                    actionProb(0, 0.97499996f),
                    actionProb(1, 0.025f)
                )
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create("--quiet -t -i " + model);
        ActionProbs[] testPreds = new ActionProbs[]{vw.predict(cbADFTrain[0])};

        ActionProbs[] expectedTestPreds = new ActionProbs[]{
            actionProbs(
                actionProb(0, 0.97499996f),
                actionProb(1, 0.025f)
            )
        };

        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }
}