package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

/**
 * Base class for numeric featurizers.
 * 
 * @author Markus Cozowicz
 */
public abstract class NumericFeaturizer implements IFeaturizer, Serializable {
    protected final int fieldIdx;
    private final int featureIdx;

    public NumericFeaturizer(int fieldIdx, String columnName, int namespaceHash) {
        this.fieldIdx = fieldIdx;

        // TODO: check if the columnName is a int? is this even support as column name?
        // this would add support for "--hash_all"
        this.featureIdx = VowpalWabbitMurmur.hash(columnName, namespaceHash);
    }

    public void featurize(Row r, ArrayList<Integer> indices, ArrayList<Double> values) {
        if (r.isNullAt(fieldIdx))
            return;
        
        double value = getAsDouble(r);
        if (value == 0)
            return;

        indices.add(featureIdx);
        values.add(value);
    }

    /**
     * Sub-classes should return the feature value as double.
     * 
     * @param r the source row.
     * @return the feature value.
     */
    protected abstract double getAsDouble(Row r);
}