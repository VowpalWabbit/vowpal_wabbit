package vowpalwabbit.spark.featurizer;

import java.io.Serializable;
import java.util.ArrayList;
import org.apache.spark.sql.Row;
import vowpalwabbit.spark.*;

public class StringFeaturizer implements IFeaturizer, Serializable {
    private int fieldIdx;
    private String fieldName;
    private int namespaceHash;
    private int mask;

    public StringFeaturizer(int fieldIdx, String fieldName, int namespaceHash, int mask) {
        this.fieldIdx = fieldIdx;
        this.fieldName = fieldName;
        this.namespaceHash = namespaceHash;
        this.mask = mask;
    }

    public void featurize(Row r, ArrayList<Integer> indices, ArrayList<Double> values) {
        if (r.isNullAt(fieldIdx))
            return;

        indices.add(VowpalWabbitMurmur.hash(fieldName + r.getString(fieldIdx), namespaceHash) & mask);
        values.add(1.0);
    }
}