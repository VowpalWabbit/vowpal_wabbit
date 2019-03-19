package vowpalwabbit.spark;

import java.io.IOException;
import java.io.Serializable;
import java.util.*;
import java.util.stream.*;
import org.apache.spark.ml.linalg.SparseVector;
import org.apache.spark.ml.linalg.VectorUDT;
import org.apache.spark.ml.Transformer;
import org.apache.spark.ml.param.ParamMap;
import org.apache.spark.ml.param.StringArrayParam;
import org.apache.spark.ml.param.Param;
import org.apache.spark.ml.param.IntParam;
import org.apache.spark.ml.param.shared.HasOutputCol;
import org.apache.spark.ml.util.DefaultParamsWritable;
import org.apache.spark.ml.util.MLReader;
import org.apache.spark.ml.util.MLWriter;
import org.apache.spark.sql.*;
import org.apache.spark.sql.catalyst.encoders.RowEncoder;
import org.apache.spark.sql.expressions.UserDefinedFunction;
import org.apache.spark.sql.types.*;
import static org.apache.spark.sql.functions.*;
import scala.collection.JavaConverters;
import scala.collection.Seq;
import vowpalwabbit.spark.featurizer.*;

/**
 * @author Markus Cozowicz
 */
public class VowpalWabbitFeaturizer extends Transformer implements Serializable, DefaultParamsWritable {

    private StringArrayParam inputCols;
    private Param<String> outputCol;
    private IntParam seed;
    // TODO: hashAll though it's not clear that numeric column names are even supported

    private final String uid;

    public VowpalWabbitFeaturizer(String uid) {
        this.uid = uid;
    }

    public VowpalWabbitFeaturizer() {
        this.uid = VowpalWabbitFeaturizer.class.getName() + "_" + UUID.randomUUID().toString();

        // some default values
        setInputCols(new String[] { "data" });
        setOutputCol("namespace0");
        setSeed(0);
    }

    ///// Input Cols

    public String[] getInputCols() { return get(this.inputCols).get(); }

    public VowpalWabbitFeaturizer setInputCols(String[] columns) {
        this.inputCols = inputCols();
        set(this.inputCols, columns);
        return this;
    }

    ///// Output Cols

    public String getOutputCol() { return get(this.outputCol).get(); }

    public VowpalWabbitFeaturizer setOutputCol(String column) {
        this.outputCol = outputCol();
        set(this.outputCol, column);
        return this;
    }

    ///// Seed

    public int getSeed() { return (int)get(this.seed).get(); }

    public VowpalWabbitFeaturizer setSeed(int value) {
        this.seed = seed();
        set(this.seed, value);
        return this;
    }

    @Override
    public Dataset<Row> transform(Dataset<?> data) {
        StructType schema = data.schema();
        StructField[] fields = schema.fields();

        List<String> inputColsList = Arrays.asList(getInputCols());

        String targetNamespace = this.getOutputCol();
        int namespaceHash = VowpalWabbitMurmur.hash(targetNamespace, this.getSeed());

        final IFeaturizer[] featurizer = new IFeaturizer[inputColsList.size()];
        for (int j = 0, i = 0; j < fields.length; j++) {
            String fieldName = fields[j].name();

            if (!inputColsList.contains(fieldName))
                continue;

            DataType fieldType = fields[j].dataType();
            if (fieldType == DataTypes.StringType)
                featurizer[i] = new StringFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.IntegerType)
                featurizer[i] = new IntegerFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.LongType)
                featurizer[i] = new LongFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.ShortType)
                featurizer[i] = new ShortFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.ByteType)
                featurizer[i] = new ByteFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.FloatType)
                featurizer[i] = new FloatFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.DoubleType)
                featurizer[i] = new DoubleFeaturizer(i, fieldName, namespaceHash);
            else if (fieldType == DataTypes.BooleanType)
                featurizer[i] = new BooleanFeaturizer(i, fieldName, namespaceHash);
            else   
                throw new RuntimeException("Unsupported type: " + fieldType);

            // Glance: seq + map
            // https://spark.apache.org/docs/2.3.0/api/java/index.html?org/apache/spark/mllib/tree/model/Split.html


            i++;

            // TODO: list types

            // BinaryType
            // CalendarIntervalType
            // DateType
            // NullType
            // TimestampType
            // getStruct
            // getMap
            // - Key<String, number/string> -> CogServices
            // getSeq
            // - List<string> -> Categories <-- spark sql split
            // - 
        }

        UserDefinedFunction mode = udf(
            (Row r) -> {
                final ArrayList<Integer> indices = new ArrayList<>(featurizer.length);
                ArrayList<Double> values = new ArrayList<>(featurizer.length);

                for (IFeaturizer f : featurizer)
                    f.featurize(r, indices, values);

                int maxIndexMask = ((1 << 31) - 1);
                // TODO: review this, but due to SparseVector limitations we don't support large indices
                for (int i = 0; i < indices.size(); i++) 
                    indices.set(i, indices.get(i) & maxIndexMask);

                // need to sort them due to SparseVector restriction
                int[] sortedIndices = IntStream.range(0, indices.size())
                    .boxed()
                    .sorted((i, j) -> indices.get(i).compareTo(indices.get(j)))
                    // filter duplicates indices
                    // Warning: 
                    //   - due to SparseVector limitations (which doesn't allow duplicates) we need filter
                    //   - VW command line allows for duplicate features with different values (just updates twice)
                    .distinct() // optimized according to https://stackoverflow.com/questions/43806467/java-streams-how-to-do-an-efficient-distinct-and-sort
                    .mapToInt(Integer::intValue)
                    .toArray();

                int[] indicesArr = new int[sortedIndices.length];
                double[] valuesArr = new double[sortedIndices.length];

                for (int i = 0; i < sortedIndices.length; i++) {
                    int idx = sortedIndices[i];
                    indicesArr[i] = indices.get(idx);
                    valuesArr[i] = values.get(idx);
                }

                return new SparseVector(maxIndexMask, indicesArr, valuesArr);
            },
            new VectorUDT()
        );

        // Note: the struct(Seq<Column>) does something weird when constructing the named_struct
        Column[] cols = inputColsList.stream().map(c -> col(c)).toArray(Column[]::new);

        return data.withColumn(getOutputCol(), mode.apply(struct(cols)));
    }

    @Override
    public Transformer copy(ParamMap extra) {
        VowpalWabbitFeaturizer copied = new VowpalWabbitFeaturizer();
       
        copied.setInputCols(this.getInputCols());
        copied.setOutputCol(this.getOutputCol());
        copied.setSeed(this.getSeed());

        return copied;
    }

    @Override
    public StructType transformSchema(StructType oldSchema) {
        return oldSchema.add(getOutputCol(), new VectorUDT());
    }

    @Override
    public String uid() {
        return uid;
    }

    @Override
    public MLWriter write() {
        return new VowpalWabbitFeaturizerWriter(this);
    }

    @Override
    public void save(String path) throws IOException {
        write().saveImpl(path);
    }

    public static MLReader<VowpalWabbitFeaturizer> read() {
        return new VowpalWabbitFeaturizerReader();
    }

    public StringArrayParam inputCols() {
        return new StringArrayParam(this, "inputCols", "Columns to be hashed");
    }

    public Param<String> outputCol() {
        return new Param<String>(this, "outputCol", "Output column created");
    }

    public IntParam seed() { 
        return new IntParam(this, "seed", "The seed used for hashing");
    }

    public VowpalWabbitFeaturizer load(String path) {
        return ((VowpalWabbitFeaturizerReader)read()).load(path);
    }
}