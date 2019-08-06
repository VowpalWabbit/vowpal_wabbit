package org.vowpalwabbit.spark;

/**
 * Holds information extract from the VW command line.
 * 
 * @author Markus Cozowicz
 */
public class VowpalWabbitArguments { 
    private int numBits;
    private int hashSeed;
    private String args;

    public VowpalWabbitArguments(int numBits, int hashSeed, String args) {
        this.numBits = numBits;
        this.hashSeed = hashSeed;
        this.args = args;
    }

    /**
     * Number of bits supplied by -b.
     * @return number of bits.
     */
    public int getNumBits() { return numBits; }

    /**
     * Hash seed supplied by --hash_seed.
     * @return hash seed.
     */
    public int getHashSeed() { return hashSeed; }

    /**
     * Command line arguments.
     * @return command line arguments stored in model.
     */
    public String getArgs() { return args; }
}