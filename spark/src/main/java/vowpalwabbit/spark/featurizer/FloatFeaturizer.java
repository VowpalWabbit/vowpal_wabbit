package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;

/**
 * @author Markus Cozowicz
 */
public class FloatFeaturizer extends NumericFeaturizer {
    public FloatFeaturizer(int fieldIdx, String fieldName, int namespaceHash) {
        super(fieldIdx, fieldName, namespaceHash);
    }

    protected double getAsDouble(Row r) {
        return (double)r.getFloat(super.fieldIdx);
    }
}