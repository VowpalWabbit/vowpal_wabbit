package vw;

import vw.learner.VWFactory;
import vw.learner.VWFloatLearner;

public final class VW {
    /**
     * Should not be directly instantiated.
     */
    private VW(){}

    /**
     * This main method only exists to test the library implementation.  To test it just run
     * java -cp target/vw-jni-*-SNAPSHOT.jar vw.VW
     * @param args No args needed.
     */
    public static void main(String[] args) {
        VWFactory.getVWLeaner("", VWFloatLearner.class).close();
        VWFactory.getVWLeaner("--quiet", VWFloatLearner.class).close();
    }

    public static native String version();
}
