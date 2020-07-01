package vowpalWabbit;

import com.google.common.util.concurrent.*;
import org.junit.Test;
import vowpalWabbit.learner.VWLearners;
import vowpalWabbit.learner.VWScalarLearner;

import javax.annotation.Nullable;
import javax.annotation.concurrent.NotThreadSafe;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;
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
        Callable<Boolean> closer = vw.closer();
        long t2 = System.currentTimeMillis();
        closer.call();
        long t3 = System.currentTimeMillis();

        assertEquals("Should only take a few milliseconds to get future", 0, t2 - t1, DELTA_TIME_MS);
        assertNotEquals("Should take longer to finish synchronous closing", 0, t3 - t1, DELTA_TIME_MS);
    }

    @Test
    public void closeMultipleTimes() throws IOException {
        try {
            final VWScalarLearner model = model(1);
            model.close();
            model.close();
        }
        catch (Exception e) {
            final PrintWriter str = new PrintWriter(new StringWriter());
            e.printStackTrace(str);
            fail("Shouldn't throw exception but threw " + str.toString());
        }
    }

    @Test
    public void closerThenClose() {
        try {
            final VWScalarLearner model = model(1);
            model.closer().call();
            model.close();
        }
        catch (Exception e) {
            final PrintWriter str = new PrintWriter(new StringWriter());
            e.printStackTrace(str);
            fail("Shouldn't throw exception but threw " + str.toString());
        }
    }

    @Test
    public void closeThenCloser() {
        try {
            final VWScalarLearner model = model(1);
            model.close();
            model.closer().call();
        }
        catch (Exception e) {
            final PrintWriter str = new PrintWriter(new StringWriter());
            e.printStackTrace(str);
            fail("Shouldn't throw exception but threw " + str.toString());
        }
    }

    @Test
    public void multipleAsynchronousCloserCallsReturnTrueOnlyOnce() throws Exception {
        final int numClosers = 10;   // Should be greater than 2 to avoid confusion.
        final VWScalarLearner model = model(PASSES);
        ArrayList<ListenableFuture<Boolean>> closeFutures = new ArrayList<ListenableFuture<Boolean>>(numClosers);
        final ListeningExecutorService exSvc =
            MoreExecutors.listeningDecorator(Executors.newFixedThreadPool(numClosers));

        for (int i = 0; i < numClosers; ++i) {
            // Submit a closer for asynchronous execution to the execution service.
            // Place the future in the list.
            closeFutures.add(exSvc.submit(model.closer()));
        }

        // Sequence the List of Futures as a Future of a List, then block until all futures return.
        final List<Boolean> closeResults  = Futures.allAsList(closeFutures).get();

        // Count the number of times the closers returned true.  This should be exactly once.
        // This also means by the pigeonhole principle that there are (numClosers - 1) closers
        // that returned false.
        int closeAttempts = 0;
        for (boolean triedToClose: closeResults) {
            closeAttempts += triedToClose ? 1 : 0;
        }
        assertEquals(1, closeAttempts);
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
        final Callable<Boolean> closer = vw.closer();

        // Submit to an executor service
        final ListenableFuture<Boolean> closeInFuture = exSvc.submit(closer);

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
        String command =
                (!DIAGNOSTICS && quiet ?
                    " --quiet " :
                    " --progress " + ((int) Math.round(passes / 10d))
                    ) +
                " --passes " + passes +
                " --holdout_off " +
                " --cache";

        // Create a learner and just have it take 1 example, but many passes.
        VWScalarLearner vw = VWLearners.create(command);

        // Just one example per pass.
        vw.learn("1 | x");

        return vw;
    }

    /**
     * It's OK to have the domain be the top type since the input value is ignored and
     * the <em>Futures.addCallback</em> method is contravariant in the callback's domain.
     */
    private static class CalledTester implements FutureCallback<Object> {
        private AtomicBoolean called = new AtomicBoolean(false);
        boolean wasCalled() { return called.get(); }

        @Override
        public void onSuccess(@Nullable Object ignored) {
            called.set(true);
        }

        @Override
        public void onFailure(Throwable throwable) {
            throw new RuntimeException(throwable);
        }
    }
}
