package vowpalWabbit.learner.concurrent;

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
import vowpalWabbit.learner.VWMulticlassLearner;


public class VWConcurrentMulticlassMultilinePredictorTest {

    @Rule
    public TemporaryFolder temporaryFolder = new TemporaryFolder();
    
    private String trainAndPersistAMulticlassMultilineModel() throws IOException {
        
        // model preparation
        String[][] train = new String[][] {
                new String[] { "1  |w a", "2  |w b", "3  |w c", "3  |w d" },
                new String[] { "2  |w e" },
                new String[] { "1  |w f", "1  |w g", "1  |w h" },
                new String[] { "1  |w i", "1  |w j" },
                new String[] { "2  |w k", "3  |w l" },
                new String[] { "1  |w m", "1  |w n", "1  |w o", "2  |w p" },
                new String[] { "1  |w i", "2  |w k", "3  |w l", "3  |w d" }
        };
        
        String model = temporaryFolder.newFile().getAbsolutePath();
        VWMulticlassLearner vwmcLeaner = VWLearners
                .create("--quiet -b 24 -c --search_task sequence --search 45 --search_neighbor_features -1:w,1:w --affix -1w,+1w -f" + model);
        for (String[] aTrain : train) {
            vwmcLeaner.learn(aTrain);
        }
        vwmcLeaner.close();
        return model;
    }
    
    
    @Test
    public void testIdxLabelConversion() throws InterruptedException, IOException, ExecutionException {
        String modelFile = trainAndPersistAMulticlassMultilineModel();
        final VWConcurrentMulticlassMultilinePredictor vwmcLeaner = VWConcurrentLearnerFactory.createMulticlassMultilinePredictorBase("foo", 
                Executors.newFixedThreadPool(5), 5, "--quiet -t -i " + modelFile, false);
        
        final String[] testInputs = new String[]{"  |w i", "  |w j", "  |w k"};
        
        ExecutorService executor = Executors.newFixedThreadPool(10);
        CompletionService<PredictionOutput> completionService = new ExecutorCompletionService<PredictionOutput>(
                executor);
        
        String[] expectedStrPred = new String[] {"1", "1", "2"};
        
        // this makes sure that predictions are called in parallel.
        for(int i=0; i< 100; i++) {
            completionService.submit(new Callable<PredictionOutput>() {
                @Override
                public PredictionOutput call() throws Exception {
                    PredictionOutput output = new PredictionOutput();
                    output.input = testInputs;
                    output.prediction = vwmcLeaner.predict(testInputs, 10);
                    return output;
                }
                
            });
        }
        
        for (int i = 0; i < 100; i++) {
            Future<PredictionOutput> output = completionService
                    .poll(2000, TimeUnit.MILLISECONDS);
            Assert.assertArrayEquals(expectedStrPred, output.get().prediction);
        }
    }
    
    public static class PredictionOutput {
        String[] prediction;
        String[] input;
    }
}