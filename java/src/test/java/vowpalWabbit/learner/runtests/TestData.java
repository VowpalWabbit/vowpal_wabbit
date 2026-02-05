package vowpalWabbit.learner.runtests;

import java.util.List;
import java.util.Map;

/**
 * Represents a single test case from core.vwtest.json
 */
public class TestData {
    public int id;
    public String desc;
    public String vw_command;
    public String bash_command;
    public Map<String, String> diff_files;
    public List<String> input_files;
    public List<Integer> depends_on;

    public boolean hasVwCommand() {
        return vw_command != null && !vw_command.isEmpty();
    }

    public boolean isBashTest() {
        return bash_command != null && !bash_command.isEmpty();
    }

    public boolean shouldSkipForJava() {
        return desc != null && desc.contains("SkipJava");
    }

    @Override
    public String toString() {
        return "Test " + id + ": " + (desc != null ? desc : "(no description)");
    }
}
