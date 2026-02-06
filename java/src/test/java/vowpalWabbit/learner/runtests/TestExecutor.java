package vowpalWabbit.learner.runtests;

import vowpalWabbit.learner.*;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.GZIPInputStream;

/**
 * Executes VW test cases using the Java JNI bindings.
 */
public class TestExecutor {
    private static final double DEFAULT_EPSILON = 1e-3;

    private final Path testRoot;
    private final Path workDir;
    private final Map<String, Path> modelCache = new HashMap<>();

    public TestExecutor(Path testRoot, Path workDir) {
        this.testRoot = testRoot;
        this.workDir = workDir;
    }

    /**
     * Execute a test case and all its dependencies.
     */
    public void executeTest(TestCase testCase) throws Exception {
        for (TestCase tc : testCase.inDependencyOrder()) {
            executeSingleTest(tc);
        }
    }

    /**
     * Execute a single test case (without dependencies).
     */
    private void executeSingleTest(TestCase tc) throws Exception {
        if (tc.getInputData().isEmpty()) {
            return;
        }

        Path inputPath = resolveInputFile(tc.getInputData());
        if (inputPath == null || !Files.exists(inputPath)) {
            throw new IllegalStateException("Input file not found: " + tc.getInputData() + " for test " + tc.getId());
        }

        if (tc.isMultiPass()) {
            executeMultiPassTest(tc, inputPath);
        } else {
            executeSinglePassTest(tc, inputPath);
        }
    }

    /**
     * Execute a multi-pass test using the VW driver.
     * The driver starts the parser which reads the data file (-d), handles
     * caching, and processes all passes automatically. This matches the C#
     * bindings' ExecuteTestWithDriver() approach and correctly handles both
     * single-line and multiline learners (e.g., cb_adf).
     */
    private void executeMultiPassTest(TestCase tc, Path inputPath) throws Exception {
        String args = prepareMultiPassArguments(tc, inputPath);

        // Ensure output directories exist before VW tries to write
        if (tc.hasFinalRegressor()) {
            Path modelPath = workDir.resolve(tc.getFinalRegressor());
            Files.createDirectories(modelPath.getParent());
        }

        // Let the driver handle all passes. close() will call finish()
        // which saves the model via -f. runDriver() sets numpasses=1
        // so performRemainingPasses() in close() is a no-op.
        try (VWLearner learner = createLearner(args)) {
            learner.runDriver();
        }

        // Register model in cache for dependent tests
        if (tc.hasFinalRegressor()) {
            Path modelPath = workDir.resolve(tc.getFinalRegressor());
            if (Files.exists(modelPath)) {
                modelCache.put(tc.getFinalRegressor(), modelPath);
            }
        }
    }

    /**
     * Execute a single-pass test by feeding examples programmatically.
     */
    private void executeSinglePassTest(TestCase tc, Path inputPath) throws Exception {
        String args = prepareSinglePassArguments(tc);

        // Load reference predictions if available
        String[] expectedPredictions = null;
        if (!tc.getPredictFile().isEmpty()) {
            Path predictRefPath = testRoot.resolve(tc.getPredictFile());
            if (Files.exists(predictRefPath)) {
                expectedPredictions = Files.readAllLines(predictRefPath, StandardCharsets.UTF_8)
                    .toArray(new String[0]);
            }
        }

        // Execute the test
        try (VWLearner learner = createLearner(args)) {
            // Query VW's native learner for multiline support. Calling learn(String[])
            // on non-multiline learners causes C++ std::terminate → JVM SIGABRT crash.
            if (learner.isMultiline()) {
                executeMultiline(learner, inputPath, tc.isTestOnly());
            } else {
                executeSingleLine(learner, inputPath, tc.isTestOnly(), expectedPredictions);
            }

            // Save model if needed
            if (tc.hasFinalRegressor()) {
                Path modelPath = workDir.resolve(tc.getFinalRegressor());
                Files.createDirectories(modelPath.getParent());
                learner.saveModel(modelPath.toFile());
                modelCache.put(tc.getFinalRegressor(), modelPath);
            }
        }
    }

