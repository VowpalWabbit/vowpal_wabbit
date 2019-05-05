package vowpalWabbit.responses;

import java.io.Serializable;
import java.util.Arrays;

/**
 * Created by jmorra on 8/12/16.
 */
public class Multilabels implements Serializable {

    // Although this is modifiable it is not intended to be updated by the user.  This data structure mimics the
    // C data structure.
    private final int[] labels;

    public Multilabels(final int[] labels) {
        this.labels = labels;
    }

    public int[] getLabels() {
        return labels;
    }

    @Override
    public String toString() {
        return "Multilabels{" +
                "labels=" + Arrays.toString(labels) +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Multilabels that = (Multilabels) o;

        return Arrays.equals(labels, that.labels);

    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(labels);
    }
}
