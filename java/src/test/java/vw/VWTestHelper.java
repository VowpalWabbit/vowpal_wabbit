package vw;

import org.junit.BeforeClass;

import java.io.File;
import java.io.IOException;

/**
 * Created by jmorra on 10/2/15.
 */
public class VWTestHelper {

    private static boolean loaded = false;

    @BeforeClass
    public static void loadLibrary() throws IOException {
        if (!loaded) {
            try {
                System.load(new File(".").getCanonicalPath() + "/target/vw_jni.lib");
            }
            catch (UnsatisfiedLinkError ignored) {
                // Do nothing as this means that the library should be loaded as part of the jar
            }
            loaded = true;
        }
    }
}
