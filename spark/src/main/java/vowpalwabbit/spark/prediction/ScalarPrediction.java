package vowpalwabbit.spark.prediction;

public class ScalarPrediction {
    private float value;
    private float confidence;

    public ScalarPrediction(float value, float confidence) {
        this.value = value;
        this.confidence = confidence;
    }

    public float getValue() { return value; }

    public float getConfidence() { return confidence; }

    @Override
    public String toString() {
        return "ScalarPrediction(" + value + ", " + confidence + ")";
    }
}