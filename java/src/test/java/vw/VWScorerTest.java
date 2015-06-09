package vw;

import org.junit.*;
import org.junit.rules.ExpectedException;

import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.OutputStreamWriter;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScorerTest {
    private static final String houseModel = "house.model";
    private static final String heightData = "|f height:0.23 weight:0.25 width:0.05";
    private static VWScorer houseScorer;

    @Rule
    public ExpectedException thrown = ExpectedException.none();

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
        new File(houseModel).deleteOnExit();
        houseScorer = new VWScorer("--quiet -t -i " + houseModel);
    }

    @AfterClass
    public static void cleanup() {
        houseScorer.close();
    }

    @Test
    @Ignore
    public void load() throws IOException, InterruptedException {
        VWScorer m1 = new VWScorer("--quiet");
        double times = 1e6;
        long jniStart = System.currentTimeMillis();
        for (int i=0; i<times; ++i) {
            m1.doLearnAndGetPrediction(heightData);
        }
        m1.close();
        long jniSeconds = (System.currentTimeMillis() - jniStart) / 1000;

        Process p = Runtime.getRuntime().exec("vowpal_wabbit/vowpalwabbit/vw --quiet");
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(p.getOutputStream()));

        long stdStart = System.currentTimeMillis();
        for (int i=0; i<times; ++i) {
            writer.write(heightData + "\n");
        }
        writer.close();
        long stdSeconds = (System.currentTimeMillis() - stdStart) / 1000;
        p.waitFor();
    }

    @Test
    public void testBlank() {
        float prediction = houseScorer.getPrediction("| ");
        assertEquals(0.075, prediction, 0.001);
    }

    @Test
    public void testLine() {
        float prediction1 = houseScorer.getPrediction("| price:0.23 sqft:0.25 age:0.05 2006");
        float prediction2 = houseScorer.getPrediction("| price:0.23 sqft:0.25 age:0.05 2006");
        assertEquals(0.118, prediction1, 0.001);
        assertEquals(0.118, prediction2, 0.001);
    }

    @Test
    public void testBlankMultiples() {
        String[] input = new String[]{"", ""};
        float[] expected = new float[]{0.075f, 0.075f};
        float[] actual = houseScorer.getPredictions(input);
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
        float[] expected = new float[]{0.118f, 0.118f, 0.527f};
        float[] actual = houseScorer.getPredictions(input);
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
        learner.close();
    }

    @Test
    public void testLearnMultiples() {
        String[] input = new String[]{"0.1 " + heightData, "0.9 " + heightData};
        VWScorer learner = new VWScorer("--quiet");
        float[] predictions = learner.doLearnAndGetPredictions(input);
        assertNotEquals(predictions[0], predictions[1], 0.001);
        learner.close();
    }

    @Test
    public void testBadVWArgs() {
        String args = "--BAD_FEATURE___ounq24tjnasdf8h";
        thrown.expect(Exception.class);
        thrown.expectMessage("unrecognised option '" + args + "'");
        new VWScorer(args + " --quiet");
    }

    @Test
    public void testManySamples() {
        File model = new File("basic.model");
        model.deleteOnExit();
        VWScorer m = new VWScorer("--quiet --loss_function logistic --link logistic -f " + model.getAbsolutePath());
        for (int i=0; i<100; ++i) {
            m.doLearnAndGetPrediction("-1 | ");
            m.doLearnAndGetPrediction("1 | ");
        }
        m.close();

        float expVwOutput = 0.50419676f;
        m = new VWScorer("--quiet -i " + model.getAbsolutePath());
        assertEquals(expVwOutput, m.getPrediction("| "), 0.0001);
    }

    @Test
    public void twoModelTest() {
        VWScorer m1 = new VWScorer("--quiet");
        VWScorer m2 = new VWScorer("--quiet");

        float a = m1.getPrediction("-1 | ");
        m1.close();
        float b = m2.getPrediction("-1 | ");
        m2.close();
        assertEquals(a, b, 0.000001);
    }
}
