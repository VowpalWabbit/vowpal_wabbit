package vw;

import vw.learner.VWFactory;

import java.io.IOException;

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
    public static void main(String[] args) throws IOException {
        VWFactory.getVWLeaner("").close();
        VWFactory.getVWLeaner("--quiet").close();
    }

    public static native String version();
}
