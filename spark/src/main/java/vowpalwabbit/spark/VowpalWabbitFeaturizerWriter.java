package vowpalwabbit.spark;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import org.apache.hadoop.fs.Path;
import org.apache.spark.ml.util.MLWriter;
import org.apache.spark.ml.util.DefaultParamsWriter$;

/**
 * @author Markus Cozowicz
 */
public class VowpalWabbitFeaturizerWriter extends MLWriter implements Serializable {

    private VowpalWabbitFeaturizer _instance;
    
    public VowpalWabbitFeaturizerWriter(VowpalWabbitFeaturizer instance) { _instance = instance; }
    public VowpalWabbitFeaturizer getInstance() { return _instance; }
    public void setInstance(VowpalWabbitFeaturizer instance) { _instance = instance; }
      
    @Override
    public void saveImpl(String path) {
        DefaultParamsWriter$.MODULE$.saveMetadata(_instance, path, sc(), DefaultParamsWriter$.MODULE$
                .getMetadataToSave$default$3(), DefaultParamsWriter$.MODULE$.getMetadataToSave$default$4());
        Data data = new Data();
        data.setInputCols(_instance.getInputCols());
        List<Data> listData = new ArrayList<>();
        listData.add(data);
        String dataPath = new Path(path, "data").toString();
        sparkSession().createDataFrame(listData, Data.class).repartition(1).write().parquet(dataPath);
    }

    public static class Data implements Serializable {
        private String[] _inputCols;
        private String _outputCol;
        private int _seed;

        public String[] getInputCols() { return _inputCols; }
        public void setInputCols(String[] inputCols) { _inputCols = inputCols; }

        public String getOutputCol() { return _outputCol; }
        public void setOutputCol(String value) { _outputCol = value; }

        public int getSeed() { return _seed; }
        public void setSeed(int value) { _seed = value; }
    }
}