package vowpalwabbit.spark;

import org.junit.Test;
import static org.junit.Assert.*;
import java.io.*;
import java.nio.file.*;
import java.nio.charset.Charset;
import java.util.List;
import vowpalwabbit.spark.prediction.*;

public class VowpalWabbitNativeIT {

    @Test
    public void testSimple1() throws Exception {
        // need to use confidence_after_training as otherwise the numbers don't match up...
        Runtime.getRuntime()
            .exec("../vowpalwabbit/vw --quiet --confidence --confidence_after_training -f target/testSimple1-ref.model -d src/test/resources/test.txt -p target/testSimple1-ref.pred")
            .waitFor();

        byte[] modelRef = Files.readAllBytes(Paths.get("target/testSimple1-ref.model"));
        List<String> predsRef = Files.readAllLines(Paths.get("target/testSimple1-ref.pred"), Charset.defaultCharset());

        byte[] model;
        try (VowpalWabbitNative vw = new VowpalWabbitNative("--quiet --confidence --confidence_after_training"))
        {
            try (VowpalWabbitExample ex = vw.createExample())
            {
                for (int i=0;i<10;i++) {
                    ex.addToNamespaceDense('a',
                        VowpalWabbitMurmur.hash("a", 0),
                        new double[] { 1.0, 2.0, 3.0 });
                    ex.setLabel(i % 2);

                    ex.learn();

                    ScalarPrediction pred = (ScalarPrediction)ex.getPrediction();

                    String[] scalarAndConfidenceRef = predsRef.get(i).split(" ");

                    // compare predictions and confidence
                    assertEquals(Float.parseFloat(scalarAndConfidenceRef[0]), pred.getValue(), 1e-4);
                    assertEquals(Float.parseFloat(scalarAndConfidenceRef[1]), pred.getConfidence(), 1e-4);

                    ex.clear();
                }

                vw.endPass();
            }

            model = vw.getModel();
            try (FileOutputStream out = new FileOutputStream("target/testSimple1.model"))
            {
                out.write(model);
            }
        }

        // compare model
        assertArrayEquals(model, modelRef);
    }
}