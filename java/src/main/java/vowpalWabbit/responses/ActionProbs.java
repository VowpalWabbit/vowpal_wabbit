package vowpalWabbit.responses;

import java.io.Serializable;
import java.util.Arrays;

/**
 * Created by jmorra on 8/12/16.
 */
public class ActionProbs implements Serializable {

    // Although this is modifiable it is not intended to be updated by the user.  This data structure mimics the
    // C data structure.
    private final ActionProb[] actionProbs;

    public ActionProbs(final ActionProb[] actionProbs) {
        this.actionProbs = actionProbs;
    }

    public ActionProb[] getActionProbs() {
        return actionProbs;
    }

    @Override
    public String toString() {
        return "ActionProbs{" +
                "actionProbs=" + Arrays.toString(actionProbs) +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        ActionProbs that = (ActionProbs) o;

        // Probably incorrect - comparing Object[] arrays with Arrays.equals
        return Arrays.equals(actionProbs, that.actionProbs);

    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(actionProbs);
    }
}
