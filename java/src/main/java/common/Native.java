package common;

import java.nio.file.*;
import java.io.File;
import java.io.IOException;
import java.util.jar.*;
import java.util.*;

public class Native {
    private static final String PLATFORM_DIR = getPlatformDir();
    private static final String LIB_NAME = getLibraryName();

    /**
     * Detects the current platform and returns the appropriate natives directory name.
     * Supported platforms: linux_64, linux_arm64, macos_x64, macos_arm64, windows_64
     */
    private static String getPlatformDir() {
        String os = System.getProperty("os.name", "").toLowerCase();
        String arch = System.getProperty("os.arch", "").toLowerCase();

        String osDir;
        if (os.contains("linux")) {
            osDir = "linux";
        } else if (os.contains("mac") || os.contains("darwin")) {
            osDir = "macos";
        } else if (os.contains("win")) {
            osDir = "windows";
        } else {
            throw new UnsupportedOperationException("Unsupported operating system: " + os);
        }

        String archDir;
        if (arch.equals("amd64") || arch.equals("x86_64")) {
            archDir = osDir.equals("linux") ? "64" : "x64";
        } else if (arch.equals("aarch64") || arch.equals("arm64")) {
            archDir = "arm64";
        } else {
            throw new UnsupportedOperationException("Unsupported architecture: " + arch);
        }

        return osDir + "_" + archDir;
    }

    /**
     * Returns the platform-specific library file name.
     */
    private static String getLibraryName() {
        String os = System.getProperty("os.name", "").toLowerCase();
        if (os.contains("win")) {
            return "vw_jni.dll";
        } else if (os.contains("mac") || os.contains("darwin")) {
            return "libvw_jni.dylib";
        } else {
            return "libvw_jni.so";
        }
    }

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