package common;

import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

/**
 * Pins the platform -> natives directory mapping.
 *
 * The directory names NativePlatform computes at runtime have to match the ones
 * java/CMakeLists.txt writes at build time. Those are two independent mappings in two
 * languages, and nothing but this test connects them. When they drifted -- CMake writing
 * natives/windows_64 while Native looked for natives/windows_x64 -- every build passed,
 * `jar tf` showed a native present, the Java test suite passed, and the Windows native in
 * the published 9.11.1 JAR on Maven Central was unloadable.
 *
 * The expected values below are the second copy of the table in java/CMakeLists.txt.
 * Changing one without the other should fail here, in milliseconds, on any platform,
 * rather than on a consumer's machine.
 */
public class NativePlatformTest {

    private void check(String osName, String archName, String expected) {
        assertEquals(osName + " / " + archName, expected, NativePlatform.directory(osName, archName));
    }

    @Test
    public void linuxDirectories() {
        // CMake: CMAKE_SYSTEM_NAME Linux, processor not aarch64/arm64 -> linux_64
        check("Linux", "amd64", "linux_64");
        check("Linux", "x86_64", "linux_64");
        check("Linux", "aarch64", "linux_arm64");
        check("Linux", "arm64", "linux_arm64");
    }

    @Test
    public void macosDirectories() {
        // macOS is the only platform using x64 rather than 64, which is what the original
        // bug got backwards.
        check("Mac OS X", "x86_64", "macos_x64");
        check("Mac OS X", "amd64", "macos_x64");
        check("Mac OS X", "aarch64", "macos_arm64");
        check("Darwin", "arm64", "macos_arm64");
    }

    @Test
    public void windowsDirectories() {
        // The regression: these must be windows_64, not windows_x64. os.name carries the
        // release, so the match has to be on a substring.
        check("Windows Server 2025", "amd64", "windows_64");
        check("Windows 10", "amd64", "windows_64");
        check("Windows 11", "x86_64", "windows_64");
        check("Windows Server 2025", "aarch64", "windows_arm64");
    }

    @Test
    public void caseIsNotSignificant() {
        check("LINUX", "AMD64", "linux_64");
        check("windows server 2025", "AMD64", "windows_64");
    }

    @Test
    public void libraryNamesPerOs() {
        assertEquals("vw_jni.dll", NativePlatform.libraryName("Windows Server 2025"));
        assertEquals("libvw_jni.dylib", NativePlatform.libraryName("Mac OS X"));
        assertEquals("libvw_jni.dylib", NativePlatform.libraryName("Darwin"));
        assertEquals("libvw_jni.so", NativePlatform.libraryName("Linux"));
    }

    @Test
    public void unsupportedPlatformsAreRejectedNotGuessed() {
        // A guess here produces a JAR path that does not exist, which is the failure this
        // whole test exists to prevent. Throwing names the problem instead.
        try {
            NativePlatform.directory("Plan 9", "amd64");
            fail("expected an exception for an unsupported operating system");
        } catch (UnsupportedOperationException expected) {
            // ok
        }
        try {
            NativePlatform.directory("Linux", "riscv64");
            fail("expected an exception for an unsupported architecture");
        } catch (UnsupportedOperationException expected) {
            // ok
        }
    }
}
