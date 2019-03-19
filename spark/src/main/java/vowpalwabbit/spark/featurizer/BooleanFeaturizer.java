package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;

/**
 * @author Markus Cozowicz
 */
public class BooleanFeaturizer extends NumericFeaturizer {
    public BooleanFeaturizer(int fieldIdx, String fieldName, int namespaceHash) {
        super(fieldIdx, fieldName, namespaceHash);
    }

    protected double getAsDouble(Row r) {
        return r.getBoolean(super.fieldIdx) ? 1 : 0;
    }
}