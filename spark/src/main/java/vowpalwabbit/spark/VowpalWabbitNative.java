package vowpalwabbit.spark;

import java.io.Closeable;

public class VowpalWabbitNative implements Closeable {
    static {
        Native.load();
    }

    private static native long initialize(String args);

    private static native byte[] getModel(long nativePointer);

    private static native void endPass(long nativePointer);

    private static native void finish(long nativePointer);

    private long nativePointer;

    public VowpalWabbitNative(String args) {
        this.nativePointer = initialize(args);
    }

    public VowpalWabbitExample createExample() {
        return new VowpalWabbitExample(this.nativePointer, false);
    }

    public VowpalWabbitExample createEmptyExample() {
        return new VowpalWabbitExample(this.nativePointer, true);
    }

    public void endPass() {
        endPass(this.nativePointer);
    }

    public byte[] getModel() {
        return getModel(this.nativePointer);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            finish(this.nativePointer);
            this.nativePointer = 0;
        }
    }
}