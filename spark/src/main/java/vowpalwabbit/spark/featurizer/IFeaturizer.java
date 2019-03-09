package vowpalwabbit.spark.featurizer;

import org.apache.spark.sql.Row;
import java.util.ArrayList;
import java.io.Serializable;

public interface IFeaturizer extends Serializable {
    void featurize(Row r, ArrayList<Integer> indices, ArrayList<Double> values);
}