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
        // ===== Multi-pass tests (require cache files, not supported with programmatic input) =====
        1, 2, 9, 10, 11, 12, 15, 18, 19, 20, 21, 22, 23, 29, 35, 42, 43, 44, 45, 46,
        48, 49, 50, 51, 52, 57, 63, 64, 65, 67, 70, 79, 86, 87, 88, 93, 110, 123, 138,
        139, 141, 151, 155, 157, 164, 165, 185, 223, 224, 226, 290, 315, 342, 343, 344,
        384, 469, 470, 473, 474, 476, 482, 503, 504, 506, 510, 512, 518, 519, 520, 523,
        534, 535, 545, 547, 550, 552, 556, 557, 558, 559, 560, 561, 562, 563, 571, 574,
        582, 583, 618, 619, 620, 626, 627, 643, 648, 649, 650, 652, 653, 658, 667, 668,
        669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 682, 683, 684, 711, 713, 714,
        715, 716, 717, 718, 719,

        // ===== Crashes or known issues =====
        13, 25, 26, 32, 39, 40, 41, 59, 60, 61, 66, 68, 90,
        210, 258, 259, 260, 288, 303, 307, 349, 350, 356, 357, 358, 437, 479, 633, 706,

        // ===== Shell scripts for input/output =====
        91, 92, 95, 96, 98, 99, 118, 119, 120, 176, 177, 207, 208,

        // ===== Float delta issues =====
        14, 16, 17, 31, 33, 34, 53, 101, 102, 103, 105, 106, 111, 112,
        188, 412, 413, 414, 422, 423, 424, 541,

        // ===== Parser/examples tests =====
        71,

        // ===== Native JSON parsing not supported in Java JNI =====
        143, 144, 146, 158, 189, 202, 237, 312, 316, 318, 319, 324, 325, 326, 347, 348, 351,

        // ===== Bash script tests =====
        149, 152, 156, 193, 194, 217, 385,

        // ===== --onethread is a shell option, not available via library =====
        195, 275, 276, 511, 644,

        // ===== Cluster mode test =====
        203,

        // ===== Testing option usage =====
        204,

        // ===== DSJSON not supported =====
        216, 256, 299, 300, 306, 310, 311, 327, 328, 329, 330, 331, 367, 368, 396, 397, 398,
        405, 406, 407, 411, 415, 417, 456, 457, 458, 459, 460, 461, 462,

        // ===== Daemon mode shell script test not supported =====
        220,

        // ===== CATS prediction types not yet supported =====
        227, 229,

        // ===== Flatbuffer input tests =====
        239, 240, 241, 242, 243, 244, 245, 246,

        // ===== Unsupported prediction types (cb_explore_adf, classify, etc.) =====
        74, 75, 76, 77, 82, 85, 126, 127, 128, 129, 133, 134, 136, 137, 148, 159, 160,
        161, 166, 167, 168, 169, 170, 171, 172, 173, 186, 196, 198, 209, 219, 221, 222,
        228, 231, 232, 238, 247, 248, 249, 250, 251, 252, 253, 257, 261, 262, 263, 264,
        265, 266, 267, 268, 269, 270, 277, 278, 279, 280, 281, 282, 283, 284, 301, 305,
        313, 320, 321, 332, 337, 339, 408, 419, 420, 421, 425, 426, 428, 439, 440, 441,
        442, 444, 445, 446, 447, 448, 449, 450, 453, 463, 465, 466, 477, 478, 502, 508,
        516, 533, 591, 606, 607, 617, 622, 623, 628, 629, 631, 632, 634, 635, 636, 637,
        638, 689,

        // ===== Boolean flag parsing issues (--cache/--audit/--help with values) =====
        104, 115, 178, 179, 180, 181, 182, 183, 197, 199, 225, 230, 255, 286, 287, 289,
        291, 292, 302, 340, 341, 361, 362, 363, 364, 369, 370, 371, 372, 483, 546, 548,
        551,

        // ===== VWScalarsLearner/VWScalarLearner type mismatch =====
        107, 114, 117, 121, 122, 145,

        // ===== Multiline learner with single-line examples =====
        109, 515, 517, 592, 621,

        // ===== Input file not found =====
        273, 274, 381, 387, 388,

        // ===== LDA (unsupported return type) =====
        472, 505, 529, 624, 625, 651,

        // ===== No data file =====
        383, 389, 390, 391, 392, 393,

        // ===== Positional args =====
        400, 404,

        // ===== Empty lines not supported =====
        464,

        // ===== Option parsing issues (--csoaa, --cs_active, --coin, --cubic, --csv, etc.) =====
        47, 84, 89, 108, 116, 162, 163, 190, 296, 352, 353, 359, 360, 373,
        374, 375, 376, 377, 378, 386, 475, 527, 530, 543, 544, 555, 566, 568, 573, 593,
        599, 611, 630, 639, 640, 646, 690, 691, 692, 693, 694, 709, 710
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
