package vowpalWabbit;

import vowpalWabbit.learner.VWLearners;

public final class VW {
    /**
     * Should not be directly instantiated.
     */
    private VW(){}

    /**
     * This main method only exists to test the library implementation.  To test it just run
     * java -cp target/vw-jni-*-SNAPSHOT.jar vowpalWabbit.VW
     * @param args No args needed.
     */
    public static void main(String[] args) {
        VWLearners.create("").close();
        VWLearners.create("--quiet").close();
    }

    public static native String version();
}
