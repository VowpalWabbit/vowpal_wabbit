package vowpalWabbit.responses;

import java.util.Arrays;

public class DecisionScores {
    private final ActionScores[] decisionScores;

    public DecisionScores(final ActionScores[] decisionScores) {
        this.decisionScores = decisionScores;
    }

    public ActionScores[] getDecisionScores() {
        return decisionScores;
    }

    @Override
    public String toString() {
        return "DecisionScores{" +
                "decisionScores=" + Arrays.toString(decisionScores) +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        DecisionScores that = (DecisionScores) o;

        return Arrays.equals(decisionScores, that.decisionScores);
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(decisionScores);
    }
}
