package vowpalWabbit;

import org.junit.Test;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

/**
 * Created by jmorra on 10/28/15.
 */
public class VWTest extends VWTestHelper {

    @Test
    public void testVersion() throws IOException {
        String actualVersion = VW.version();
        final String pkgVersion = "#define PACKAGE_VERSION ";
        BufferedReader reader = new BufferedReader(new FileReader("../vowpalwabbit/config.h"));
        try {
            String line;
            while (null != (line = reader.readLine()) && !line.startsWith(pkgVersion)) {
                continue;
            }

            if (null != line) {
                final String expectedVersion = line.replace(pkgVersion, "").replace("\"", "");
                assertEquals(expectedVersion, actualVersion);
            }
            else {
                fail("Couldn't find #define PACKAGE_VERSION in config.h");
            }
        }
        finally {
            reader.close();
        }
    }
}
