package vowpalwabbit.spark;

import java.io.Closeable;

/**
 * @author Markus Cozowicz
 */
public class VowpalWabbitNative implements Closeable {
    static {
        Native.load();
    }

    private static native long initialize(String args);

    private static native long initializeFromModel(String args, byte[] model);

    public native byte[] getModel();

    public native VowpalWabbitArguments getArguments();

    public native void endPass();

    private static native void finish(long nativePointer);

    private long nativePointer;

    public VowpalWabbitNative(String args) {
        this.nativePointer = initialize(args);
    }

    public VowpalWabbitNative(String args, byte[] model) {
        this.nativePointer = initializeFromModel(args, model);
    }

    public VowpalWabbitExample createExample() {
        return new VowpalWabbitExample(this.nativePointer, false);
    }

    public VowpalWabbitExample createEmptyExample() {
        return new VowpalWabbitExample(this.nativePointer, true);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            finish(this.nativePointer);
            this.nativePointer = 0;
        }
    }
}