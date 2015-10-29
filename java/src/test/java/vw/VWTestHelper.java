package vw;

import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.rules.ExpectedException;
import org.junit.rules.TemporaryFolder;

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
                System.load(new File(".").getCanonicalPath() + "/target/vw_jni.lib");
            }
            catch (UnsatisfiedLinkError ignored) {
                // Do nothing as this means that the library should be loaded as part of the jar
            }
        }
    }
}
