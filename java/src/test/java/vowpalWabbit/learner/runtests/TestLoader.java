package vowpalWabbit.learner.runtests;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.io.*;
import java.lang.reflect.Type;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;

/**
 * Loads test cases from core.vwtest.json and resolves dependencies.
 */
public class TestLoader {

    /**
     * Tests to skip for Java bindings.
     * Based on C# skip list with Java-specific additions.
     */
    public static final Set<Integer> SKIP_LIST = Collections.unmodifiableSet(new HashSet<>(Arrays.asList(
        // Crashes or known issues
        13, 32, 39, 258, 40, 259, 41, 260, 59, 60, 61, 66, 68, 90,
        25, 26, 349, 350, 356, 357, 358,

        // Shell scripts for input/output
        92, 95, 96, 98, 91, 99, 118, 119, 120,
        176, 177, 207, 208,

        // Float delta issues
        14, 16, 17, 31, 33, 34, 53, 101, 102, 103, 105, 106, 111, 112, 412, 413, 414, 422, 423, 424,

        // --examples to test parser
        71,

        // Native JSON parsing not supported in Java JNI
        143, 144, 146, 158, 189, 202, 237, 312, 316, 318, 319, 324, 325, 326, 347, 351, 348,

        // Bash script tests
        149, 152, 156, 193, 194, 217, 385,

        // Possibly float delta
        188,

        // --onethread is a shell option, not available via library
        195, 275, 276,

        // Cluster mode test
        203,

        // Testing option usage
        204,

        // DSJSON not supported
        216, 256, 299, 300, 306, 310, 311, 327, 328, 329, 330, 331, 367, 368, 396, 397, 398,
        405, 406, 407, 411, 415, 417, 456, 457, 458, 459, 460, 461, 462,

        // Daemon mode shell script test not supported
        220,

        // CATS prediction types not yet supported
        227, 229,

        // Flatbuffer input tests
        239, 240, 241, 242, 243, 244, 245, 246,

        // No data file
        383, 389, 390, 391, 392, 393,

        // Positional args
        400, 404,

        // Empty lines not supported
        464
    )));

    private final Path testRoot;
    private final Map<String, TestCase> outputModels = new HashMap<>();
    private final Map<Integer, TestCase> testCases = new LinkedHashMap<>();

    public TestLoader(Path testRoot) {
        this.testRoot = testRoot;
    }

    /**
     * Load tests from core.vwtest.json
     */
    public List<TestCase> loadTests() throws IOException {
        Path jsonPath = testRoot.resolve("core.vwtest.json");

        List<TestData> tests;
        try (Reader reader = Files.newBufferedReader(jsonPath, StandardCharsets.UTF_8)) {
            Gson gson = new Gson();
            Type listType = new TypeToken<List<TestData>>(){}.getType();
            tests = gson.fromJson(reader, listType);
        }

        for (TestData data : tests) {
            if (!data.hasVwCommand() || data.shouldSkipForJava() || data.isBashTest()) {
                continue;
            }

            TestCase testCase = new TestCase(data);

            // Track output models for dependency resolution
            if (testCase.hasFinalRegressor()) {
                outputModels.put(testCase.getFinalRegressor(), testCase);
            }

            // Resolve dependencies based on initial regressor
            if (testCase.hasInitialRegressor()) {
                TestCase dep = outputModels.get(testCase.getInitialRegressor());
                if (dep != null) {
                    testCase.setDependency(dep);
                } else if (!testCase.getInitialRegressor().startsWith("model-sets/")) {
                    // Model from model-sets/ are pre-existing, others need a dependency
                    Path modelPath = testRoot.resolve(testCase.getInitialRegressor());
                    if (!Files.exists(modelPath)) {
                        System.err.println("Warning: Missing dependency '" + testCase.getInitialRegressor()
                            + "' for test " + testCase.getId());
                    }
                }
            }

            testCases.put(testCase.getId(), testCase);
        }

        return new ArrayList<>(testCases.values());
    }

    /**
     * Get a specific test case by ID.
     */
    public TestCase getTestCase(int id) {
        return testCases.get(id);
    }

    /**
     * Check if a test should be skipped.
     */
    public static boolean shouldSkip(int testId) {
        return SKIP_LIST.contains(testId);
    }

    /**
     * Find the test root directory by searching upward from the current directory.
     */
    public static Path findTestRoot() {
        // Try various locations
        String[] candidates = {
            "test",                          // Running from repo root
            "../test",                       // Running from java/
            "../../test",                    // Running from java/target/
            System.getenv("VW_TEST_ROOT")    // Environment variable override
        };

        for (String candidate : candidates) {
            if (candidate == null) continue;
            Path path = Paths.get(candidate);
            if (Files.exists(path.resolve("core.vwtest.json"))) {
                return path.toAbsolutePath().normalize();
            }
        }

        // Try to find from class location
        try {
            Path classPath = Paths.get(TestLoader.class.getProtectionDomain().getCodeSource().getLocation().toURI());
            Path root = classPath;
            for (int i = 0; i < 10; i++) {
                Path testPath = root.resolve("test");
                if (Files.exists(testPath.resolve("core.vwtest.json"))) {
                    return testPath.toAbsolutePath().normalize();
                }
                root = root.getParent();
                if (root == null) break;
            }
        } catch (Exception e) {
            // Ignore and fall through to error
        }

        throw new IllegalStateException(
            "Cannot find test root directory with core.vwtest.json. " +
            "Set VW_TEST_ROOT environment variable or run from repository root.");
    }
}
