package common;

import java.nio.file.*;
import java.io.File;
import java.io.IOException;
import java.util.jar.*;
import java.util.*;

public class Native {
    // Delegated to NativePlatform, which has no static initialiser, so the mapping stays
    // testable without a native library present. See NativePlatformTest.
    private static final String PLATFORM_DIR =
        NativePlatform.directory(System.getProperty("os.name", ""), System.getProperty("os.arch", ""));
    private static final String LIB_NAME =
        NativePlatform.libraryName(System.getProperty("os.name", ""));

    private static void try_load_from_path() {
        System.loadLibrary("vw_jni");
    }

    private static void try_load_from_jar() throws IOException {
        String nativesPrefix = "natives/" + PLATFORM_DIR + "/";

        // create temp directory
        Path tempDirectory = Files.createTempDirectory("tmplibvw");
        tempDirectory.toFile().deleteOnExit();

        // Extract library and dependencies
        File jarFile = new File(Native.class.getProtectionDomain().getCodeSource().getLocation().getPath());
        JarFile jar = null;
        boolean foundLibrary = false;
        try {
            jar = new JarFile(jarFile);
            Enumeration<JarEntry> entries = jar.entries();
            while (entries.hasMoreElements()) {
                final String name = entries.nextElement().getName();
                if (name.endsWith("/"))
                    continue;

                if (name.startsWith(nativesPrefix)) {
                    Path dest_file = tempDirectory.resolve(name);
                    if (!dest_file.normalize().startsWith(tempDirectory)) {
                        throw new RuntimeException("Bad zip entry");
                    }
                    Path parent = dest_file.getParent();
                    if (parent != null)
                        Files.createDirectories(parent);

                    Files.copy(Native.class.getResourceAsStream("/" + name), dest_file);
                    foundLibrary = true;
                }
            }
        } finally {
            if (jar != null)
                jar.close();
        }

        if (!foundLibrary) {
            throw new UnsupportedOperationException(
                "No native library found for platform: " + PLATFORM_DIR +
                ". Supported platforms can be found at https://vowpalwabbit.org/docs/vowpal_wabbit/java/");
        }

        // load the library
        System.load(tempDirectory.resolve(nativesPrefix + LIB_NAME).toString());
    }

    static {
        try {
            try_load_from_path();
        } catch (UnsatisfiedLinkError ex) {
            try {
                try_load_from_jar();
            } catch (Exception ex_inner) {
                throw new RuntimeException("Unable to load native library 'vw_jni'", ex_inner);
            }
            catch (UnsatisfiedLinkError ex_inner) {
                throw new RuntimeException("Unable to load native library 'vw_jni'", ex_inner);
            }
        }
    }

    public static void load() {
        // just execute the static constructor
    }
}