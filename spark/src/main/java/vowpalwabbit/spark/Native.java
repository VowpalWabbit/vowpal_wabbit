package vowpalwabbit.spark;

// For cross-platform, but it doesn't support dependencies yet
// import org.scijava.nativelib.NativeLoader;
import java.nio.file.*;
import java.net.URL;
import java.io.File;
import java.util.jar.*;
import java.util.*;

public class Native {
    static {
        try {
           // NativeLoader.loadLibrary("vw_spark_jni");

            // create temp directory
           Path tempDirectory = Files.createTempDirectory("tmplibvw");
           tempDirectory.toFile().deleteOnExit();

           // Extract library and dependencies
           File jarFile = new File(Native.class.getProtectionDomain().getCodeSource().getLocation().getPath());
            try (JarFile jar = new JarFile(jarFile)) {
                Enumeration<JarEntry> entries = jar.entries();
                while(entries.hasMoreElements()) {
                    final String name = entries.nextElement().getName();
                    if (name.endsWith("/"))
                        continue;

                    if (name.startsWith("natives/linux_64/")) {
                        Path parent = Paths.get(name).getParent();
                        if (parent != null)
                            Files.createDirectories(tempDirectory.resolve(parent));

                        Files.copy(Native.class.getResourceAsStream("/" + name), tempDirectory.resolve(name));
                    }
                }
            }

            // load the library
            System.load(tempDirectory.resolve("natives/linux_64/libvw_spark_jni.so").toString());
        } catch (Exception e) {
            throw new RuntimeException("Unable to load native library 'vw_spark_jni'", e);
        }
    }

    public static void load() {
        // just execute the static constructor
    }
}