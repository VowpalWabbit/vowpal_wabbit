package common;

/**
 * Where a native library lives inside the JAR, for a given operating system and
 * architecture.
 *
 * <p>The directory names here are a contract with {@code java/CMakeLists.txt}, which picks
 * the same names at build time when it copies the built library into
 * {@code java/target/bin/natives/}. Two independent mappings, in two languages, with
 * nothing connecting them but {@link NativePlatformTest}. They drifted once -- CMake
 * writing {@code windows_64} while this computed {@code windows_x64} -- and the Windows
 * native in the published 9.11.1 JAR was unloadable while every build and test passed.
 * Change one side, change both.
 *
 * <p>This lives apart from {@link Native} on purpose. Native's static initialiser loads
 * the library, so anything reading a constant on it drags that in; the mapping needs to be
 * callable with no native present, for every platform, from any platform.
 */
final class NativePlatform {

    private NativePlatform() {}

    /**
     * Supported: linux_64, linux_arm64, macos_x64, macos_arm64, windows_64, windows_arm64.
     *
     * @throws UnsupportedOperationException rather than guessing. A guess produces a JAR
     *     path that does not exist, which fails later and further away.
     */
    static String directory(String osName, String archName) {
        String os = osName.toLowerCase();
        String arch = archName.toLowerCase();

        String osDir;
        if (os.contains("linux")) {
            osDir = "linux";
        } else if (os.contains("mac") || os.contains("darwin")) {
            osDir = "macos";
        } else if (os.contains("win")) {
            osDir = "windows";
        } else {
            throw new UnsupportedOperationException("Unsupported operating system: " + osName);
        }

        String archDir;
        if (arch.equals("amd64") || arch.equals("x86_64")) {
            // macOS is the only one using x64; linux and windows are both plain 64. Getting
            // this backwards for windows is the bug this class was extracted over.
            archDir = osDir.equals("macos") ? "x64" : "64";
        } else if (arch.equals("aarch64") || arch.equals("arm64")) {
            archDir = "arm64";
        } else {
            throw new UnsupportedOperationException("Unsupported architecture: " + archName);
        }

        return osDir + "_" + archDir;
    }

    /**
     * The library file name for an operating system.
     *
     * <p>Tests macOS before Windows, and matches "windows" rather than "win", because
     * "darwin" contains "win". The original ordering returned vw_jni.dll for an os.name of
     * "Darwin" while {@link #directory} put the same platform in macos_*, so a JVM
     * reporting Darwin would have looked for macos_arm64/vw_jni.dll. HotSpot reports
     * "Mac OS X", so this never fired in practice -- but directory() accepts "darwin"
     * explicitly, and the two should not disagree about what that means.
     */
    static String libraryName(String osName) {
        String os = osName.toLowerCase();
        if (os.contains("mac") || os.contains("darwin")) {
            return "libvw_jni.dylib";
        } else if (os.contains("windows")) {
            return "vw_jni.dll";
        } else {
            return "libvw_jni.so";
        }
    }
}
