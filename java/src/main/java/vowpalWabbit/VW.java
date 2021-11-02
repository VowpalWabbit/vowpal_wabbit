package vowpalWabbit;

import common.Native;
import vowpalWabbit.learner.VWLearners;

public final class VW {
    static {
        try {
            // Load from java.library.path
            System.loadLibrary("vw_jni");
        } catch (UnsatisfiedLinkError e) {
            // Load from JAR
            Native.load();
        }
    }

    /**
     * Should not be directly instantiated.
     */
    private VW() {
    }

    /**
     * This main method only exists to test the library implementation. To test
     * it, just run java -cp target/vw-jni-*-SNAPSHOT.jar vowpalWabbit.VW
     *
     * @param args No args needed.
     * @throws Exception possibly during close.
     */
    public static void main(String[] args) throws Exception {
        VWLearners.create("").close();
        VWLearners.create("--quiet").close();
    }

    public static native String version();
}
