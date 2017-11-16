package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;
import vowpalWabbit.responses.ActionScores;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWActionScoresLearnerTest extends VWTestHelper {
    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Test
    public void testCSOAA() throws IOException {
        String[][] data = new String[][]{
                new String[]{
                        "1:1.0 | a_1 b_1 c_1",
                        "2:0.0 | a_2 b_2 c_2",
                        "3:2.0 | a_3 b_3 c_3"
                },
                new String[]{
                        "1:1.0 | b_1 c_1 d_1",
                        "2:0.0 | b_2 c_2 d_2"
                },
                new String[]{
                        "1:1.0 | a_1 b_1 c_1",
                        "3:2.0 | a_3 b_3 c_3"
                }
        };

        VWActionScoresLearner vw = VWLearners.create("--csoaa_ldf mc --quiet --csoaa_rank");

        ActionScores[] pred = new ActionScores[data.length];
        for (int j=0; j< 100; ++j) {
            for (int i=0; i<data.length; ++i) {
                pred[i] = vw.learn(data[i]);
            }
        }

        vw.close();

        ActionScores[] expected = new ActionScores[]{
                actionScores(
                        actionScore(1, -1.0573887f),
                        actionScore(0, -0.033036415f),
                        actionScore(2, 1.0063205f)
                ),
                actionScores(
                        actionScore(1, -1.0342788f),
                        actionScore(0, 0.9994181f)
                ),
                actionScores(
                        actionScore(0, 0.033397526f),
                        actionScore(1, 1.0227613f)
                )
        };
        assertArrayEquals(expected, pred);
    }

    @Test
    public void testCBADF() throws IOException {
        testCBADF(false);
    }

    @Test
    public void testCBADFWithRank() throws IOException {
        testCBADF(true);
    }

    private void testCBADF(boolean withRank) throws IOException {
        String[][] cbADFTrain = new String[][]{
            new String[]{"| a:1 b:0.5","0:0.1:0.75 | a:0.5 b:1 c:2"},
            new String[]{"shared | s_1 s_2","0:1.0:0.5 | a:1 b:1 c:1","| a:0.5 b:2 c:1"},
            new String[]{"| a:1 b:0.5","0:0.1:0.75 | a:0.5 b:1 c:2"},
            new String[]{"shared | s_1 s_2","0:1.0:0.5 | a:1 b:1 c:1","| a:0.5 b:2 c:1"}
        };
        String model = temporaryFolder.newFile().getAbsolutePath();
        String cli = "--quiet --cb_adf -f " + model;
        if (withRank)
            cli += " --rank_all";
        VWActionScoresLearner vw = VWLearners.create(cli);
        ActionScores[] trainPreds = new ActionScores[cbADFTrain.length];
        for (int i=0; i<cbADFTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbADFTrain[i]);
        }

        ActionScores[] expectedTrainPreds = new ActionScores[]{
            actionScores(
                actionScore(0, 0),
                actionScore(1, 0)
            ),
            actionScores(
                actionScore(0, 0.14991696f),
                actionScore(1, 0.14991696f)
            ),
            actionScores(
                actionScore(0, 0.27180168f),
                actionScore(1, 0.31980497f)
            ),
            actionScores(
                actionScore(1, 0.35295868f),
                actionScore(0, 0.3869971f)
            )
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create("--quiet -t -i " + model);
        ActionScores[] testPreds = new ActionScores[]{vw.predict(cbADFTrain[0])};

        ActionScores[] expectedTestPreds = new ActionScores[]{
            actionScores(
                actionScore(0, 0.33543912f),
                actionScore(1, 0.37897447f)
            )
        };

        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }
}
