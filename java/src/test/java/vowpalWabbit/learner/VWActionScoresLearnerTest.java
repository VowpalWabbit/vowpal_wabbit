package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;
import vowpalWabbit.responses.ActionScore;
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
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0),
                new ActionScore(1, 0)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.14991696f),
                new ActionScore(1, 0.14991696f)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.27180168f),
                new ActionScore(1, 0.31980497f)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(1, 0.35295868f),
                new ActionScore(0, 0.3869971f)
            })
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);

        vw = VWLearners.create("--quiet -t -i " + model);
        ActionScores[] testPreds = new ActionScores[]{vw.predict(cbADFTrain[0])};

        ActionScores[] expectedTestPreds = new ActionScores[]{
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.33543912f),
                new ActionScore(1, 0.37897447f)
            })
        };

        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds);
    }
}
