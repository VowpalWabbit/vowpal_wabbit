package vowpalWabbit.learner.runtests;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Processed test case with parsed arguments and resolved dependencies.
 */
public class TestCase {
    private final int id;
    private final String description;
    private final String arguments;
    private final String inputData;
    private final String initialRegressor;
    private final String finalRegressor;
    private final String stderr;
    private final String predictFile;
    private TestCase dependency;

    public TestCase(TestData data) {
        this.id = data.id;
        this.description = data.desc != null ? data.desc : "";
        this.arguments = data.vw_command != null ? data.vw_command : "";
        this.inputData = matchArgument(arguments, "-d");
        this.initialRegressor = matchArgument(arguments, "-i");
        this.finalRegressor = matchArgument(arguments, "-f");

        // Parse diff_files for stderr and predict file
        String stderrTemp = "";
        String predictTemp = "";
        if (data.diff_files != null) {
            for (Map.Entry<String, String> entry : data.diff_files.entrySet()) {
                if ("stderr".equals(entry.getKey())) {
                    stderrTemp = entry.getValue();
                } else if (entry.getKey().endsWith(".predict")) {
                    predictTemp = entry.getValue();
                }
            }
        }
        this.stderr = stderrTemp;
        this.predictFile = predictTemp;
    }

    public int getId() {
        return id;
    }

    public String getDescription() {
        return description;
    }

    public String getArguments() {
        return arguments;
    }

    public String getInputData() {
        return inputData;
    }

    public String getInitialRegressor() {
        return initialRegressor;
    }

    public String getFinalRegressor() {
        return finalRegressor;
    }

    public String getStderr() {
        return stderr;
    }

    public String getPredictFile() {
        return predictFile;
    }

    public TestCase getDependency() {
        return dependency;
    }

    public void setDependency(TestCase dependency) {
        this.dependency = dependency;
    }

    public boolean hasDependency() {
        return dependency != null;
    }

    public boolean hasInitialRegressor() {
        return initialRegressor != null && !initialRegressor.isEmpty();
    }

    public boolean hasFinalRegressor() {
        return finalRegressor != null && !finalRegressor.isEmpty();
    }

    public boolean isTestOnly() {
        return arguments.contains("-t ");
    }

    public boolean isMultiPass() {
        return arguments.contains("--passes");
    }

    public boolean isJson() {
        return arguments.contains("--json") || arguments.contains("--dsjson");
    }

    /**
     * Returns this test case and all its dependencies in execution order
     * (dependencies first).
     */
    public List<TestCase> inDependencyOrder() {
        List<TestCase> tests = new ArrayList<>();
        TestCase current = this;
        while (current != null) {
            tests.add(current);
            current = current.dependency;
        }
        Collections.reverse(tests);
        return tests;
    }

    /**
     * Extract argument value from VW command string.
     */
    private static String matchArgument(String args, String option) {
        if (args == null || args.isEmpty()) {
            return "";
        }
        Pattern pattern = Pattern.compile(Pattern.quote(option) + "\\s+(\\S+)");
        Matcher matcher = pattern.matcher(args);
        return matcher.find() ? matcher.group(1) : "";
    }

    @Override
    public String toString() {
        return "Test " + id + ": " + description;
    }
}
