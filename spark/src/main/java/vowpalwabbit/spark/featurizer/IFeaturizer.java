package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;
import java.util.ArrayList;
import java.io.Serializable;

/**
 * @author Markus Cozowicz
 */
public interface IFeaturizer extends Serializable {
    /**
     * Implemententation should extract the features from {@code Row} and append to {@code indices} and {@code values}.
     * 
     * @param r the source row.
     * @param indices the indices that should be populated.
     * @param values the values that should be populated.
     */
    void featurize(Row r, ArrayList<Integer> indices, ArrayList<Double> values);
}