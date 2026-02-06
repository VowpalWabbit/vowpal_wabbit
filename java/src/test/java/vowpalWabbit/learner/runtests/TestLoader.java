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
     * Aligned with C# skip list (PR 4861) plus genuine Java-specific limitations.
     * JSON/DSJSON tests are filtered separately via TestCase.isJson() in RunTestsIT.
     * Bash/shell tests are filtered via TestData.isBashTest() in loadTests().
     */
    public static final Set<Integer> SKIP_LIST = Collections.unmodifiableSet(new HashSet<>(Arrays.asList(
        // ===== C# skip list baseline (PR 4861) =====
        69,                              // --examples flag not respected by test helper
        237, 238,                        // Multiline learner format issues
        239, 240, 241, 242, 243, 244,    // Flatbuffer input format (not supported in wrappers)
        273, 274,                        // --help causes process exit, crashes test host

        // ===== Java-specific: additional flatbuffer tests =====
        245, 246,

        // ===== Unsupported prediction types (not mapped in JNI getReturnType) =====
        // CATS/CATS_PDF → ACTION_PDF_VALUE / PDF prediction types
        224, 225, 226, 227, 229, 482, 483,
        // cs_active → ACTIVE_MULTICLASS prediction type
        162, 163, 475, 543, 544, 630,
        // cbzo → NOPRED prediction type
        277, 278, 279, 280, 281, 282, 283, 284,
        // LDA → returns MULTICLASS_PROBS (not mapped in JNI)
        17, 472, 505, 529, 624, 625, 651, 652,

        // ===== --onethread is a shell option, not available via library =====
        193, 195, 275, 276, 511, 643, 644,

        // ===== --csv is not compiled into the library build =====
        555, 611, 690, 691, 692, 693, 694,

        // ===== search entity_relation task has C++ assert() that aborts JVM =====
        63, 504, 518, 519, 559, 667, 668, 669,

        // ===== automl --predict_only_model bit_precision mismatch =====
        424, 440, 448,

        // ===== cb_adf multipass crash regression test (close exception in library mode) =====
        342,

        // ===== Cluster mode (not supported via library) =====
        203,

        // ===== Daemon mode (not supported via library) =====
        220
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

            // Resolve dependencies based on initial regressor or feature mask
            String depModel = "";
            if (testCase.hasInitialRegressor()) {
                depModel = testCase.getInitialRegressor();
            } else if (testCase.hasFeatureMask()) {
                depModel = testCase.getFeatureMask();
            }
            if (!depModel.isEmpty()) {
                TestCase dep = outputModels.get(depModel);
                if (dep != null) {
                    testCase.setDependency(dep);
                } else if (!depModel.startsWith("model-sets/")) {
                    Path modelPath = testRoot.resolve(depModel);
                    if (!Files.exists(modelPath)) {
                        System.err.println("Warning: Missing dependency '" + depModel
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
