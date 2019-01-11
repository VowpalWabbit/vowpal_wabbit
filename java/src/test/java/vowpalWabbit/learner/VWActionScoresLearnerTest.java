package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;
import vowpalWabbit.responses.ActionScore;
import vowpalWabbit.responses.ActionScores;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertTrue;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWActionScoresLearnerTest extends VWTestHelper {
    private final float TOLERANCE = 0.001f;

    // https://floating-point-gui.de/errors/comparison/
    private static boolean equalWithTolerance(float a, float b, float epsilon) {
        final float absA = Math.abs(a);
        final float absB = Math.abs(b);
        final float diff = Math.abs(a - b);

        if (a == b) { // shortcut, handles infinities
            return true;
        } else if (a == 0 || b == 0 || diff < Float.MIN_NORMAL) {
            // a or b is zero or both are extremely close to it
            // relative error is less meaningful here
            return diff < (epsilon * Float.MIN_NORMAL);
        } else { // use relative error
            return diff / Math.min((absA + absB), Float.MAX_VALUE) < epsilon;
        }
    }

    private boolean equalWithTolerance(ActionScore a1, ActionScore a2, float tolerance) {
        return (a1.getAction() == a2.getAction())
                && equalWithTolerance(a1.getScore(), a2.getScore(), tolerance);
    }

    private boolean equalWithTolerance(ActionScores a1, ActionScores a2, float tolerance) {
        ActionScore[] a1_scores = a1.getActionScores();
        ActionScore[] a2_scores = a2.getActionScores();

        for (int i = 0; i < a1_scores.length; i++) {
           if(!equalWithTolerance(a1_scores[i], a2_scores[i], tolerance)){
               return false;
           }
        }

        return true;
    }

    private boolean equalWithTolerance(ActionScores[] a1, ActionScores[] a2, float tolerance) {
        for (int i = 0; i < a1.length; i++) {
            if(!equalWithTolerance(a1[i], a2[i], tolerance)) {
                return false;
            }
        }

        return true;
    }

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
        assertTrue(equalWithTolerance(expected, pred, TOLERANCE));
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
        assertTrue(equalWithTolerance(expectedTrainPreds, trainPreds, TOLERANCE));

        vw = VWLearners.create("--quiet -t -i " + model);
        ActionScores[] testPreds = new ActionScores[]{vw.predict(cbADFTrain[0])};

        ActionScores[] expectedTestPreds = new ActionScores[]{
            actionScores(
                actionScore(0, 0.33543912f),
                actionScore(1, 0.37897447f)
            )
        };

        vw.close();
        assertTrue(equalWithTolerance(expectedTestPreds, testPreds, TOLERANCE));
    }
}
