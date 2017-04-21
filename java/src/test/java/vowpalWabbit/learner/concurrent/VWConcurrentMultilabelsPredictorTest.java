package vowpalWabbit.learner.concurrent;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletionService;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorCompletionService;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import vowpalWabbit.learner.VWLearners;
import vowpalWabbit.learner.VWMultilabelsLearner;
import vowpalWabbit.responses.Multilabels;

public class VWConcurrentMultilabelsPredictorTest {

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();
    
    private String trainAndPersistAMultilabelModel() throws IOException {
        // Note that the expected values in this test were obtained by running
        // vw from the command line as follows
        // echo -e "1 | a\n2 | a b\n3 | a c\n2 | a b\n3 | b c\n1 | a c\n2 | d" | ../vowpalwabbit/vw --multilabel_oaa 4 -f multilabel.model -p multilabel.train.out
        // echo -e "| a b c d\n| b d" | ../vowpalwabbit/vw -t -i multilabel.model -p multilabel.test.out
        String[] train = new String[]{
                "1 | a",
                "2 | a b",
                "3 | a c",
                "2 | a b",
                "3 | b c",
                "1 | a c",
                "2 | d"
        };
        String model = temporaryFolder.newFile().getAbsolutePath();
        VWMultilabelsLearner vw = VWLearners.create("--quiet --multilabel_oaa 4 -f " + model);
        Multilabels[] trainPreds = new Multilabels[train.length];
        for (int i=0; i<train.length; ++i) {
            trainPreds[i] = vw.learn(train[i]);
        }
        Multilabels[] expectedTrainPreds = new Multilabels[]{
                new Multilabels(new int[]{}),
                new Multilabels(new int[]{1}),
                new Multilabels(new int[]{1}),
                new Multilabels(new int[]{}),
                new Multilabels(new int[]{2}),
                new Multilabels(new int[]{3}),
                new Multilabels(new int[]{})
        };
        vw.close();
        
        return model;
    }
    
    @Test
    public void testMultilabelsConcurrency() throws FileNotFoundException,
            IOException, InterruptedException, ExecutionException {
        
        String modelFile = trainAndPersistAMultilabelModel();
        
        final VWConcurrentMultilabelsPredictor predictor = VWConcurrentLearnerFactory.createMultilabelsPredictor("foo", 
                Executors.newFixedThreadPool(5), 5, "--quiet -t -i " + modelFile);
        
        final String[] test = new String[]{
                "| a b c d",
                "| b d"
        };
        Multilabels[] expectedTestPreds = new Multilabels[]{new Multilabels(new int[]{}), new Multilabels(new int[]{2})};
        
        ExecutorService executor = Executors.newFixedThreadPool(10);
        CompletionService<PredictionOutput> completionService = new ExecutorCompletionService<PredictionOutput>(
                executor);

        for (int i=0; i<100; ++i) {
            final int inputIndex = i % test.length;
            completionService.submit(new Callable<PredictionOutput>() {
                @Override
                public PredictionOutput call() throws Exception {
                    PredictionOutput output = new PredictionOutput();
                    output.inputIndex = inputIndex;
                    output.input = test[output.inputIndex];
                    output.prediction = predictor.predict(output.input, 10);
                    return output;
                }
            });
        }
        
        for (int i = 0; i < 100; i++) {
            Future<PredictionOutput> output = completionService
                    .poll(2000, TimeUnit.MILLISECONDS);
            PredictionOutput prediction = output.get(); 
            Assert.assertEquals(expectedTestPreds[prediction.inputIndex], prediction.prediction);
        }
        
    }
    
    public static class PredictionOutput {
        Multilabels prediction;
        int inputIndex;
        String input;
    }
}
