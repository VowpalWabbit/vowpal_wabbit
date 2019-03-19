package vowpalwabbit.spark;

/**
 * Holds information extract from the VW command line.
 * 
 * @author Markus Cozowicz
 */
public class VowpalWabbitArguments { 
    private int numBits;
    private int hashSeed;

    public VowpalWabbitArguments(int numBits, int hashSeed) {
        this.numBits = numBits;
        this.hashSeed = hashSeed;
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
}