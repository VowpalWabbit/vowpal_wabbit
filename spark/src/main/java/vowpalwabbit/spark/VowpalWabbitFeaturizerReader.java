package vowpalwabbit.spark;

import java.io.Serializable;
import java.util.List;
import org.apache.hadoop.fs.Path;
import org.apache.spark.ml.util.DefaultParamsReader;
import org.apache.spark.ml.util.DefaultParamsReader$;
import org.apache.spark.ml.util.MLReader;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;

/**
 * @author Markus Cozowicz
 */
public class VowpalWabbitFeaturizerReader extends MLReader<VowpalWabbitFeaturizer> implements Serializable {
    
    private String className = VowpalWabbitFeaturizer.class.getName();  
    public String getClassName() { return className; }
    public void setClassName(String className) { className = className; }
    
    @Override
    public VowpalWabbitFeaturizer load(String path) {
        DefaultParamsReader.Metadata metadata = DefaultParamsReader$.MODULE$.loadMetadata(path, sc(), className);
        String dataPath = new Path(path, "data").toString();
        Dataset<Row> data = sparkSession().read().parquet(dataPath);
        
        List<String> listInputColumns = data.select("inputCols").head().getList(0);
        String[] inputColumns = listInputColumns.toArray(new String[listInputColumns.size()]);
        String outputColumn = data.select("outputCol").head().getString(0);
        int seed = data.select("seed").head().getInt(0);
        VowpalWabbitFeaturizer transformer = new VowpalWabbitFeaturizer()
                .setInputCols(inputColumns)
                .setOutputCol(outputColumn)
                .setSeed(seed);
        DefaultParamsReader$.MODULE$.getAndSetParams(transformer, metadata, DefaultParamsReader$.MODULE$.getAndSetParams$default$3());

        return transformer;
    }
}