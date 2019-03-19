package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;

/**
 * @author Markus Cozowicz
 */
public class DoubleFeaturizer extends NumericFeaturizer {
    public DoubleFeaturizer(int fieldIdx, String fieldName, int namespaceHash) {
        super(fieldIdx, fieldName, namespaceHash);
    }

    protected double getAsDouble(Row r) {
        return r.getDouble(super.fieldIdx);
    }
}