package vw.responses;

import java.util.Arrays;

/**
 * Created by jmorra on 8/12/16.
 */
public class Multilabels {

    private final int[] labels;

    public Multilabels(final int[] labels) {
        this.labels = labels;
    }

    public int[] getLabels() {
        return labels;
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
