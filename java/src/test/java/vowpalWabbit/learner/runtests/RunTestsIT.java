package vowpalWabbit.learner.runtests;

import org.junit.jupiter.api.*;
import org.junit.jupiter.api.io.TempDir;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

import java.nio.file.Path;
import java.util.List;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assumptions.*;

/**
 * Integration tests that run VW test cases from core.vwtest.json through the Java JNI bindings.
 *
 * <p>This mirrors the test infrastructure used by C# bindings, executing the same
 * test suite to ensure Java API compatibility.</p>
 *
 * <p>To run these tests, the test root directory must be accessible. The loader
 * searches for core.vwtest.json in common locations or uses VW_TEST_ROOT env var.</p>
 */
@DisplayName("VW RunTests (core.vwtest.json)")
public class RunTestsIT {

    private static Path testRoot;
    private static List<TestCase> allTestCases;
    private static boolean initialized = false;
    private static String initError = null;

    @TempDir
    Path tempDir;

    @BeforeAll
    static void loadTests() {
        try {
            testRoot = TestLoader.findTestRoot();
            TestLoader loader = new TestLoader(testRoot);
            allTestCases = loader.loadTests();
            initialized = true;
            System.out.println("Loaded " + allTestCases.size() + " test cases from " + testRoot);
        } catch (Exception e) {
            initError = e.getMessage();
            System.err.println("Failed to load tests: " + initError);
        }
    }

    /**
     * Provides test case arguments for the parameterized test.
     */
    static Stream<Arguments> testCaseProvider() {
        if (!initialized || allTestCases == null) {
            return Stream.empty();
        }
        return allTestCases.stream()
            .map(tc -> Arguments.of(tc.getId(), tc.getDescription(), tc));
    }

    @ParameterizedTest(name = "Test {0}: {1}")
    @MethodSource("testCaseProvider")
    @DisplayName("RunTest")
    void runTest(int testId, String description, TestCase testCase) throws Exception {
        // Skip if initialization failed
        assumeTrue(initialized, "Test initialization failed: " + initError);

        // Skip tests in the skip list
        assumeFalse(TestLoader.shouldSkip(testId),
            "Test " + testId + " is in skip list");

        // Skip JSON tests (not supported in Java JNI)
        assumeFalse(testCase.isJson(),
            "Test " + testId + " uses JSON format (not supported)");

        // Execute the test
        TestExecutor executor = new TestExecutor(testRoot, tempDir);
        try {
            executor.executeTest(testCase);
        } catch (Throwable e) {
            fail("Test " + testId + " failed: " + e.getMessage(), e);
        }
    }

    /**
     * Simple smoke test to verify the test infrastructure works.
     */
    @Test
    @DisplayName("Test infrastructure smoke test")
    void smokeTest() {
        assumeTrue(initialized, "Test initialization failed: " + initError);
        assertNotNull(testRoot, "Test root should be set");
        assertNotNull(allTestCases, "Test cases should be loaded");
        assertTrue(allTestCases.size() > 0, "Should have test cases");

        // Count skipped vs runnable
        long skipped = allTestCases.stream()
            .filter(tc -> TestLoader.shouldSkip(tc.getId()) || tc.isJson())
            .count();
        long runnable = allTestCases.size() - skipped;

        System.out.println("Total tests: " + allTestCases.size());
        System.out.println("Skipped: " + skipped);
        System.out.println("Runnable: " + runnable);

        assertTrue(runnable > 100, "Should have at least 100 runnable tests");
    }

    /**
     * Test a simple scalar learning test case (Test 3).
     */
    @Test
    @DisplayName("Specific test: scalar learning (Test 3)")
    void testScalarLearning() throws Exception {
        assumeTrue(initialized, "Test initialization failed: " + initError);

        TestCase tc = findTestCase(3);
        assumeTrue(tc != null, "Test case 3 not found");
        assumeFalse(TestLoader.shouldSkip(3), "Test 3 is in skip list");

        TestExecutor executor = new TestExecutor(testRoot, tempDir);
        executor.executeTest(tc);
    }

    /**
     * Test multiclass classification (Test 11 - OAA).
     */
    @Test
    @DisplayName("Specific test: multiclass OAA (Test 11)")
    void testMulticlassOAA() throws Exception {
        assumeTrue(initialized, "Test initialization failed: " + initError);

        TestCase tc = findTestCase(11);
        assumeTrue(tc != null, "Test case 11 not found");
        assumeFalse(TestLoader.shouldSkip(11), "Test 11 is in skip list");

        TestExecutor executor = new TestExecutor(testRoot, tempDir);
        executor.executeTest(tc);
    }

    /**
     * Test with model save/load (Tests 1 and 2).
     */
    @Test
    @DisplayName("Specific test: model save/load (Tests 1-2)")
    void testModelSaveLoad() throws Exception {
        assumeTrue(initialized, "Test initialization failed: " + initError);

        // Test 2 depends on Test 1
        TestCase tc2 = findTestCase(2);
        assumeTrue(tc2 != null, "Test case 2 not found");
        assumeFalse(TestLoader.shouldSkip(1), "Test 1 is in skip list");
        assumeFalse(TestLoader.shouldSkip(2), "Test 2 is in skip list");

        TestExecutor executor = new TestExecutor(testRoot, tempDir);
        // This should execute both test 1 and test 2 in order
        executor.executeTest(tc2);
    }

    private TestCase findTestCase(int id) {
        if (allTestCases == null) return null;
        return allTestCases.stream()
            .filter(tc -> tc.getId() == id)
            .findFirst()
            .orElse(null);
    }
}
