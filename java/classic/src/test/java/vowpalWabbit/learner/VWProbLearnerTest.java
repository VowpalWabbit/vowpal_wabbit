package vowpalWabbit.learner;

import org.junit.Test;
import vowpalWabbit.VWTestHelper;

import java.io.IOException;

import static org.junit.Assert.assertArrayEquals;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWProbLearnerTest extends VWTestHelper {

    @Test
    public void prob() throws IOException {
        String[][] data = new String[][]{
            new String[]{"1:1.0 | a_1 b_1 c_1", "2:0.0 | a_2 b_2 c_2", "3:2.0 | a_3 b_3 c_3"},
            new String[]{"1:1.0 | b_1 c_1 d_1", "2:0.0 | b_2 c_2 d_2"},
            new String[]{"1:1.0 | a_1 b_1 c_1", "3:2.0 | a_3 b_3 c_3"}
        };

        VWProbLearner vw = VWLearners.create("--quiet --csoaa_ldf=mc --loss_function=logistic --probabilities");
        float[] pred = new float[data.length];
        for (int i=0; i<data.length; ++i) {
            pred[i] = vw.learn(data[i]);
        }
        vw.close();

        float[] expected = new float[]{0.333333f,
                0.38244691f,
                0.4915067f
        };
        assertArrayEquals(expected, pred, 0.00001f);
    }
}