    /**
     * Prepare arguments for multi-pass tests.
     * Keep -d, -c, --cache, -p, -f so the driver can handle everything.
     */
    private String prepareMultiPassArguments(TestCase tc, Path inputPath) {
        String args = tc.getArguments();

        // Resolve -d to absolute path
        args = args.replaceAll("-d\\s+\\S+", "-d " + inputPath.toString());

        // Resolve -i (initial regressor) path
        if (tc.hasInitialRegressor()) {
            args = resolveModelArg(args, "-i", tc.getInitialRegressor());
        }

        // Resolve -f (final regressor) path to work directory
        if (tc.hasFinalRegressor()) {
            Path modelPath = workDir.resolve(tc.getFinalRegressor());
            args = args.replaceAll("-f\\s+\\S+", "-f " + modelPath.toString());
        }

        // Redirect -p (predictions) to work directory
        Path predictOutPath = workDir.resolve("test_" + tc.getId() + ".predict");
        if (args.matches(".*-p\\s+\\S+.*")) {
            args = args.replaceAll("-p\\s+\\S+", "-p " + predictOutPath.toString());
        }

        // Redirect --cache_file to work directory, or add one if -c/--cache is present
        if (args.matches(".*--cache_file\\s+\\S+.*")) {
            Path cacheFile = workDir.resolve("test_" + tc.getId() + ".cache");
            args = args.replaceAll("--cache_file\\s+\\S+", "--cache_file " + cacheFile.toString());
        } else if (args.matches(".*(^| )-c( |$).*") || args.contains("--cache")) {
            Path cacheFile = workDir.resolve("test_" + tc.getId() + ".cache");
            args = args + " --cache_file " + cacheFile.toString();
        }

        // Resolve remaining file path arguments relative to testRoot
        args = resolveTestRootPaths(args);

        // Redirect output file arguments to workDir
        args = resolveOutputPaths(args);

        if (!args.contains("--quiet")) {
            args = args + " --quiet";
        }

        // Strip --audit to prevent stdout pollution (corrupts surefire pipe)
        args = args.replaceAll("(^| )--audit( |$)", "$1$2");

        return args.trim().replaceAll("\\s+", " ");
    }

    /**
     * Prepare arguments for single-pass tests.
     * Strip -d, -p, -c (examples fed programmatically).
     */
    private String prepareSinglePassArguments(TestCase tc) {
        String args = tc.getArguments();

        // Remove -d (examples fed programmatically)
        args = args.replaceAll("-d\\s+\\S+", "");

        // Remove -p (predictions validated inline)
        args = args.replaceAll("-p\\s+\\S+", "");

        // Resolve -i (initial regressor) path
        if (tc.hasInitialRegressor()) {
            args = resolveModelArg(args, "-i", tc.getInitialRegressor());
        }

        // Remove -f (model saved explicitly via saveModel() in executeSinglePassTest).
        // Keeping -f would cause a double-save: once via saveModel() and once during
        // finish(), which breaks reductions like automl that modify state in pre_save_load.
        args = args.replaceAll("-f\\s+\\S+", "");

        // Resolve --feature_mask model path
        args = resolveModelOption(args, "--feature_mask");

        // Resolve remaining file path arguments relative to testRoot
        args = resolveTestRootPaths(args);

        // Redirect output file arguments to workDir
        args = resolveOutputPaths(args);

        if (!args.contains("--quiet")) {
            args = args + " --quiet";
        }

        // Strip --audit to prevent stdout pollution (corrupts surefire pipe)
        args = args.replaceAll("(^| )--audit( |$)", "$1$2");

        // Remove cache options (not needed for single-pass programmatic input)
        args = args.replaceAll("(^| )-c( |$)", "$1$2");
        args = args.replaceAll("(^| )--cache( |$)", "$1$2");
        args = args.replaceAll("--cache_file\\s+\\S+", "");

        return args.trim().replaceAll("\\s+", " ");
    }

    /**
     * Resolve a model argument (-i or --feature_mask) from modelCache or testRoot.
     */
    private String resolveModelArg(String args, String option, String modelName) {
        Path modelPath = modelCache.get(modelName);
        if (modelPath == null) {
            modelPath = testRoot.resolve(modelName);
        }
        return args.replaceAll(Pattern.quote(option) + "\\s+\\S+",
            Matcher.quoteReplacement(option + " " + modelPath.toString()));
    }

