package vowpalWabbit.responses;

import java.io.Serializable;

/**
 * Created by jmorra on 8/12/16.
 */
public class ActionProb implements Serializable {

    private final int action;
    private final float probability;

    public ActionProb(final int action, final float probability) {
        this.action = action;
        this.probability = probability;
    }

    public int getAction() {
        return action;
    }

    public float getProbability() {
        return probability;
    }

    @Override
    public String toString() {
        return "ActionProb{" +
                "action=" + action +
                ", probability=" + probability +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        ActionProb that = (ActionProb) o;

        if (action != that.action) return false;
        return Float.compare(that.probability, probability) == 0;

    }

    @Override
    public int hashCode() {
        int result = action;
        result = 31 * result + (probability != +0.0f ? Float.floatToIntBits(probability) : 0);
        return result;
    }
}
