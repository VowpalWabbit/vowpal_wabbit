package org.vowpalwabbit.spark;

import java.io.Closeable;

/**
 * Main wrapper for VowpalWabbit native implementation.
 * 
 * @author Markus Cozowicz
 */
public class VowpalWabbitNative implements Closeable {
    static {
        // load the native libraries
        Native.load();
    }

    /**
     * Initializes the native VW data structures.
     * 
     * @param args VW command line arguments.
     * @return pointer to vw data structure defined in global_data.h.
     */
    private static native long initialize(String args);

    /**
     * Initializes the native VW data structures.
     * 
     * <p>
     * Note: The {@code args} must be compatible with the command line arguments
     * stored in {@code model}.
     * </p>
     * 
     * @param args  VW command line arguments.
     * @param model VW model to initialize this instance from.
     * @return pointer to vw data structure defined in global_data.h.
     */
    private static native long initializeFromModel(String args, byte[] model);

    /**
     * Perform remaining passes.
     */
    public native void performRemainingPasses();

    /**
     * Returns a snapshot of the current model.
     * 
     * @return serialized VW model.
     */
    public native byte[] getModel();

    /**
     * Returns a subset of the current arguments VW received (e.g. numbits)
     * 
     * @return VW argument object.
     */
    public native VowpalWabbitArguments getArguments();

    public native VowpalWabbitPerformanceStatistics getPerformanceStatistics();

    /**
     * Signals the end of the current pass over the data.
     */
    public native void endPass();

    /**
     * Free's the vw data structure.
     */
    private native void finish();

    /**
     * Invokes the native implementation of Murmur hash. Exposed through
     * VowpalWabbitMurmur.
     */
    static native int hash(byte[] data, int offset, int len, int seed);

    /**
     * Pointer to vw data structure defined in global_data.h
     */
    private long nativePointer;

    /**
     * Initializes the native VW data structures.
     * 
     * @param args VW command line arguments.
     */
    public VowpalWabbitNative(String args) {
        this.nativePointer = initialize(args);
    }

    /**
     * Initializes the native VW data structures.
     * 
     * <p>
     * Note: The {@code args} must be compatible with the command line arguments
     * stored in {@code model}.
     * </p>
     * 
     * @param args  VW command line arguments.
     * @param model VW model to initialize this instance from.
     */
    public VowpalWabbitNative(String args, byte[] model) {
        this.nativePointer = initializeFromModel(args, model);
    }

    /**
     * Creates a new VW example associated with this this instance.
     * 
     * @return new {@code VowpalWabbitExample} object.
     */
    public VowpalWabbitExample createExample() {
        return new VowpalWabbitExample(this.nativePointer, false);
    }

    /**
     * Creates a new empty VW example associated with this this instance. This is
     * used to mark the end of a multiline example.
     * 
     * @return new {@code VowpalWabbitExample} object.
     */
    public VowpalWabbitExample createEmptyExample() {
        return new VowpalWabbitExample(this.nativePointer, true);
    }

    /**
     * Frees the native resources.
     */
    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            finish();
            this.nativePointer = 0;
        }
    }
}