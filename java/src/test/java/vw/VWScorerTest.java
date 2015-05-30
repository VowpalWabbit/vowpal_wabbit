package vw;

import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScorerTest {
    private static final String houseModel = "target/house.model";
    private static final String heightData = "|f height:0.23 weight:0.25 width:0.05";

    @BeforeClass
    public static void setup() {
        // Since we want this test to continue to work between VW changes, we can't store the model
        // Instead, we'll make a new model for each test
        String[] houseData = new String[]{
                "0 | price:.23 sqft:.25 age:.05 2006",
                "1 2 'second_house | price:.18 sqft:.15 age:.35 1976",
                "0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924"};
        VWScorer learner = new VWScorer(" --quiet -f " + houseModel);
        learner.doLearnAndGetPredictions(houseData);
        learner.close();
    }

    @Test
    public void testBlank() {
        VWScorer scorer = new VWScorer("--quiet -t -i " + houseModel);
        float prediction = scorer.getPrediction("| ");
        assertEquals(0.075, prediction, 0.001);
    }

    @Test
    public void testLine() {
        VWScorer scorer = new VWScorer("--quiet -t -i " + houseModel);
        float prediction1 = scorer.getPrediction("| price:0.23 sqft:0.25 age:0.05 2006");
        float prediction2 = scorer.getPrediction("| price:0.23 sqft:0.25 age:0.05 2006");
        assertEquals(0.118, prediction1, 0.001);
        assertEquals(0.118, prediction2, 0.001);
    }

    @Test
    public void testBlankMultiples() {
        String[] input = new String[]{"", ""};
        VWScorer scorer = new VWScorer("--quiet -t -i " + houseModel);
        float[] expected = new float[]{0.075f, 0.075f};
        float[] actual = scorer.getPredictions(input);
        assertEquals(expected.length, actual.length);
        for (int i=0; i<expected.length; ++i) {
            assertEquals(expected[i], actual[i], 0.001);
        }
    }

    @Test
    public void testPredictionMultiples() {
        String[] input = new String[]{
                "| price:0.23 sqft:0.25 age:0.05 2006",
                "| price:0.23 sqft:0.25 age:0.05 2006",
                "0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924"
        };
        VWScorer scorer = new VWScorer("--quiet -t -i " + houseModel);
        float[] expected = new float[]{0.118f, 0.118f, 0.527f};
        float[] actual = scorer.getPredictions(input);
        assertEquals(expected.length, actual.length);
        for (int i=0; i<expected.length; ++i) {
            assertEquals(expected[i], actual[i], 0.001);
        }
    }

    @Test
    public void testLearn() {
        VWScorer learner = new VWScorer("--quiet");
        float firstPrediction = learner.doLearnAndGetPrediction("0.1 " + heightData);    
        float secondPrediction = learner.doLearnAndGetPrediction("0.9 " + heightData);
        assertNotEquals(firstPrediction, secondPrediction, 0.001);
    }

    @Test
    public void testLearnMultiples() {
        String[] input = new String[]{"0.1 " + heightData, "0.9 " + heightData};
        VWScorer learner = new VWScorer("--quiet");
        float[] predictions = learner.doLearnAndGetPredictions(input);
        assertNotEquals(predictions[0], predictions[1], 0.001);
    }
}
