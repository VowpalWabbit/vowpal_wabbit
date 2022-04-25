package common;

// TODO: Remove the hardcoded directory when loading the library, so that the
// loader works cross-platform, but it doesn't support dependencies yet.

import java.nio.file.*;
import java.io.File;
import java.io.IOException;
import java.util.jar.*;
import java.util.*;

public class Native {
    private static void try_load_from_path() {
        System.loadLibrary("vw_jni");
    }

    private static void try_load_from_jar() throws IOException {
        // create temp directory
            Path tempDirectory = Files.createTempDirectory("tmplibvw");
            tempDirectory.toFile().deleteOnExit();

            // Extract library and dependencies
            File jarFile = new File(Native.class.getProtectionDomain().getCodeSource().getLocation().getPath());
            JarFile jar = null;
            try {
                jar = new JarFile(jarFile);
                Enumeration<JarEntry> entries = jar.entries();
                while (entries.hasMoreElements()) {
                    final String name = entries.nextElement().getName();
                    if (name.endsWith("/"))
                        continue;

                    if (name.startsWith("natives/linux_64/")) {
                        Path dest_file = tempDirectory.resolve(name);
                        if (!dest_file.normalize().startsWith(tempDirectory))
                        {
                            throw new RuntimeException("Bad zip entry");
                        }
                        Path parent = dest_file.getParent();
                        if (parent != null)
                            Files.createDirectories(tempDirectory.resolve(parent));

                        Files.copy(Native.class.getResourceAsStream("/" + name), dest_file);
                    }
                }
            } finally {
                if (jar != null)
                    jar.close();
            }

            // load the library
            System.load(tempDirectory.resolve("natives/linux_64/libvw_jni.so").toString());
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