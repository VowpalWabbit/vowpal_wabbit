package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;

/**
 * @author Markus Cozowicz
 */
public class LongFeaturizer extends NumericFeaturizer {
    public LongFeaturizer(int fieldIdx, String columnName, int namespaceHash) {
        super(fieldIdx, columnName, namespaceHash);
    }

    protected double getAsDouble(Row r) {
        return (double)r.getLong(super.fieldIdx);
    }
}