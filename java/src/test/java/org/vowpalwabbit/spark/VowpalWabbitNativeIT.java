package org.vowpalwabbit.spark;

import org.junit.Assume;
import org.junit.Test;
import static org.junit.Assert.*;
import java.io.*;
import java.nio.file.*;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.*;
import org.vowpalwabbit.spark.prediction.*;

import vowpalWabbit.responses.ActionProbs;
import vowpalWabbit.responses.DecisionScores;

/**
 * command line invocation
 *
 * mvn verify -Dtest=foo
 * -Dit.test=org.vowpalwabbit.spark.VowpalWabbitNativeIT#testAudit
 * -DfailIfNoTests=false -Dmaven.javadoc.skip=true
 *
 * @author Markus Cozowicz
 */
public class VowpalWabbitNativeIT {
    @Test
    public void testHashing() throws Exception {
        String w1 = "ஜெய்";

        byte[] sarr = ("a" + w1).getBytes(StandardCharsets.UTF_8);

        int h1 = VowpalWabbitMurmur.hash(sarr, 0, sarr.length, -1801964169);
        int h1n = VowpalWabbitMurmur.hashNative(sarr, 0, sarr.length, -1801964169);

        assertEquals(h1, h1n);
    }

    @Test
    public void testWrappedVsCommandLine() throws Exception {
        String vwBinary = Files.readAllLines(Paths.get(getClass().getResource("/vw_cli_bin.txt").toURI())).get(0).trim();

        // need to use confidence_after_training as otherwise the numbers don't match
        // up...
        Runtime.getRuntime().exec(vwBinary
                + " --quiet --confidence --confidence_after_training -f target/testSimple1-ref.model -d src/test/resources/test.txt -p target/testSimple1-ref.pred")
                .waitFor();

        byte[] modelRef = Files.readAllBytes(Paths.get("target/testSimple1-ref.model"));
        List<String> predsRef = Files.readAllLines(Paths.get("target/testSimple1-ref.pred"), Charset.defaultCharset());

        byte[] model;
        VowpalWabbitNative vw = null;
        VowpalWabbitExample ex = null;
        FileOutputStream out = null;

        try {
            vw = new VowpalWabbitNative("--quiet --confidence --confidence_after_training");
            ex = vw.createExample();

            for (int i = 0; i < 10; i++) {
                ex.addToNamespaceDense('a', VowpalWabbitMurmur.hash("a", 0), new double[] { 1.0, 2.0, 3.0 });
                ex.setLabel(i % 2);

                ex.learn();

                ScalarPrediction pred = (ScalarPrediction) ex.getPrediction();

                String[] scalarAndConfidenceRef = predsRef.get(i).split(" ");

                // compare predictions and confidence
                assertEquals(Float.parseFloat(scalarAndConfidenceRef[0]), pred.getValue(), 1e-4);
                assertEquals(Float.parseFloat(scalarAndConfidenceRef[1]), pred.getConfidence(), 1e-4);

                ex.clear();
            }

            vw.endPass();

            model = vw.getModel();
            out = new FileOutputStream("target/testSimple1.model");
            out.write(model);

        } finally {
            if (out != null)
                out.close();

            if (ex != null)
                ex.close();

            if (vw != null)
                vw.close();
        }

        // compare model
        assertArrayEquals(model, modelRef);
    }

    @Test
    public void testPrediction() throws Exception {
        byte[] model;
        float learnPrediction = 0f;
        VowpalWabbitNative vw = null;
        VowpalWabbitExample ex = null;

        try {
            vw = new VowpalWabbitNative("--quiet");
            ex = vw.createExample();
            for (int i = 0; i < 10; i++) {
                ex.addToNamespaceDense('a', VowpalWabbitMurmur.hash("a", 0), new double[] { 1.0, 2.0, 3.0 });
                ex.setLabel(i % 2);

                ex.learn();
                ex.clear();
            }

            vw.endPass();

            ex.close();

            model = vw.getModel();

            ex = vw.createExample();
            ex.addToNamespaceDense('a', VowpalWabbitMurmur.hash("a", 0), new double[] { 1.0, 2.0, 3.0 });

            ex.predict();

            ScalarPrediction pred = (ScalarPrediction) ex.getPrediction();
            learnPrediction = pred.getValue();

            assertTrue(learnPrediction > 0);

            vw.close();

            // test the model
            vw = new VowpalWabbitNative("--quiet", model);
            VowpalWabbitArguments args = vw.getArguments();

            assertEquals(18, args.getNumBits());
            assertEquals(0, args.getHashSeed());

            ex = vw.createExample();
            ex.addToNamespaceDense('a', VowpalWabbitMurmur.hash("a", 0), new double[] { 1.0, 2.0, 3.0 });

            pred = (ScalarPrediction) ex.predict();

            assertEquals(learnPrediction, pred.getValue(), 1e-4);
        } finally {
            if (ex != null)
                ex.close();

            if (vw != null)
                vw.close();
        }
    }