    /**
     * Resolve a named model option (like --feature_mask) from modelCache or testRoot.
     */
    private String resolveModelOption(String args, String option) {
        Pattern p = Pattern.compile(Pattern.quote(option) + "\\s+(\\S+)");
        Matcher m = p.matcher(args);
        if (m.find()) {
            String modelName = m.group(1);
            Path modelPath = modelCache.get(modelName);
            if (modelPath == null) {
                modelPath = testRoot.resolve(modelName);
            }
            args = args.substring(0, m.start()) + option + " " + modelPath.toString() + args.substring(m.end());
        }
        return args;
    }

    /**
     * Resolve input file path arguments relative to testRoot.
     */
    private String resolveTestRootPaths(String args) {
        String[] inputOptions = {
            "--dictionary_path", "--input_feature_regularizer",
            "--search_allowed_transitions"
        };
        for (String opt : inputOptions) {
            Pattern p = Pattern.compile(Pattern.quote(opt) + "\\s+(\\S+)");
            Matcher m = p.matcher(args);
            if (m.find()) {
                String value = m.group(1);
                if (!value.startsWith("/")) {
                    Path resolved = testRoot.resolve(value);
                    args = args.substring(0, m.start()) + opt + " " + resolved.toString() + args.substring(m.end());
                }
            }
        }
        return args;
    }

    /**
     * Resolve output file path arguments to workDir.
     */
    private String resolveOutputPaths(String args) {
        String[] outputOptions = {"--readable_model", "--invert_hash"};
        for (String opt : outputOptions) {
            Pattern p = Pattern.compile(Pattern.quote(opt) + "\\s+(\\S+)");
            Matcher m = p.matcher(args);
            if (m.find()) {
                String value = m.group(1);
                if (!value.startsWith("/")) {
                    Path resolved = workDir.resolve(value);
                    args = args.substring(0, m.start()) + opt + " " + resolved.toString() + args.substring(m.end());
                }
            }
        }
        return args;
    }

    private Path resolveInputFile(String inputFile) {
        if (inputFile == null || inputFile.isEmpty()) {
            return null;
        }
        return testRoot.resolve(inputFile);
    }

    private VWLearner createLearner(String args) {
        return VWLearners.create(args);
    }

