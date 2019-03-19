package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

/**
 * Featurizes a string by concatenating the input column name and field value.
 * 
 * @author Markus Cozowicz
 */
public class StringFeaturizer implements IFeaturizer, Serializable {
    private final int fieldIdx;
    private final String columnName;
    private final int namespaceHash;

    public StringFeaturizer(int fieldIdx, String columnName, int namespaceHash) {
        this.fieldIdx = fieldIdx;
        this.columnName = columnName;
        this.namespaceHash = namespaceHash;
    }

    public void featurize(Row r, ArrayList<Integer> indices, ArrayList<Double> values) {
        if (r.isNullAt(fieldIdx))
            return;

        indices.add(VowpalWabbitMurmur.hash(columnName + r.getString(fieldIdx), namespaceHash));
        values.add(1.0);
    }
}