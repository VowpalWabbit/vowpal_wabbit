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

public class VowpalWabbitFeaturizer extends Transformer implements Serializable, DefaultParamsWritable {

    private StringArrayParam inputCols;
    private Param<String> outputCol;
    private final String uid;

    public VowpalWabbitFeaturizer(String uid) {
        this.uid = uid;
    }

    public VowpalWabbitFeaturizer() {
        this.uid = VowpalWabbitFeaturizer.class.getName() + "_" + UUID.randomUUID().toString();
    }

    public String[] getInputCols() { return get(this.inputCols).get(); }

    public VowpalWabbitFeaturizer setInputCols(String[] columns) {
        this.inputCols = inputCols();
        set(this.inputCols, columns);
        return this;
    }

    public String getOutputCol() { return get(this.outputCol).get(); }

    public VowpalWabbitFeaturizer setOutputCol(String column) {
        this.outputCol = outputCol();
        set(this.outputCol, column);
        return this;
    }

    // TODO: params (hashAll, seed, bits)

    @Override
    public Dataset<Row> transform(Dataset<?> data) {
        StructType schema = data.schema();
        StructField[] fields = schema.fields();

        List<String> inputColsList = Arrays.asList(getInputCols());

        // TODO: hashAll, seed, bits
        // TODO: add parameters to meta data so the trainer can validate
        final int numBits = 18;
        int mask = (1 << numBits) - 1;

        String targetNamespace = getOutputCol();
        int seed = 0; // TODO
        int namespaceHash = VowpalWabbitMurmur.hash(targetNamespace, seed);

        final IFeaturizer[] featurizer = new IFeaturizer[inputColsList.size()];
        for (int j = 0, i = 0; j < fields.length; j++) {
            String fieldName = fields[j].name();

            if (!inputColsList.contains(fieldName))
                continue;

            DataType fieldType = fields[j].dataType();
            if (fieldType == DataTypes.StringType)
                featurizer[i] = new StringFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.IntegerType)
                featurizer[i] = new IntegerFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.LongType)
                featurizer[i] = new LongFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.ShortType)
                featurizer[i] = new ShortFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.ByteType)
                featurizer[i] = new ByteFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.FloatType)
                featurizer[i] = new FloatFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.DoubleType)
                featurizer[i] = new DoubleFeaturizer(i, fieldName, namespaceHash, mask);
            else if (fieldType == DataTypes.BooleanType)
                featurizer[i] = new BooleanFeaturizer(i, fieldName, namespaceHash, mask);
            else   
                throw new RuntimeException("Unsupported type: " + fieldType);

            i++;

            // TODO: list types

            // BinaryType
            // CalendarIntervalType
            // DateType
            // NullType
            // TimestampType
            // getStruct
            // getMap
            // getSeq
        }

        UserDefinedFunction mode = udf(
            (Row r) -> {
                final ArrayList<Integer> indices = new ArrayList<>(featurizer.length);
                ArrayList<Double> values = new ArrayList<>(featurizer.length);

                for (IFeaturizer f : featurizer)
                    f.featurize(r, indices, values);

                // need to sort them due to SparseVector restriction
                int[] sortedIndices = IntStream.range(0, indices.size())
                    .boxed().sorted((i, j) -> indices.get(i).compareTo(indices.get(j)))
                    .mapToInt(Integer::intValue).toArray();

                // TODO: maybe need to filter duplicates?
                int[] indicesArr = new int[indices.size()];
                double[] valuesArr = new double[values.size()];

                for (int i = 0; i<indices.size(); i++) {
                    indicesArr[sortedIndices[i]] = indices.get(i);
                    valuesArr[sortedIndices[i]] = values.get(i);
                }

                return new SparseVector((int)(1 << numBits), indicesArr, valuesArr);
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
        // TODO:
        copied.setInputCols(this.getInputCols());
        copied.setOutputCol(this.getOutputCol());

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

    public VowpalWabbitFeaturizer load(String path) {
        return ((VowpalWabbitFeaturizerReader)read()).load(path);
    }
}