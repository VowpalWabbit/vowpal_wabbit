package vw.learner;

import org.junit.*;
import org.junit.rules.ExpectedException;
import org.junit.rules.TemporaryFolder;
import vw.VW;
import vw.VWTestHelper;

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
public class VWFloatLearnerTest {
    private String houseModel;
    private final String heightData = "|f height:0.23 weight:0.25 width:0.05";
    private VWFloatLearner houseScorer;

    @Rule
    public ExpectedException thrown = ExpectedException.none();

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @BeforeClass
    public static void globalSetup() throws IOException {
        VWTestHelper.loadLibrary();
    }

    @Before
    public void setup() throws IOException {
        // Since we want this test to continue to work between VW changes, we can't store the model
        // Instead, we'll make a new model for each test
        houseModel = temporaryFolder.newFile().getAbsolutePath();
        String[] houseData = new String[]{
                "0 | price:.23 sqft:.25 age:.05 2006",
                "1 2 'second_house | price:.18 sqft:.15 age:.35 1976",
                "0 1 0.5 'third_house | price:.53 sqft:.32 age:.87 1924"};
        VWFloatLearner learner = new VWFloatLearner(" --quiet -f " + houseModel);
        for (String d : houseData) {
            learner.learn(d);
        }
        learner.close();
        houseScorer = new VWFloatLearner("--quiet -t -i " + houseModel);
    }

    @After
    public void cleanup() {
        houseScorer.close();
    }

    private long streamingLoadTest(int times) {
        VWFloatLearner m1 = new VWFloatLearner("--quiet");
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
        VWFloatLearner learner = new VWFloatLearner("--quiet");
        float firstPrediction = learner.learn("0.1 " + heightData);
        float secondPrediction = learner.learn("0.9 " + heightData);
        assertNotEquals(firstPrediction, secondPrediction, 0.001);
        learner.close();
    }

    @Test
    public void testBadVWArgs() {
        final String args = "--BAD_FEATURE___ounq24tjnasdf8h";
        thrown.expect(IllegalArgumentException.class);
        new VWFloatLearner(args + " --quiet");
    }

    @Test
    public void testManySamples() {
        File model = new File("basic.model");
        model.deleteOnExit();
        VWFloatLearner m = new VWFloatLearner("--quiet --loss_function logistic --link logistic -f " + model.getAbsolutePath());
        for (int i=0; i<100; ++i) {
            m.learn("-1 | ");
            m.learn("1 | ");
        }
        m.close();

        float expVwOutput = 0.50419676f;
        m = new VWFloatLearner("--quiet -i " + model.getAbsolutePath());
        assertEquals(expVwOutput, m.predict("| "), 0.0001);
    }

    @Test
    public void twoModelTest() {
        VWFloatLearner m1 = new VWFloatLearner("--quiet");
        VWFloatLearner m2 = new VWFloatLearner("--quiet");

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
        VWFloatLearner s = new VWFloatLearner("--quiet");
        s.close();
        s.predict("1 | ");
    }

    @Test
    public void testOldModel() {
        thrown.expect(Exception.class);
        thrown.expectMessage("bad model format!");
        VWFloatLearner vw = new VWFloatLearner("--quiet -i src/test/resources/vw_7.8.model");
        vw.close();
    }

    @Test
    public void testBadModel() {
        // Right now VW seg faults on a bad model.  Ideally we should throw an exception
        // that the Java layer could do something about
        thrown.expect(Exception.class);
        thrown.expectMessage("Model content is corrupted, weight vector index 1347768914 must be less than total vector length 262144");
        VWFloatLearner vw = new VWFloatLearner("--quiet -i src/test/resources/vw_bad.model");
        vw.close();
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
        VWFloatLearner learn = new VWFloatLearner("--quiet --loss_function logistic -f " + model);
        for (String d : data.keySet()) {
            learn.learn(d);
        }
        learn.close();

        int numThreads = Runtime.getRuntime().availableProcessors();
        ExecutorService threadPool = Executors.newFixedThreadPool(numThreads);
        final VWFloatLearner predict = new VWFloatLearner("--quiet -i " + model);
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

    @Test
    public void testVersion() throws IOException {
        String actualVersion = VW.version();
        final String pkgVersion = "#define PACKAGE_VERSION ";
        BufferedReader reader = new BufferedReader(new FileReader("../vowpalwabbit/config.h"));
        try {
            String line = null;
            while(null != (line = reader.readLine()) && !line.startsWith(pkgVersion)) {
                continue;
            }

            if (null != line) {
                final String expectedVersion = line.replace(pkgVersion, "").replace("\"", "");
                assertEquals(expectedVersion, actualVersion);
            }
            else {
                fail("Couldn't find #define PACKAGE_VERSION in config.h");
            }
        }
        finally {
            reader.close();
        }
    }
}
