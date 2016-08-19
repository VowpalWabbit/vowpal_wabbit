package vw.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vw.VWTestHelper;
import vw.responses.ActionScore;
import vw.responses.ActionScores;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWActionScoresLearnerTest extends VWTestHelper {
    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    private String[][] cbADFTrain = new String[][]{
            new String[] {"| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"},
            new String[] {"shared | s_1 s_2", "0:1.0:0.5 | a:1 b:1 c:1", "| a:0.5 b:2 c:1"},
            new String[] {"| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"},
            new String[] {"shared | s_1 s_2", "0:1.0:0.5 | a:1 b:1 c:1", "| a:0.5 b:2 c:1"}
    };

    @Test
    public void testCBExplore() throws IOException {
        String[] cbTrain = new String[]{
                "1:2:0.4 | a c",
                "3:0.5:0.2 | b d",
                "4:1.2:0.5 | a b c",
                "2:1:0.3 | b c",
                "3:1.5:0.7 | a d"
        };

        VWActionScoresLearner vw = VWLearners.create("--quiet --cb_explore 4");
        ActionScores[] trainPreds = new ActionScores[cbTrain.length];
        for (int i=0; i<cbTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbTrain[i]);
        }
        ActionScores[] expectedTrainPreds = new ActionScores[]{
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.9625f),
                new ActionScore(1, 0.0125f),
                new ActionScore(2, 0.0125f),
                new ActionScore(3, 0.0125f)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.0125f),
                new ActionScore(1, 0.9625f),
                new ActionScore(2, 0.0125f),
                new ActionScore(3, 0.0125f)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.0125f),
                new ActionScore(1, 0.9625f),
                new ActionScore(2, 0.0125f),
                new ActionScore(3, 0.0125f)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.0125f),
                new ActionScore(1, 0.9625f),
                new ActionScore(2, 0.0125f),
                new ActionScore(3, 0.0125f)
            }),
            new ActionScores(new ActionScore[]{
                new ActionScore(0, 0.0125f),
                new ActionScore(1, 0.9625f),
                new ActionScore(2, 0.0125f),
                new ActionScore(3, 0.0125f)
            })
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);
    }

    @Test
    public void testCBADFWithRank() throws IOException {
        VWActionScoresLearner vw = VWLearners.create("--quiet --cb_adf --rank_all");
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
    }

    @Test
    public void testCBADFExplore() throws IOException {
        VWActionScoresLearner vw = VWLearners.create("--quiet --cb_explore_adf");
        ActionScores[] trainPreds = new ActionScores[cbADFTrain.length];
        for (int i=0; i<cbADFTrain.length; ++i) {
            trainPreds[i] = vw.learn(cbADFTrain[i]);
        }
        ActionScores[] expectedTrainPreds = new ActionScores[]{
                new ActionScores(new ActionScore[]{
                    new ActionScore(0, 0.97499996f),
                    new ActionScore(1, 0.025f)
                }),
                new ActionScores(new ActionScore[]{
                    new ActionScore(0, 0.97499996f),
                    new ActionScore(1, 0.025f)
                }),
                new ActionScores(new ActionScore[]{
                    new ActionScore(0, 0.97499996f),
                    new ActionScore(1, 0.025f)
                }),
                new ActionScores(new ActionScore[]{
                    new ActionScore(1, 0.97499996f),
                    new ActionScore(0, 0.025f)
                })
        };
        vw.close();
        assertArrayEquals(expectedTrainPreds, trainPreds);
    }

    @Test
    public void testCBADF() throws IOException {
        VWActionScoresLearner vw = VWLearners.create("--quiet --cb_adf");
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
    }
}
