package vowpalWabbit.learner;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import vowpalWabbit.VWTestHelper;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScalarLearnerTest extends VWTestHelper {
    private VWScalarLearner houseScorer;

    private VWScalarLearner initHouse(String vwArgs) throws IOException {
        // Since we want this test to continue to work between VW changes, we can't store the model
        // Instead, we'll make a new model for each test
        String houseModel = temporaryFolder.newFile().getAbsolutePath();
        String[] houseData = new String[]{
                "0 | price:.23 sqft:.25 age:.05 2006",
                "1 2 'second_house | price:.18 sqft:.15 age:.35 1976",
                "0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924"};
        VWScalarLearner learner = VWLearners.create(vwArgs + " -f " + houseModel);
        for (String d : houseData) {
            learner.learn(d);
        }
        learner.close();
        return VWLearners.create("--quiet -t -i " + houseModel);
    }

    @Before
    public void setup() throws IOException {
        houseScorer = initHouse(" --quiet --holdout_off");
    }

    @After
    public void cleanup() throws IOException {
        houseScorer.close();
    }

    @Test
    public void testBlank() {
        float prediction = houseScorer.predict("| ");
        assertEquals(0.075, prediction, 0.001);
    }

    @Test
    public void testLine() {
        float prediction1 = houseScorer.predict("| price:0.23 sqft:0.25 age:0.05 2006");
        float prediction2 = houseScorer.predict("| price:0.23 sqft:0.25 age:0.05 2006");
        assertEquals(0.118, prediction1, 0.001);
        assertEquals(0.118, prediction2, 0.001);
    }

    @Test
    public void testLearn() throws IOException {
        VWScalarLearner learner = VWLearners.create("--quiet");
        String heightData = "|f height:0.23 weight:0.25 width:0.05";
        float firstPrediction = learner.learn("0.1 " + heightData);
        float secondPrediction = learner.learn("0.9 " + heightData);
        assertNotEquals(firstPrediction, secondPrediction, 0.001);
        learner.close();
    }

    @Test
    public void testManySamples() throws IOException {
        File model = new File("basic.model");
        model.deleteOnExit();
        VWScalarLearner m = VWLearners.create("--quiet --loss_function logistic --link logistic -f " + model.getAbsolutePath());
        for (int i=0; i<100; ++i) {
            m.learn("-1 | ");
            m.learn("1 | ");
        }
        m.close();

        float expVwOutput = 0.50419676f;
        m = VWLearners.create("--quiet -i " + model.getAbsolutePath());
        assertEquals(expVwOutput, m.predict("| "), 0.0001);
    }

    @Test
    public void twoModelTest() throws IOException {
        VWScalarLearner m1 = VWLearners.create("--quiet");
        VWScalarLearner m2 = VWLearners.create("--quiet");

        float a = m1.predict("-1 | ");
        m1.close();
        float b = m2.predict("-1 | ");
        m2.close();
        assertEquals(a, b, 0.000001);
    }

    @Test
    public void testMultiplePasses() throws IOException {
        String cache = temporaryFolder.newFile().getAbsolutePath();
        VWScalarLearner multiPassModel = initHouse(" --quiet --holdout_off --passes 10 -k --cache_file " + cache);
        try {
            String ex = "| price:0.23 sqft:0.25 age:0.05 2006";
            float singlePassExample = houseScorer.predict(ex);
            float multiPassExample = multiPassModel.predict(ex);
            assertNotEquals(singlePassExample, multiPassExample, 0.1);
        }
        finally {
            multiPassModel.close();
        }
    }
}
