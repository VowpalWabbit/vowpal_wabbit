package vowpalWabbit;

import vowpalWabbit.learner.VWLearners;

public final class VW {
    static {
        System.loadLibrary("vw_jni");
    }

    /**
     * Should not be directly instantiated.
     */
    private VW(){}

    /**
     * This main method only exists to test the library implementation.  To test it just run
     * java -cp target/vw-jni-*-SNAPSHOT.jar vowpalWabbit.VW
     * @param args No args needed.
     * @throws Exception possibly during close.
     */
    public static void main(String[] args) throws Exception {
        VWLearners.create("").close();
        VWLearners.create("--quiet").close();
    }

    public static native String version();
}
