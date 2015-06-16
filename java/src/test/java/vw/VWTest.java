package vw;

import org.junit.*;
import org.junit.rules.ExpectedException;
import org.junit.rules.TemporaryFolder;
import vw.exception.IllegalVWInput;

import java.io.*;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.*;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWTest {
    private String houseModel;
    private final String heightData = "|f height:0.23 weight:0.25 width:0.05";
    private VW houseScorer;

    @Rule
    public ExpectedException thrown = ExpectedException.none();

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Before
    public void setup() throws IOException {
        // Since we want this test to continue to work between VW changes, we can't store the model
        // Instead, we'll make a new model for each test
        houseModel = temporaryFolder.newFile().getAbsolutePath();
        String[] houseData = new String[]{
                "0 | price:.23 sqft:.25 age:.05 2006",
                "1 2 'second_house | price:.18 sqft:.15 age:.35 1976",
                "0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924"};
        VW learner = new VW(" --quiet -f " + houseModel);
        for (String d : houseData) {
            learner.learn(d);
        }
        learner.close();
        houseScorer = new VW("--quiet -t -i " + houseModel);
    }

    @After
    public void cleanup() {
        houseScorer.close();
    }

    private long streamingLoadTest(int times) {
        VW m1 = new VW("--quiet");
        long start = System.currentTimeMillis();
        for (int i=0; i<times; ++i) {
            // This will force a new string to be created every time for a fair test
            m1.learn(heightData + "");
        }
        m1.close();
        return System.currentTimeMillis() - start;
    }

    private long stdLoadTest(int times) throws IOException, InterruptedException {
        Process p = Runtime.getRuntime().exec("../vowpalwabbit/vw --quiet");
        PrintWriter writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(p.getOutputStream())));

        long start = System.currentTimeMillis();
        for (int i=0; i<times; ++i) {
            writer.println(heightData);
        }
        writer.close();
        p.waitFor();
        return System.currentTimeMillis() - start;
    }

    @Test
    @Ignore
    public void loadTest() throws IOException, InterruptedException {
        int times = (int)1e6;

        System.out.println("Milliseconds for JNI layer: " + streamingLoadTest(times));
        System.out.println("Milliseconds for external process: " + stdLoadTest(times));
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
    public void testLearn() {
        VW learner = new VW("--quiet");
        float firstPrediction = learner.learn("0.1 " + heightData);
        float secondPrediction = learner.learn("0.9 " + heightData);
        assertNotEquals(firstPrediction, secondPrediction, 0.001);
        learner.close();
    }

    @Test
    public void testBadVWArgs() {
        final String args = "--BAD_FEATURE___ounq24tjnasdf8h";
        thrown.expect(IllegalArgumentException.class);
        new VW(args + " --quiet");
    }

    @Test
    public void testManySamples() {
        File model = new File("basic.model");
        model.deleteOnExit();
        VW m = new VW("--quiet --loss_function logistic --link logistic -f " + model.getAbsolutePath());
        for (int i=0; i<100; ++i) {
            m.learn("-1 | ");
            m.learn("1 | ");
        }
        m.close();

        float expVwOutput = 0.50419676f;
        m = new VW("--quiet -i " + model.getAbsolutePath());
        assertEquals(expVwOutput, m.predict("| "), 0.0001);
    }

    @Test
    public void twoModelTest() {
        VW m1 = new VW("--quiet");
        VW m2 = new VW("--quiet");

        float a = m1.predict("-1 | ");
        m1.close();
        float b = m2.predict("-1 | ");
        m2.close();
        assertEquals(a, b, 0.000001);
    }

    @Test
    public void testAlreadyClosed() {
        thrown.expect(IllegalStateException.class);
        thrown.expectMessage("Already closed.");
        VW s = new VW("--quiet");
        s.close();
        s.predict("1 | ");
    }

    @Test
    @Ignore
    public void testBadModel() {
        // Right now VW seg faults on a bad model.  Ideally we should throw an exception
        // that the Java layer could do something about
        thrown.expect(Exception.class);
        thrown.expectMessage("bad VW model");
        VW vw = new VW("--quiet -i src/test/resources/vw_bad.model");
        vw.close();
    }

    @Test
    public void testContextualBandits() throws IOException {
        // Note that the expected values in this test were obtained by running
        // vw from the command line as follows
        // echo -e "1:2:0.4 | a c\n3:0.5:0.2 | b d\n4:1.2:0.5 | a b c\n2:1:0.3 | b c\n3:1.5:0.7 | a d" | ../vowpalwabbit/vw --cb 4 -f cb.model -p cb.train.out
        // echo -e "1:2 3:5 4:1:0.6 | a c d\n1:0.5 2:1:0.4 3:2 4:1.5 | c d" | ../vowpalwabbit/vw -i cb.model -t -p cb.out
        String[] train = new String[]{
                "1:2:0.4 | a c",
                "3:0.5:0.2 | b d",
                "4:1.2:0.5 | a b c",
                "2:1:0.3 | b c",
                "3:1.5:0.7 | a d"
        };
        String cbModel = temporaryFolder.newFile().getAbsolutePath();
        VW vw = new VW("--quiet --cb 4 -f " + cbModel);
        float[] trainPreds = new float[train.length];
        for (int i=0; i<train.length; ++i) {
            trainPreds[i] = vw.learn(train[i]);
        }
        float[] expectedTrainPreds = new float[]{1, 2, 2, 2, 2};
        vw.close();

        assertArrayEquals(expectedTrainPreds, trainPreds, 0.00001f);

        vw = new VW("--quiet -t -i " + cbModel);
        String[] test = new String[]{
                "1:2 3:5 4:1:0.6 | a c d",
                "1:0.5 2:1:0.4 3:2 4:1.5 | c d"
        };

        float[] testPreds = new float[test.length];
        for (int i=0; i<testPreds.length; ++i) {
            testPreds[i] = vw.predict(test[i]);
        }
        float[] expectedTestPreds = new float[]{4, 4};
        vw.close();
        assertArrayEquals(expectedTestPreds, testPreds, 0.000001f);
    }

    @Test
    public void testConcurrency() throws IOException, InterruptedException {
        final Map<String, Float> data = new TreeMap<String, Float>();

        data.put("-1 | 2", -0.444651f);
        data.put("-1 | 4", -0.448271f);
        data.put("-1 | 6", -0.449493f);
        data.put("-1 | 8", -0.450034f);
        data.put("1 | 1", 0.175389f);
        data.put("1 | 3", 0.174267f);
        data.put("1 | 5", 0.173154f);
        data.put("1 | 7", 0.172148f);

        final String model = temporaryFolder.newFile().getAbsolutePath();
        VW learn = new VW("--quiet --loss_function logistic -f " + model);
        for (String d : data.keySet()) {
            learn.learn(d);
        }
        learn.close();

        int numThreads = Runtime.getRuntime().availableProcessors();
        ExecutorService threadPool = Executors.newFixedThreadPool(numThreads);
        final VW predict = new VW("--quiet -i " + model);
        for (int i=0; i<numThreads; ++i) {
            Runnable run = new Runnable() {
                @Override
                public void run() {
                    for (int j=0; j<5e4; ++j) {
                        for (Entry<String, Float> e : data.entrySet()) {
                            float actual = predict.predict(e.getKey());
                            assertEquals(e.getValue(), actual, 1e-6f);
                        }
                    }
                }
            };
            threadPool.submit(run);
        }
        threadPool.shutdown();
        threadPool.awaitTermination(1, TimeUnit.DAYS);
        predict.close();
    }

    private void expectMultiLabelFailure(String type) {
        thrown.expect(IllegalVWInput.class);
        thrown.expectMessage("VW JNI layer only supports simple and multiclass predictions");
        VW vw = new VW("--quiet " + type);
        vw.close();
    }

    @Test
    public void testCSOASSLDF() {
        expectMultiLabelFailure("--csoaa_ldf 3");
    }

    @Test
    public void testCSOASSRank() {
        expectMultiLabelFailure("--csoaa_rank 3");
    }

    @Test
    public void testCBADF() {
        expectMultiLabelFailure("--cb_adf 3");
    }

    @Test
    public void testRankAll() {
        expectMultiLabelFailure("--rank_all 3");
    }

    @Test
    public void testMultiLabel() {
        expectMultiLabelFailure("--multilabel_oaa 3");
    }
}
