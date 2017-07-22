package vowpalWabbit;

import com.google.common.util.concurrent.*;
import org.junit.Test;
import vowpalWabbit.learner.VWLearners;
import vowpalWabbit.learner.VWScalarLearner;

import javax.annotation.Nullable;
import javax.annotation.concurrent.NotThreadSafe;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.Callable;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

import static org.junit.Assert.*;

/**
 * Test various functions related to closing the model.
 *
 * These tests may fail as they are likely related to the speed of the computer on which they are run.
 * Created by ryan.deak on 7/21/17.
 */
@NotThreadSafe
public class ClosingTest  extends VWTestHelper {
    private static final int DELTA_TIME_MS = 50; // milliseconds
    private static final int PASSES = 25000;

    /**
     * Enable to see diagnostics during tests.
     */
    private static final boolean DIAGNOSTICS = false;

    @Test
    public void testCloseAsyncIsNearImmediate() throws Exception {
        VWScalarLearner vw = model(PASSES);

        long t1 = System.currentTimeMillis();
        Callable<Void> closer = vw.closer();
        long t2 = System.currentTimeMillis();
        closer.call();
        long t3 = System.currentTimeMillis();

        assertEquals("Should only take a few milliseconds to get future", 0, t2 - t1, DELTA_TIME_MS);
        assertNotEquals("Should take longer to finish synchronous closing", 0, t3 - t1, DELTA_TIME_MS);
    }

    @Test
    public void testRegisteringClosingCallbackNoDelay() throws Exception {
        testClosingCallback(0);
    }

    @Test
    public void testRegisteringClosingCallbackWithShortDelay() throws Exception {
        testClosingCallback(250);
    }

    @Test
    public void testRegisteringClosingCallbackWithLongDelay() throws Exception {
        testClosingCallback(1000);
    }

    /**
     * Max test run time: ~ sleepMsAfterClose + 2.5 seconds
     * @param msBeforeRegisteringCallback milliseconds to wait before registering the close-detecting callback.
     * @throws Exception due to closing issues.
     */
    private void testClosingCallback(final int msBeforeRegisteringCallback) throws Exception {
        final ListeningExecutorService exSvc = MoreExecutors.listeningDecorator(Executors.newFixedThreadPool(1));
        final VWScalarLearner vw = model(PASSES);
        final CalledTester callback = new CalledTester();
        final int sleepMs = 100;
        final int maxIterations = 50;
        int i = 0;

        // Get the closer
        final Callable<Void> closer = vw.closer();

        // Submit to an executor service
        final ListenableFuture<Void> closeInFuture = exSvc.submit(closer);

        // Delay prior to registering callback.
        if (0 < msBeforeRegisteringCallback)
            Thread.sleep(Math.min(5000, msBeforeRegisteringCallback));

        // Register callback.
        Futures.addCallback(closeInFuture, callback);

        // Test whether the callback is called.
        for (; i < maxIterations && !callback.wasCalled(); ++i) {
            Thread.sleep(sleepMs);
        }

        assertTrue("Callback was never called.", callback.wasCalled());
    }

    private VWScalarLearner model(final int passes) throws IOException {
        return model(passes, true);
    }

    private VWScalarLearner model(final int passes, final boolean quiet) throws IOException {
        File cacheFile = File.createTempFile("ClosingTest", ".vw.cache");
        cacheFile.deleteOnExit();

        String command =
                (!DIAGNOSTICS && quiet ?
                    " --quiet " :
                    " --progress " + ((int) Math.round(passes / 10d))
                ) +
                " --passes " + passes +
                " --holdout_off " +
                " --cache_file " + cacheFile.getCanonicalPath();

        // Create a learner and just have it take 1 example, but many passes.
        VWScalarLearner vw = VWLearners.create(command);

        // Just one example per pass.
        vw.learn("1 | x");

        return vw;
    }


    private static class CalledTester implements FutureCallback<Void> {
        private AtomicBoolean called = new AtomicBoolean(false);
        boolean wasCalled() { return called.get(); }

        @Override
        public void onSuccess(@Nullable Void _void) {
            called.set(true);
        }

        @Override
        public void onFailure(Throwable throwable) {
            throw new RuntimeException(throwable);
        }
    }
}
