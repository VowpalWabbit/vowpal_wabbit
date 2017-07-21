package vowpalWabbit.responses;

import java.io.Serializable;

/**
 * Created by jmorra on 8/12/16.
 */
public class ActionScore implements Serializable {

    private final int action;
    private final float score;

    public ActionScore(final int action, final float score) {
        this.action = action;
        this.score = score;
    }

    public int getAction() {
        return action;
    }

    public float getScore() {
        return score;
    }

    @Override
    public String toString() {
        return "ActionScore{" +
                "action=" + action +
                ", score=" + score +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        ActionScore that = (ActionScore) o;

        if (action != that.action) return false;
        return Float.compare(that.score, score) == 0;

    }

    @Override
    public int hashCode() {
        int result = action;
        result = 31 * result + (score != +0.0f ? Float.floatToIntBits(score) : 0);
        return result;
    }
}