    @Test
    public void testBFGS() throws Exception {
        // Skip on Windows: getTotalNumberOfFeatures() returns wrong value due to
        // JNI long/size_t mismatch in the Spark native bindings on Windows.
        Assume.assumeFalse(System.getProperty("os.name", "").toLowerCase().contains("win"));
        File tempFile = File.createTempFile("vowpalwabbit", ".cache");
        tempFile.deleteOnExit();
        String cachePath = tempFile.getAbsolutePath();
        VowpalWabbitNative vw = null;
        VowpalWabbitExample ex = null;

        try {
            vw = new VowpalWabbitNative(
                    "--loss_function=logistic -l 3.1 --power_t 0.2 --bfgs --passes 2 -k --cache_file=" + cachePath);
            // make sure getArguments works
            assertTrue(vw.getArguments().getArgs().contains("--bfgs"));

            ex = vw.createExample();

            for (int i = 0; i < 10; i++) {
                ex.addToNamespaceDense('a', VowpalWabbitMurmur.hash("a", 0), new double[] { 1.0, 2.0, 3.0 });
                ex.setLabel((i % 2) * 2 - 1);

                ex.learn();
                ex.clear();
            }

            vw.endPass();
            vw.performRemainingPasses();

            // validate arguments
            VowpalWabbitArguments args = vw.getArguments();

            assertEquals(3.1, args.getLearningRate(), 0.001);
            assertEquals(0.2, args.getPowerT(), 0.001);

            VowpalWabbitPerformanceStatistics stats = vw.getPerformanceStatistics();

            assertEquals(4, stats.getNumberOfExamplesPerPass());
            assertEquals(9.0, stats.getWeightedExampleSum(), 0.0001);
            assertEquals(-1.0, stats.getWeightedLabelSum(), 0.0001);
            assertEquals(0.6931, stats.getAverageLoss(), 0.0001);
            assertEquals(-0.223144, stats.getBestConstant(), 0.0001);
            assertEquals(0.6869, stats.getBestConstantLoss(), 0.0001);
            assertEquals(36, stats.getTotalNumberOfFeatures());

        } finally {
            if (ex != null)
                ex.close();

            if (vw != null)
                vw.close();
        }
    }

    @Test
    public void testAudit() throws Exception {
        VowpalWabbitNative vw = null;
        VowpalWabbitExample ex = null;

        try {
            // exepct no crash, can't directly validate as it writes to stdout
            vw = new VowpalWabbitNative("--loss_function=logistic --link=logistic -a");

            ex = vw.createExample();

            for (int i = 0; i < 2; i++) {
                ex.addToNamespaceDense('a', VowpalWabbitMurmur.hash("a", 0), new double[] { 1.0, 2.0, 3.0 });
                ex.setLabel((i % 2) * 2 - 1);

                ex.learn();
                ex.clear();
            }

            vw.endPass();

        } finally {
            if (ex != null)
                ex.close();

            if (vw != null)
                vw.close();
        }
    }

    public interface VowpalWabbitLabelOperator {
        public void op(VowpalWabbitExample ex);
    }

    private void testLabelSetter(VowpalWabbitLabelOperator op) throws Exception {
        VowpalWabbitNative vw = null;
        VowpalWabbitExample ex = null;

        try {
            // exepct no crash, can't directly validate as it writes to stdout
            vw = new VowpalWabbitNative("");

            ex = vw.createExample();

            op.op(ex);
        } finally {
            if (ex != null)
                ex.close();

            if (vw != null)
                vw.close();
        }
    }

