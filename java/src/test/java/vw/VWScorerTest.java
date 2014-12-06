package vw;

import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScorerTest {
    private static VWScorer scorer;

    @BeforeClass
    public static void setup() {
        scorer = new VWScorer("-i src/test/resources/house.model --quiet -t");
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
