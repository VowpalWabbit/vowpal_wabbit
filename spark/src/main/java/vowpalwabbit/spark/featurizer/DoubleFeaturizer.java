package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

public class DoubleFeaturizer extends NumericFeaturizer {
    public DoubleFeaturizer(int fieldIdx, String fieldName, int namespaceHash, int mask) {
        super(fieldIdx, fieldName, namespaceHash, mask);
    }

    protected double getAsDouble(Row r) {
        return r.getDouble(super.fieldIdx);
    }
}