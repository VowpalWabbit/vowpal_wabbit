package org.vowpalwabbit.spark.prediction;

/**
 * A scalar prediction.
 * 
 * @author Markus Cozowicz
 */
public class ScalarPrediction {
    private float value;
    private float confidence;

    public ScalarPrediction(float value, float confidence) {
        this.value = value;
        this.confidence = confidence;
    }

    /**
     * The predicted value.
     * 
     * @return predicted value.
     */
    public float getValue() { return value; }

    /**
     * The confidence of the {@code value}. Needs to be enabled using --confidence
     * 
     * @return confidence value.
     */
    public float getConfidence() { return confidence; }

    @Override
    public String toString() {
        return "ScalarPrediction(" + value + ", " + confidence + ")";
    }
}