package vowpalWabbit.learner;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;
import vowpalWabbit.responses.DecisionScores;

import java.io.IOException;

import static org.junit.Assert.assertEquals;

public class VWCCBLearnerTest extends VWTestHelper {
    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Test
    public void testCCB() throws IOException {
        /*String model = temporaryFolder.newFile().getAbsolutePath();
        String[] examples = new String[]{
                "ccb shared |U country_US id_1",
                "ccb action |A news",
                "ccb action |A sports",
                "ccb slot 1:-1:0.9 | slot_0"
        };

        String cli = "--ccb_explore_adf --cb_type mtr --epsilon 0.01  -f " + model;
        VWCCBLearner vw = VWLearners.create(cli);
        vw.learn(examples);
        vw.close();

        vw = VWLearners.create("-t --ccb_explore_adf --cb_type mtr --epsilon 0.01  -i " + model);
        DecisionScores predict = vw.predict(examples);
        assertEquals(predict.getDecisionScores().length, 1);
        vw.close();*/
    }
}
