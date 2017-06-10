package vowpalWabbit;

import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.rules.ExpectedException;
import org.junit.rules.TemporaryFolder;
import vowpalWabbit.responses.ActionProb;
import vowpalWabbit.responses.ActionProbs;
import vowpalWabbit.responses.ActionScore;
import vowpalWabbit.responses.ActionScores;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWTestHelper {

    private static AtomicBoolean loaded = new AtomicBoolean(false);

    @Rule
    public ExpectedException thrown = ExpectedException.none();

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();

    @BeforeClass
    public static void loadLibrary() throws IOException {
        if (!loaded.getAndSet(true)) {
            try {
                System.loadLibrary("vw_jni");
            }
            catch (UnsatisfiedLinkError ignored) {
                // Do nothing as this means that the library should be loaded as part of the jar
            }
        }
    }

    public ActionScores actionScores(ActionScore... actionScores) {
        return new ActionScores(actionScores);
    }

    public ActionProbs actionProbs(ActionProb... actionProbs) {
        return new ActionProbs(actionProbs);
    }

    public ActionScore actionScore(int action, float score) {
        return new ActionScore(action, score);
    }

    public ActionProb actionProb(int action, float prob) {
        return new ActionProb(action, prob);
    }
}
