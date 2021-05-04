package common;

// TODO: Remove the hardcoded directory when loading the library, so that the
// loader works cross-platform, but it doesn't support dependencies yet.

import java.nio.file.*;
import java.io.File;
import java.util.jar.*;
import java.util.*;

public class Native {
    static {
        try {
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
                        Path parent = Paths.get(name).getParent();
                        if (parent != null)
                            Files.createDirectories(tempDirectory.resolve(parent));

                        Files.copy(Native.class.getResourceAsStream("/" + name), tempDirectory.resolve(name));
                    }
                }
            } finally {
                if (jar != null)
                    jar.close();
            }

            // load the library
            System.load(tempDirectory.resolve("natives/linux_64/libvw_jni.so").toString());

        } catch (Exception e) {
            throw new RuntimeException("Unable to load native library 'vw_jni'", e);
        }
    }

    public static void load() {
        // just execute the static constructor
    }
}