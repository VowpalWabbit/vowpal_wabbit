package vowpalWabbit.responses;

import java.io.Serializable;
import java.util.Arrays;

/**
 * Prediction result for cs_active reductions containing the predicted class
 * and classes requiring more information.
 */
public class ActiveMulticlass implements Serializable {

    private final int predictedClass;
    private final int[] moreInfoRequiredForClasses;

    public ActiveMulticlass(int predictedClass, int[] moreInfoRequiredForClasses) {
        this.predictedClass = predictedClass;
        this.moreInfoRequiredForClasses = moreInfoRequiredForClasses;
    }

    public int getPredictedClass() {
        return predictedClass;
    }

    public int[] getMoreInfoRequiredForClasses() {
        return moreInfoRequiredForClasses;
    }

    @Override
    public String toString() {
        return "ActiveMulticlass{" +
                "predictedClass=" + predictedClass +
                ", moreInfoRequiredForClasses=" + Arrays.toString(moreInfoRequiredForClasses) +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        ActiveMulticlass that = (ActiveMulticlass) o;
        return predictedClass == that.predictedClass &&
                Arrays.equals(moreInfoRequiredForClasses, that.moreInfoRequiredForClasses);
    }

    @Override
    public int hashCode() {
        return 31 * predictedClass + Arrays.hashCode(moreInfoRequiredForClasses);
    }
}
