package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

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