    @Test
    public void testSimpleLabel() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setLabel(1,2);
            }
        });
    }

    @Test
    public void testMulticlassLabel() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setMulticlassLabel(0.5f, 2);
            }
        });
    }

    @Test
    public void testCostSensitiveLabels() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setCostSensitiveLabels(
                    new float[] { 0.5f, 0.2f },
                    new int[] { 0, 1}
                );
            }
        });
    }

    @Test
    public void tesContextualBanditContinuousLabel() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setContextualBanditContinuousLabel(
                    new float[] { 1f, 2f, 3f },
                    new float[] { 4f, 5f, 6f },
                    new float[] { 0.1f, 0.3f, 0.6f });
            }
        });
    }

    @Test
    public void testSlatesSharedLabel() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setSlatesSharedLabel(0.3f);
            }
        });
    }

    @Test
    public void testSlatesActionLabel() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setSlatesActionLabel(3);
            }
        });
    }

    @Test
    public void testSlatesSlotLabel() throws Exception {
        testLabelSetter(new VowpalWabbitLabelOperator() {
            public void op(VowpalWabbitExample ex) {
                ex.setSlatesSlotLabel(
                    new int[] { 1, 2 },
                    new float[] { 0.4f, 0.6f });
            }
        });
    }

    @Test
    public void testFromStringSingleLineText() throws Exception {
        VowpalWabbitNative vw = null;

        try {
            vw = new VowpalWabbitNative("--quiet");
            vw.learnFromString("0 | price:.23 sqft:.25 age:.05 2006");
            vw.learnFromString("1 2 'second_house | price:.18 sqft:.15 age:.35 1976");
            ScalarPrediction pred = (ScalarPrediction)vw.predictFromString("| price:.53 sqft:.32 age:.87 1924");
            assertNotEquals(0.0, pred.getValue(), 0.01);
        } finally {
            if (vw != null) {
                vw.close();
            }
        }
    }

    @Test
    public void testFromStringMultiLineText() throws Exception {
        VowpalWabbitNative vw = null;

        try {
            vw = new VowpalWabbitNative("--quiet --ccb_explore_adf");

            vw.learnFromString(
                "ccb shared |User b\n"
                + "ccb action |Action d\n"
                + "ccb action |Action e\n"
                + "ccb action |Action f\n"
                + "ccb action |Action ff\n"
                + "ccb action |Action fff\n"
                + "ccb slot 0:0:0.2 |Slot h\n"
                + "ccb slot 1:0:0.25 |Slot i\n"
                + "ccb slot 2:0:0.333333 |Slot j\n");
            DecisionScores pred = (DecisionScores)vw.predictFromString(
                "ccb shared |User b\n"
                + "ccb action |Action d\n"
                + "ccb action |Action e\n"
                + "ccb action |Action f\n"
                + "ccb action |Action ff\n"
                + "ccb action |Action fff\n"
                + "ccb slot |Slot h\n"
                + "ccb slot |Slot i\n"
                + "ccb slot |Slot j\n");
            assertEquals(3, pred.getDecisionScores().length);
            assertEquals(5, pred.getDecisionScores()[0].getActionScores().length);
            assertEquals(4, pred.getDecisionScores()[1].getActionScores().length);
            assertEquals(3, pred.getDecisionScores()[2].getActionScores().length);

        } finally {
            if (vw != null) {
                vw.close();
            }
        }
    }

    @Test
    public void testFromStringMultiLineDSJSON() throws Exception {
        VowpalWabbitNative vw = null;

        try {
            vw = new VowpalWabbitNative("--quiet --cb_explore_adf --dsjson");

            vw.learnFromString("{\"_label_cost\":-0.0,\"_label_probability\":0.05000000074505806,\"_label_Action\":4,\"_labelIndex\":3,\"o\":[{\"v\":0.0,\"EventId\":\"13118d9b4c114f8485d9dec417e3aefe\",\"ActionTaken\":false}],\"Timestamp\":\"2021-02-04T16:31:29.2460000Z\",\"Version\":\"1\",\"EventId\":\"13118d9b4c114f8485d9dec417e3aefe\",\"a\":[4,2,1,3],\"c\":{\"FromUrl\":[{\"timeofday\":\"Afternoon\",\"weather\":\"Sunny\",\"name\":\"Cathy\"}],\"_multi\":[{\"_tag\":\"Cappucino\",\"i\":{\"constant\":1,\"id\":\"Cappucino\"},\"j\":[{\"type\":\"hot\",\"origin\":\"kenya\",\"organic\":\"yes\",\"roast\":\"dark\"}]},{\"_tag\":\"Cold brew\",\"i\":{\"constant\":1,\"id\":\"Cold brew\"},\"j\":[{\"type\":\"cold\",\"origin\":\"brazil\",\"organic\":\"yes\",\"roast\":\"light\"}]},{\"_tag\":\"Iced mocha\",\"i\":{\"constant\":1,\"id\":\"Iced mocha\"},\"j\":[{\"type\":\"cold\",\"origin\":\"ethiopia\",\"organic\":\"no\",\"roast\":\"light\"}]},{\"_tag\":\"Latte\",\"i\":{\"constant\":1,\"id\":\"Latte\"},\"j\":[{\"type\":\"hot\",\"origin\":\"brazil\",\"organic\":\"no\",\"roast\":\"dark\"}]}]},\"p\":[0.05,0.05,0.05,0.85],\"VWState\":{\"m\":\"ff0744c1aa494e1ab39ba0c78d048146/550c12cbd3aa47f09fbed3387fb9c6ec\"},\"_original_label_cost\":-0.0}");
            ActionProbs pred = (ActionProbs)vw.predictFromString("{\"_label_cost\":-1.0,\"_label_probability\":0.8500000238418579,\"_label_Action\":1,\"_labelIndex\":0,\"o\":[{\"v\":1.0,\"EventId\":\"bf50a49c34b74937a81e8d6fc95faa99\",\"ActionTaken\":false}],\"Timestamp\":\"2021-02-04T16:31:29.9430000Z\",\"Version\":\"1\",\"EventId\":\"bf50a49c34b74937a81e8d6fc95faa99\",\"a\":[1,3,2,4],\"c\":{\"FromUrl\":[{\"timeofday\":\"Evening\",\"weather\":\"Snowy\",\"name\":\"Alice\"}],\"_multi\":[{\"_tag\":\"Cappucino\",\"i\":{\"constant\":1,\"id\":\"Cappucino\"},\"j\":[{\"type\":\"hot\",\"origin\":\"kenya\",\"organic\":\"yes\",\"roast\":\"dark\"}]},{\"_tag\":\"Cold brew\",\"i\":{\"constant\":1,\"id\":\"Cold brew\"},\"j\":[{\"type\":\"cold\",\"origin\":\"brazil\",\"organic\":\"yes\",\"roast\":\"light\"}]},{\"_tag\":\"Iced mocha\",\"i\":{\"constant\":1,\"id\":\"Iced mocha\"},\"j\":[{\"type\":\"cold\",\"origin\":\"ethiopia\",\"organic\":\"no\",\"roast\":\"light\"}]},{\"_tag\":\"Latte\",\"i\":{\"constant\":1,\"id\":\"Latte\"},\"j\":[{\"type\":\"hot\",\"origin\":\"brazil\",\"organic\":\"no\",\"roast\":\"dark\"}]}]},\"p\":[0.85,0.05,0.05,0.05],\"VWState\":{\"m\":\"ff0744c1aa494e1ab39ba0c78d048146/550c12cbd3aa47f09fbed3387fb9c6ec\"},\"_original_label_cost\":-1.0}");
            assertEquals(4, pred.getActionProbs().length);

        } finally {
            if (vw != null) {
                vw.close();
            }
        }
    }

    @Test
    public void testMergeModels() throws Exception {
        VowpalWabbitNative vw1 = null;
        VowpalWabbitNative vw2 = null;
        VowpalWabbitNative vwMerged = null;

        try {
            vw1 = new VowpalWabbitNative("--quiet");
            vw1.learnFromString("0 | price:.23 sqft:.25 age:.05 2006");
            vw2 = new VowpalWabbitNative("--quiet");
            vw2.learnFromString("1 'second_house | price:.18 sqft:.15 age:.35 1976");

            vwMerged = VowpalWabbitNative.mergeModels(null, new VowpalWabbitNative[] { vw1, vw2 });

            assertEquals(1.0, vw1.getPerformanceStatistics().getWeightedExampleSum(), 0.001);
            assertEquals(1.0, vw2.getPerformanceStatistics().getWeightedExampleSum(), 0.001);
            assertEquals(2.0, vwMerged.getPerformanceStatistics().getWeightedExampleSum(), 0.001);
        } finally {
            if (vw1 != null) {
                vw1.close();
            }
            if (vw2 != null) {
                vw2.close();
            }
            if (vwMerged != null) {
                vwMerged.close();
            }
        }
    }
}