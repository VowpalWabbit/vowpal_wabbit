package vowpalWabbit.learner;

import org.junit.Ignore;
import org.junit.Test;
import org.junit.Rule;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.VWTestHelper;

import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;

/**
 * Created by jmorra on 10/29/15.
 * NOTE: Because of some changes to VW during some of the tests which are supposed to trigger failures
 *   standard error is written to.  This is expected behavior during these tests.
 */
public class VWLearnersTest extends VWTestHelper {

    private final String heightData = "|f height:0.23 weight:0.25 width:0.05";

    @Test
    public void testWrongType() {
        thrown.expect(ClassCastException.class);
        VWScalarsLearner learner = VWLearners.create("--cb 4 --quiet");
    }

    @Test
    public void testBadVWArgs() {
        final String args = "--BAD_FEATURE___ounq24tjnasdf8h";
        thrown.expect(IllegalArgumentException.class);
        VWLearners.create(args + " --quiet");
    }

    @Test
    public void testAlreadyClosed() throws IOException {
        thrown.expect(IllegalStateException.class);
        thrown.expectMessage("Already closed.");
        VWScalarLearner s = VWLearners.create("--quiet");
        s.close();
        s.predict("1 | ");
    }

    @Test
    public void testOldModel() throws IOException {
        thrown.expect(Exception.class);
        thrown.expectMessage("bad model format!");
        VWScalarsLearner vw = VWLearners.create("--quiet -i src/test/resources/vw_7.8.model");
        vw.close();
    }

    @Test
    public void testBadModel() throws IOException {
        // Right now VW seg faults on a bad model.  Ideally we should throw an exception
        // that the Java layer could do something about
        thrown.expect(Exception.class);
        thrown.expectMessage("Model content is corrupted, weight vector index 1347768914 must be less than total vector length 262144");
        VWScalarsLearner vw = VWLearners.create("--quiet -i src/test/resources/vw_bad.model");
        vw.close();
    }

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @Test
    public void testSaveModel() throws IOException {
        VWLearner vw = VWLearners.create("--quiet");
        File file = temporaryFolder.newFile("saved_test_model");
        vw.saveModel(file);
        vw.close();
        assert(file.exists());
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
        VWScalarLearner learn = VWLearners.create("--quiet --loss_function logistic -f " + model);
        for (String d : data.keySet()) {
            learn.learn(d);
        }
        learn.close();

        int numThreads = Runtime.getRuntime().availableProcessors();
        ExecutorService threadPool = Executors.newFixedThreadPool(numThreads);
        final VWScalarLearner predict = VWLearners.create("--quiet -i " + model);
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

    private long streamingLoadTest(int times) throws IOException {
        VWScalarsLearner m1 = VWLearners.create("--quiet");
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
            writer.println(heightData + "");
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
}
