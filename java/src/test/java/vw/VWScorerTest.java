package vw;

import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;

import static org.junit.Assert.assertEquals;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScorerTest {
    private static VWScorer scorer;

    @BeforeClass
    public static void setup() throws IOException, InterruptedException {
        // Since we want this test to continue to work between VW changes, we can't store the model
        // Instead, we'll make a new model for each test
        String vwModel = "target/house.model";
        Runtime.getRuntime().exec("../vowpalwabbit/vw -d src/test/resources/house.vw -f " + vwModel).waitFor();
        scorer = new VWScorer("--quiet -t -i " + vwModel);
    }

    @Test
    public void testBlank() {
        float prediction = scorer.getPrediction("| ");
        assertEquals(0.07496, prediction, 0.00001);
    }

    @Test
    public void testLine() {
        float prediction = scorer.getPrediction("| price:0.23 sqft:0.25 age:0.05 2006");
        assertEquals(0.118467, prediction, 0.00001);
    }
}
