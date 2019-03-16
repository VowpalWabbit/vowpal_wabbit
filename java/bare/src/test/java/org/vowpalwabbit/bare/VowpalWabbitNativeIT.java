package org.vowpalwabbit.bare;

import org.junit.Test;
import static org.junit.Assert.*;
import java.io.*;
import java.nio.file.*;
import java.nio.charset.Charset;
import java.util.*;
import org.vowpalwabbit.bare.prediction.*;

/**
 * @author Markus Cozowicz
 */
public class VowpalWabbitNativeIT {

    @Test
    public void testWrappedVsCommandLine() throws Exception {
        String vwBinary = Files.readAllLines(Paths.get(getClass().getResource("/vw-bin.txt").getPath())).get(0);

        // need to use confidence_after_training as otherwise the numbers don't match up...
        Runtime.getRuntime()
            .exec(vwBinary + " --quiet --confidence --confidence_after_training -f target/testSimple1-ref.model -d src/test/resources/test.txt -p target/testSimple1-ref.pred")
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

    @Test
    public void testPrediction() throws Exception {
        byte[] model;
        float learnPrediction = 0f;
        try (VowpalWabbitNative vw = new VowpalWabbitNative("--quiet"))
        {
            try (VowpalWabbitExample ex = vw.createExample())
            {
                for (int i=0;i<10;i++) {
                    ex.addToNamespaceDense('a',
                        VowpalWabbitMurmur.hash("a", 0),
                        new double[] { 1.0, 2.0, 3.0 });
                    ex.setLabel(i % 2);

                    ex.learn();
                    ex.clear();
                }

                vw.endPass();
            }

            model = vw.getModel();

            try (VowpalWabbitExample ex = vw.createExample())
            {
                    ex.addToNamespaceDense('a',
                        VowpalWabbitMurmur.hash("a", 0),
                        new double[] { 1.0, 2.0, 3.0 });

                    ex.predict();

                    ScalarPrediction pred = (ScalarPrediction)ex.getPrediction();
                    learnPrediction = pred.getValue();

                    assertTrue(learnPrediction > 0);
            }
        }

        try (VowpalWabbitNative vw = new VowpalWabbitNative("--quiet", model))
        {
            VowpalWabbitArguments args = vw.getArguments();

            assertEquals(18, args.getNumBits());
            assertEquals(0, args.getHashSeed());

            try (VowpalWabbitExample ex = vw.createExample())
            {
                ex.addToNamespaceDense('a',
                    VowpalWabbitMurmur.hash("a", 0),
                    new double[] { 1.0, 2.0, 3.0 });

                ScalarPrediction pred = (ScalarPrediction)ex.predict();

                assertEquals(learnPrediction, pred.getValue(), 1e-4);
            }
        }
    }
}