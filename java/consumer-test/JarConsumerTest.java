import vowpalWabbit.learner.VWLearners;
import vowpalWabbit.learner.VWScalarLearner;

/**
 * Does the published JAR work on this platform?
 *
 * The rest of the Java pipeline tests the code. This tests the artifact, and only the
 * artifact: it runs with the assembled vw-jni JAR as its entire classpath, nothing built
 * from the working tree, and nothing on java.library.path. That is the situation of
 * somebody who has just added the dependency to their project.
 *
 * What it is looking for is the gap between "the JAR contains a native library" and "the
 * native library loads". The assemble job checks the first with `jar tf | grep natives/`,
 * and that cannot catch:
 *
 *   - a natives/ directory whose name does not match what common.Native computes from
 *     os.name and os.arch on this platform, so the loader never finds it
 *   - a library built for the wrong architecture, which the five parallel build jobs make
 *     easy to get wrong
 *   - a truncated or missing upload for one of those five platforms
 *   - symbols that do not resolve -- a JNI function missing extern "C" gets its name
 *     mangled, and Java throws UnsatisfiedLinkError from a JAR that looks perfect
 *
 * Deliberately says nothing about whether VW's numbers are right; the test suite does
 * that. This only asks whether the thing we are about to publish can be loaded and called.
 */
public class JarConsumerTest {
    public static void main(String[] args) throws Exception {
        System.out.println("os.name      = " + System.getProperty("os.name"));
        System.out.println("os.arch      = " + System.getProperty("os.arch"));
        System.out.println("java.version = " + System.getProperty("java.version"));

        // The native is loaded by VWLearners' static initialiser, which tries
        // java.library.path first and falls back to extracting the right native out of
        // the JAR. With nothing on java.library.path, this exercises the fallback -- the
        // path every real consumer takes.
        VWScalarLearner learner = VWLearners.create("--quiet");

        // Same shape as VWScalarLearnerTest: two learns on the same features with
        // different labels have to move the prediction.
        String features = "|f height:0.23 weight:0.25 width:0.05";
        float first = learner.learn("0.1 " + features);
        float second = learner.learn("0.9 " + features);
        learner.close();

        System.out.println("prediction 1 = " + first);
        System.out.println("prediction 2 = " + second);

        if (Float.isNaN(first) || Float.isInfinite(first)
                || Float.isNaN(second) || Float.isInfinite(second)) {
            throw new AssertionError("non-finite prediction: " + first + ", " + second);
        }
        if (Math.abs(second - first) < 0.001f) {
            throw new AssertionError(
                "learning did not move the prediction (" + first + " -> " + second + "); "
                + "the native loaded but is not doing anything");
        }

        System.out.println("OK: native loaded from the JAR and the learner works");
    }
}
