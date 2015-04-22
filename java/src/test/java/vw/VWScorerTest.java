package vw;

import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import org.junit.Ignore;

/**
 * Created by jmorra on 11/24/14.
 */
public class VWScorerTest {
    private static final String houseModel = "target/house.model";
    private static final String houseData = "src/test/resources/house.vw";

    //Note: had to remove the static init for the scorer, because it made the tests interact with each other and fail,
    //      even when using different features and/or different namespaces with the same scorer,
    //      or even with two different scorer instances.. !
  
    @Test
    public void testBlank() throws IOException, InterruptedException {
        VWScorer scorer = getScorer();    
        float prediction = scorer.getPrediction("| ");
        assertEquals(0.075, prediction, 0.001);
    }

    @Test
    public void testLine() throws IOException, InterruptedException {
        VWScorer scorer = getScorer();
        float prediction = scorer.getPrediction("| price:0.23 sqft:0.25 age:0.05 2006");
        assertEquals(0.118, prediction, 0.001);
    }
 
   
    @Test
    public void testLearn() throws IOException, InterruptedException {      
        VWScorer learner = getScorer();  
        float firstPrediction = learner.getPrediction("|f height:0.23 weight:0.25 width:0.05");
        
        learner.doLearnAndGetPrediction("0.1 |f height:0.23 weight:0.25 width:0.05");
        float secondPrediction = learner.getPrediction("|f height:0.23 weight:0.25 width:0.05");
    
        learner.doLearnAndGetPrediction("0.9 |f height:0.23 weight:0.25 width:0.05");
        float thirdPrediction = learner.getPrediction("|f height:0.23 weight:0.25 width:0.05"); 
 
        assertNotEquals(firstPrediction, secondPrediction, 0.001);
        assertNotEquals(firstPrediction, thirdPrediction, 0.001);
        assertNotEquals(secondPrediction, thirdPrediction, 0.001);


    }

    private VWScorer getScorer() throws IOException, InterruptedException {
        // Since we want this test to continue to work between VW changes, we can't store the model
        // Instead, we'll make a new model for each test
        Runtime.getRuntime().exec("../vowpalwabbit/vw -d " + houseData + " -f " + houseModel).waitFor();
        VWScorer scorer = new VWScorer("--quiet -t -i " + houseModel);
        return scorer;
    }


}
