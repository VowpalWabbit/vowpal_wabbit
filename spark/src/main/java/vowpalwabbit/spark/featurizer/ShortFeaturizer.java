package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

public class ShortFeaturizer extends NumericFeaturizer {
    public ShortFeaturizer(int fieldIdx, String fieldName, int namespaceHash, int mask) {
        super(fieldIdx, fieldName, namespaceHash, mask);
    }

    protected double getAsDouble(Row r) {
        return (double)r.getShort(super.fieldIdx);
    }
}