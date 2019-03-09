package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

public abstract class NumericFeaturizer implements IFeaturizer, Serializable {
    protected int fieldIdx;
    private int featureIdx;

    public NumericFeaturizer(int fieldIdx, String fieldName, int namespaceHash, int mask) {
        this.fieldIdx = fieldIdx;

        // TODO: check if the fieldName is a int? is this even support as column name?
        this.featureIdx = VowpalWabbitMurmur.hash(fieldName, namespaceHash) & mask;
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

    protected abstract double getAsDouble(Row r);
}