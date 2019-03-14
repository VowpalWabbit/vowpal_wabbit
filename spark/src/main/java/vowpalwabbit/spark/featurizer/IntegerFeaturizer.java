package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

/**
 * @author Markus Cozowicz
 */
public class IntegerFeaturizer extends NumericFeaturizer {

    public IntegerFeaturizer(int fieldIdx, String fieldName, int namespaceHash) {
        super(fieldIdx, fieldName, namespaceHash);
    }

    protected double getAsDouble(Row r) {
        return (double)r.getInt(super.fieldIdx);
    }
}