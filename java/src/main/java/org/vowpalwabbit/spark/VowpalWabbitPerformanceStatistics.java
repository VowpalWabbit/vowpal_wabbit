package org.vowpalwabbit.spark;

/**
 * Holds performance statistics (e.g. the last block printed in the command line
 * version).
 * 
 * @author Markus Cozowicz
 */
public class VowpalWabbitPerformanceStatistics implements java.io.Serializable {
	private static final long serialVersionUID = 1L;

	private long numberOfExamplesPerPass;

	private double weightedExampleSum;

	private double weightedLabelSum;

	private double averageLoss;

	private float bestConstant;

	private float bestConstantLoss;

	private long totalNumberOfFeatures;

	public VowpalWabbitPerformanceStatistics(long numberOfExamplesPerPass, double weightedExampleSum,
			double weightedLabelSum, double averageLoss, float bestConstant, float bestConstantLoss,
			long totalNumberOfFeatures) {

		this.numberOfExamplesPerPass = numberOfExamplesPerPass;
		this.weightedExampleSum = weightedExampleSum;
		this.weightedLabelSum = weightedLabelSum;
		this.averageLoss = averageLoss;
		this.bestConstant = bestConstant;
		this.bestConstantLoss = bestConstantLoss;
		this.totalNumberOfFeatures = totalNumberOfFeatures;
	}

	/**
	 * @return the averageLoss
	 */
	public double getAverageLoss() {
		return averageLoss;
	}

	/**
	 * @return the bestConstant
	 */
	public float getBestConstant() {
		return bestConstant;
	}

	/**
	 * @return the bestConstantLoss
	 */
	public float getBestConstantLoss() {
		return bestConstantLoss;
	}

	/**
	 * @return the numberOfExamplesPerPass
	 */
	public long getNumberOfExamplesPerPass() {
		return numberOfExamplesPerPass;
	}

	/**
	 * @return the totalNumberOfFeatures
	 */
	public long getTotalNumberOfFeatures() {
		return totalNumberOfFeatures;
	}

	/**
	 * @return the weightedExampleSum
	 */
	public double getWeightedExampleSum() {
		return weightedExampleSum;
	}

	/**
	 * @return the weightedLabelSum
	 */
	public double getWeightedLabelSum() {
		return weightedLabelSum;
	}
}
