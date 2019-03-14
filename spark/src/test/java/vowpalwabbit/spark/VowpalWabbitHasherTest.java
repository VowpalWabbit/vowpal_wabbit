package vowpalwabbit.spark;

import org.junit.Test;
import static org.junit.Assert.*;
import org.apache.spark.sql.*;
import org.apache.spark.sql.types.*;
import static org.apache.spark.sql.functions.*;

public class VowpalWabbitHasherTest {

    @Test
    public void testSimple1() throws Exception {

        SparkSession spark = SparkSession.builder()
            .master("local[2]") // 2 ... number of threads
            .appName("ASA")
            .config("spark.sql.shuffle.partitions", 1)
            .config("spark.ui.enabled", false)
            .getOrCreate();

        // spark.getSparkContext().setLogLevel("ERROR");
        Dataset<Row> df = spark //.sqlContext
            .read()
            .format("csv")
            .option("header", "true")
            .option("inferSchema", "true")
            .load(getClass().getResource("/dataset1.csv").getPath())
            .select(col("ts").cast(DataTypes.LongType), col("category"), col("value"));

        VowpalWabbitFeaturizer hasher = new VowpalWabbitFeaturizer()
            .setInputCols(new String[] { "category", "value" })
            .setOutputCol("foo"); // serves namespace

        Dataset<Row> dfOutput = hasher.transform(df);

        dfOutput.show();
    }
}