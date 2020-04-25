package org.vowpalwabbit.spark;

/**
 * Holds information extract from the VW command line.
 * 
 * @author Markus Cozowicz
 */
public class VowpalWabbitArguments implements java.io.Serializable {
    private static final long serialVersionUID = 1L;

    private int numBits;
    private int hashSeed;
    private String args;
    private double learningRate;
    private double powerT;

    public VowpalWabbitArguments(int numBits, int hashSeed, String args, double learningRate, double powerT) {
        this.numBits = numBits;
        this.hashSeed = hashSeed;
        this.args = args;
        this.learningRate = learningRate;
        this.powerT = powerT;
    }

    /**
     * Number of bits supplied by -b.
     * 
     * @return number of bits.
     */
    public int getNumBits() {
        return numBits;
    }

    /**
     * Hash seed supplied by --hash_seed.
     * 
     * @return hash seed.
     */
    public int getHashSeed() {
        return hashSeed;
    }

    /**
     * Command line arguments.
     * 
     * @return command line arguments stored in model.
     */
    public String getArgs() {
        return args;
    }

    /**
     * Learning rate is not encoded into the command line argument. Exposing
     * separately.
     * 
     * @return the learning rate.
     */
    public double getLearningRate() {
        return learningRate;
    }

    /**
     * PowerT is not encoded into the command line argument. Exposing separately.
     * 
     * @return power t
     */
    public double getPowerT() {
        return powerT;
    }
}