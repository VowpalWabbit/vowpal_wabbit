package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;

/**
 * @author Markus Cozowicz
 */
public class ByteFeaturizer extends NumericFeaturizer {
    public ByteFeaturizer(int fieldIdx, String fieldName, int namespaceHash) {
        super(fieldIdx, fieldName, namespaceHash);
    }

    protected double getAsDouble(Row r) {
        return (double)r.getByte(super.fieldIdx);
    }
}