    /**
     * Check if input file contains multiline examples (blank line separators).
     */
    private boolean isMultilineData(Path inputPath) throws IOException {
        try (BufferedReader reader = openReader(inputPath)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.trim().isEmpty()) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Execute test with single-line examples.
     */
    private void executeSingleLine(VWLearner learner, Path inputPath,
                                   boolean testOnly, String[] expectedPredictions) throws IOException {
        int lineNr = 0;
        try (BufferedReader reader = openReader(inputPath)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.trim().isEmpty()) {
                    continue;
                }

                Object prediction;
                if (testOnly) {
                    prediction = predict(learner, line);
                } else {
                    prediction = learn(learner, line);
                }

                if (expectedPredictions != null && lineNr < expectedPredictions.length) {
                    validatePrediction(prediction, expectedPredictions[lineNr], lineNr);
                }

                lineNr++;
            }
        }
    }

    /**
     * Execute test with multiline examples.
     */
    private void executeMultiline(VWLearner learner, Path inputPath, boolean testOnly) throws IOException {
        List<String> lines = new ArrayList<>();

        try (BufferedReader reader = openReader(inputPath)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.trim().isEmpty()) {
                    if (!lines.isEmpty()) {
                        String[] examples = lines.toArray(new String[0]);
                        if (testOnly) {
                            predictMultiline(learner, examples);
                        } else {
                            learnMultiline(learner, examples);
                        }
                        lines.clear();
                    }
                } else {
                    lines.add(line);
                }
            }

            // Process remaining lines
            if (!lines.isEmpty()) {
                String[] examples = lines.toArray(new String[0]);
                if (testOnly) {
                    predictMultiline(learner, examples);
                } else {
                    learnMultiline(learner, examples);
                }
            }
        }
    }

    private Object learn(VWLearner learner, String example) {
        if (learner instanceof VWScalarLearner) return ((VWScalarLearner) learner).learn(example);
        if (learner instanceof VWScalarsLearner) return ((VWScalarsLearner) learner).learn(example);
        if (learner instanceof VWMulticlassLearner) return ((VWMulticlassLearner) learner).learn(example);
        if (learner instanceof VWMultilabelsLearner) return ((VWMultilabelsLearner) learner).learn(example);
        if (learner instanceof VWActionScoresLearner) return ((VWActionScoresLearner) learner).learn(example);
        if (learner instanceof VWActionProbsLearner) return ((VWActionProbsLearner) learner).learn(example);
        if (learner instanceof VWProbLearner) return ((VWProbLearner) learner).learn(example);
        if (learner instanceof VWCCBLearner) return ((VWCCBLearner) learner).learn(example);
        if (learner instanceof VWNoPredLearner) return ((VWNoPredLearner) learner).learn(example);
        if (learner instanceof VWActionPDFValueLearner) return ((VWActionPDFValueLearner) learner).learn(example);
        if (learner instanceof VWPDFLearner) return ((VWPDFLearner) learner).learn(example);
        if (learner instanceof VWActiveMulticlassLearner) return ((VWActiveMulticlassLearner) learner).learn(example);
        throw new IllegalStateException("Unsupported learner type: " + learner.getClass().getName());
    }

    private Object predict(VWLearner learner, String example) {
        if (learner instanceof VWScalarLearner) return ((VWScalarLearner) learner).predict(example);
        if (learner instanceof VWScalarsLearner) return ((VWScalarsLearner) learner).predict(example);
        if (learner instanceof VWMulticlassLearner) return ((VWMulticlassLearner) learner).predict(example);
        if (learner instanceof VWMultilabelsLearner) return ((VWMultilabelsLearner) learner).predict(example);
        if (learner instanceof VWActionScoresLearner) return ((VWActionScoresLearner) learner).predict(example);
        if (learner instanceof VWActionProbsLearner) return ((VWActionProbsLearner) learner).predict(example);
        if (learner instanceof VWProbLearner) return ((VWProbLearner) learner).predict(example);
        if (learner instanceof VWCCBLearner) return ((VWCCBLearner) learner).predict(example);
        if (learner instanceof VWNoPredLearner) return ((VWNoPredLearner) learner).predict(example);
        if (learner instanceof VWActionPDFValueLearner) return ((VWActionPDFValueLearner) learner).predict(example);
        if (learner instanceof VWPDFLearner) return ((VWPDFLearner) learner).predict(example);
        if (learner instanceof VWActiveMulticlassLearner) return ((VWActiveMulticlassLearner) learner).predict(example);
        throw new IllegalStateException("Unsupported learner type: " + learner.getClass().getName());
    }

    private Object learnMultiline(VWLearner learner, String[] examples) {
        if (learner instanceof VWScalarLearner) return ((VWScalarLearner) learner).learn(examples);
        if (learner instanceof VWScalarsLearner) return ((VWScalarsLearner) learner).learn(examples);
        if (learner instanceof VWMulticlassLearner) return ((VWMulticlassLearner) learner).learn(examples);
        if (learner instanceof VWMultilabelsLearner) return ((VWMultilabelsLearner) learner).learn(examples);
        if (learner instanceof VWActionScoresLearner) return ((VWActionScoresLearner) learner).learn(examples);
        if (learner instanceof VWActionProbsLearner) return ((VWActionProbsLearner) learner).learn(examples);
        if (learner instanceof VWProbLearner) return ((VWProbLearner) learner).learn(examples);
        if (learner instanceof VWCCBLearner) return ((VWCCBLearner) learner).learn(examples);
        if (learner instanceof VWNoPredLearner) return ((VWNoPredLearner) learner).learn(examples);
        if (learner instanceof VWActionPDFValueLearner) return ((VWActionPDFValueLearner) learner).learn(examples);
        if (learner instanceof VWPDFLearner) return ((VWPDFLearner) learner).learn(examples);
        if (learner instanceof VWActiveMulticlassLearner) return ((VWActiveMulticlassLearner) learner).learn(examples);
        throw new IllegalStateException("Unsupported learner type: " + learner.getClass().getName());
    }

    private Object predictMultiline(VWLearner learner, String[] examples) {
        if (learner instanceof VWScalarLearner) return ((VWScalarLearner) learner).predict(examples);
        if (learner instanceof VWScalarsLearner) return ((VWScalarsLearner) learner).predict(examples);
        if (learner instanceof VWMulticlassLearner) return ((VWMulticlassLearner) learner).predict(examples);
        if (learner instanceof VWMultilabelsLearner) return ((VWMultilabelsLearner) learner).predict(examples);
        if (learner instanceof VWActionScoresLearner) return ((VWActionScoresLearner) learner).predict(examples);
        if (learner instanceof VWActionProbsLearner) return ((VWActionProbsLearner) learner).predict(examples);
        if (learner instanceof VWProbLearner) return ((VWProbLearner) learner).predict(examples);
        if (learner instanceof VWCCBLearner) return ((VWCCBLearner) learner).predict(examples);
        if (learner instanceof VWNoPredLearner) return ((VWNoPredLearner) learner).predict(examples);
        if (learner instanceof VWActionPDFValueLearner) return ((VWActionPDFValueLearner) learner).predict(examples);
        if (learner instanceof VWPDFLearner) return ((VWPDFLearner) learner).predict(examples);
        if (learner instanceof VWActiveMulticlassLearner) return ((VWActiveMulticlassLearner) learner).predict(examples);
        throw new IllegalStateException("Unsupported learner type: " + learner.getClass().getName());
    }

    /**
     * Validate prediction against expected value.
     */
    private void validatePrediction(Object actual, String expected, int lineNr) {
        if (actual == null || expected == null || expected.trim().isEmpty()) {
            return;
        }

        try {
            if (actual instanceof Float || actual instanceof Double) {
                double actualValue = ((Number) actual).doubleValue();
                String[] parts = expected.trim().split("\\s+");
                double expectedValue = Double.parseDouble(parts[0]);
                if (!fuzzyEquals(actualValue, expectedValue, DEFAULT_EPSILON)) {
                    throw new AssertionError(String.format(
                        "Prediction mismatch at line %d: expected %f, got %f",
                        lineNr, expectedValue, actualValue));
                }
            } else if (actual instanceof Integer) {
                int actualValue = (Integer) actual;
                int expectedValue = Integer.parseInt(expected.trim().split("\\s+")[0]);
                if (actualValue != expectedValue) {
                    throw new AssertionError(String.format(
                        "Prediction mismatch at line %d: expected %d, got %d",
                        lineNr, expectedValue, actualValue));
                }
            }
            // For complex types (ActionScores, etc.) we skip validation for now
        } catch (NumberFormatException e) {
            // Skip validation if expected format doesn't match
        }
    }

    /**
     * Validate prediction file against reference file.
     */
    private void validatePredictionFile(Path referencePath, Path actualPath) throws IOException {
        List<String> expected = Files.readAllLines(referencePath, StandardCharsets.UTF_8);
        List<String> actual = Files.readAllLines(actualPath, StandardCharsets.UTF_8);

        int minLines = Math.min(expected.size(), actual.size());
        for (int i = 0; i < minLines; i++) {
            String expLine = expected.get(i).trim();
            String actLine = actual.get(i).trim();
            if (expLine.isEmpty() || actLine.isEmpty()) continue;

            try {
                String[] expParts = expLine.split("\\s+");
                String[] actParts = actLine.split("\\s+");
                int minParts = Math.min(expParts.length, actParts.length);
                for (int j = 0; j < minParts; j++) {
                    double expVal = Double.parseDouble(expParts[j]);
                    double actVal = Double.parseDouble(actParts[j]);
                    if (!fuzzyEquals(expVal, actVal, DEFAULT_EPSILON)) {
                        throw new AssertionError(String.format(
                            "Prediction mismatch at line %d col %d: expected %s, got %s",
                            i, j, expParts[j], actParts[j]));
                    }
                }
            } catch (NumberFormatException e) {
                if (!expLine.equals(actLine)) {
                    throw new AssertionError(String.format(
                        "Prediction mismatch at line %d: expected '%s', got '%s'",
                        i, expLine, actLine));
                }
            }
        }
    }

    private boolean fuzzyEquals(double a, double b, double epsilon) {
        if (Double.isNaN(a) && Double.isNaN(b)) return true;
        if (Double.isInfinite(a) && Double.isInfinite(b)) return a == b;
        return Math.abs(a - b) <= epsilon * Math.max(1.0, Math.max(Math.abs(a), Math.abs(b)));
    }

    private BufferedReader openReader(Path path) throws IOException {
        InputStream is = Files.newInputStream(path);
        if (path.toString().endsWith(".gz")) {
            is = new GZIPInputStream(is);
        }
        return new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8));
    }
}
