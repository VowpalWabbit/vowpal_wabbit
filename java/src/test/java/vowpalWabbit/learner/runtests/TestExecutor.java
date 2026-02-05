package vowpalWabbit.learner.runtests;

import vowpalWabbit.learner.*;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.zip.GZIPInputStream;

/**
 * Executes VW test cases using the Java JNI bindings.
 */
public class TestExecutor {
    private static final double DEFAULT_EPSILON = 5e-4;

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
        String args = prepareArguments(tc);

        // Determine input file
        Path inputPath = resolveInputFile(tc.getInputData());
        if (inputPath == null || !Files.exists(inputPath)) {
            throw new IllegalStateException("Input file not found: " + tc.getInputData() + " for test " + tc.getId());
        }

        // Load reference predictions if available
        String[] expectedPredictions = null;
        if (!tc.getPredictFile().isEmpty()) {
            Path predictRefPath = testRoot.resolve(tc.getPredictFile());
            if (Files.exists(predictRefPath)) {
                expectedPredictions = Files.readAllLines(predictRefPath, StandardCharsets.UTF_8)
                    .toArray(new String[0]);
            }
        }

        // Detect if multiline format
        boolean isMultiline = isMultilineData(inputPath);

        // Execute the test
        try (VWLearner learner = createLearner(args)) {
            if (isMultiline) {
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
     * Prepare VW arguments, resolving file paths.
     */
    private String prepareArguments(TestCase tc) {
        String args = tc.getArguments();

        // Remove -d argument (we'll feed examples programmatically)
        args = args.replaceAll("-d\\s+\\S+", "");

        // Remove -p argument (we validate predictions differently)
        args = args.replaceAll("-p\\s+\\S+", "");

        // Resolve -i (initial regressor) path
        if (tc.hasInitialRegressor()) {
            String modelName = tc.getInitialRegressor();
            Path modelPath = modelCache.get(modelName);
            if (modelPath == null) {
                modelPath = testRoot.resolve(modelName);
            }
            args = args.replaceAll("-i\\s+\\S+", "-i " + modelPath.toString());
        }

        // Resolve -f (final regressor) path
        if (tc.hasFinalRegressor()) {
            Path modelPath = workDir.resolve(tc.getFinalRegressor());
            args = args.replaceAll("-f\\s+\\S+", "-f " + modelPath.toString());
        }

        // Add quiet mode to suppress output
        if (!args.contains("--quiet")) {
            args = args + " --quiet";
        }

        // Remove cache options (we don't use caching in tests)
        // Must match -c and --cache as whole tokens to avoid stripping -c from --csoaa, --cubic, etc.
        args = args.replaceAll("(^| )-c( |$)", "$1$2");
        args = args.replaceAll("(^| )--cache( |$)", "$1$2");
        args = args.replaceAll("--cache_file\\s+\\S+", "");

        // Remove holdout_off as it may cause issues
        // args = args.replaceAll("--holdout_off", "");

        return args.trim().replaceAll("\\s+", " ");
    }

    /**
     * Resolve input file path.
     */
    private Path resolveInputFile(String inputFile) {
        if (inputFile == null || inputFile.isEmpty()) {
            return null;
        }
        return testRoot.resolve(inputFile);
    }

    /**
     * Create appropriate VW learner based on arguments.
     */
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

                // Validate prediction if reference available
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
                        if (testOnly) {
                            predictMultiline(learner, lines);
                        } else {
                            learnMultiline(learner, lines);
                        }
                        lines.clear();
                    }
                } else {
                    lines.add(line);
                }
            }

            // Process remaining lines
            if (!lines.isEmpty()) {
                if (testOnly) {
                    predictMultiline(learner, lines);
                } else {
                    learnMultiline(learner, lines);
                }
            }
        }
    }

    /**
     * Learn from a single example.
     */
    private Object learn(VWLearner learner, String example) {
        if (learner instanceof VWScalarLearner) {
            return ((VWScalarLearner) learner).learn(example);
        } else if (learner instanceof VWMulticlassLearner) {
            return ((VWMulticlassLearner) learner).learn(example);
        } else if (learner instanceof VWActionScoresLearner) {
            return ((VWActionScoresLearner) learner).learn(example);
        } else if (learner instanceof VWActionProbsLearner) {
            return ((VWActionProbsLearner) learner).learn(example);
        } else if (learner instanceof VWMultilabelsLearner) {
            return ((VWMultilabelsLearner) learner).learn(example);
        } else {
            // Generic fallback
            ((VWScalarLearner) learner).learn(example);
            return null;
        }
    }

    /**
     * Predict from a single example.
     */
    private Object predict(VWLearner learner, String example) {
        if (learner instanceof VWScalarLearner) {
            return ((VWScalarLearner) learner).predict(example);
        } else if (learner instanceof VWMulticlassLearner) {
            return ((VWMulticlassLearner) learner).predict(example);
        } else if (learner instanceof VWActionScoresLearner) {
            return ((VWActionScoresLearner) learner).predict(example);
        } else if (learner instanceof VWActionProbsLearner) {
            return ((VWActionProbsLearner) learner).predict(example);
        } else if (learner instanceof VWMultilabelsLearner) {
            return ((VWMultilabelsLearner) learner).predict(example);
        } else {
            return ((VWScalarLearner) learner).predict(example);
        }
    }

    /**
     * Learn from multiline example.
     */
    private void learnMultiline(VWLearner learner, List<String> lines) {
        String[] examples = lines.toArray(new String[0]);
        if (learner instanceof VWActionScoresLearner) {
            ((VWActionScoresLearner) learner).learn(examples);
        } else if (learner instanceof VWActionProbsLearner) {
            ((VWActionProbsLearner) learner).learn(examples);
        } else if (learner instanceof VWScalarsLearner) {
            ((VWScalarsLearner) learner).learn(examples);
        } else {
            // Fall back to learning each line individually
            for (String line : lines) {
                learn(learner, line);
            }
        }
    }

    /**
     * Predict from multiline example.
     */
    private Object predictMultiline(VWLearner learner, List<String> lines) {
        String[] examples = lines.toArray(new String[0]);
        if (learner instanceof VWActionScoresLearner) {
            return ((VWActionScoresLearner) learner).predict(examples);
        } else if (learner instanceof VWActionProbsLearner) {
            return ((VWActionProbsLearner) learner).predict(examples);
        } else if (learner instanceof VWScalarsLearner) {
            return ((VWScalarsLearner) learner).predict(examples);
        } else {
            // Fall back to predicting each line individually
            Object lastPrediction = null;
            for (String line : lines) {
                lastPrediction = predict(learner, line);
            }
            return lastPrediction;
        }
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
     * Fuzzy float comparison with epsilon tolerance.
     */
    private boolean fuzzyEquals(double a, double b, double epsilon) {
        if (Double.isNaN(a) && Double.isNaN(b)) return true;
        if (Double.isInfinite(a) && Double.isInfinite(b)) return a == b;
        return Math.abs(a - b) <= epsilon * Math.max(1.0, Math.max(Math.abs(a), Math.abs(b)));
    }

    /**
     * Open a reader for the input file, handling gzip compression.
     */
    private BufferedReader openReader(Path path) throws IOException {
        InputStream is = Files.newInputStream(path);
        if (path.toString().endsWith(".gz")) {
            is = new GZIPInputStream(is);
        }
        return new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8));
    }
}
