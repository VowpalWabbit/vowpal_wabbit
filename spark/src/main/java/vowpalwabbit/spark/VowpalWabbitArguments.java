package vowpalwabbit.spark;

/**
 * @author Markus Cozowicz
 */
public class VowpalWabbitArguments { 
    private int numBits;
    private int hashSeed;

    public VowpalWabbitArguments(int numBits, int hashSeed) {
        this.numBits = numBits;
        this.hashSeed = hashSeed;
    }

    public int getNumBits() { return numBits; }

    public int getHashSeed() { return hashSeed; }
}