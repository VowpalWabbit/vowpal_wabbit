
package vowpalwabbit.spark;

import java.io.Closeable;

public class VowpalWabbitExample implements Closeable {
    private static native long initialize(long vwNativePointer, boolean isEmpty);

    private static native long finish(long nativePointer);

    public native void clear();

    public native void addToNamespaceDense(char ns, int baseIndex, double[] values);

    public native void addToNamespaceSparse(char ns, int[] indices, double[] values);

    // https://github.com/VowpalWabbit/vowpal_wabbit/blob/master/cs/cli/vw_label.h
    // TODO: support others too (e.g. multiclass)

    public native void setLabel(float weight, float label);

    public native void learn();
    
    public native Object predict();

    public native Object getPrediction();

    // this points to a C++ example wrapper class
    private long nativePointer;
        
    VowpalWabbitExample(long vwNativePointer, boolean isEmpty) {
        this.nativePointer = initialize(vwNativePointer, isEmpty);
    }

    public void setLabel(float label) {
        setLabel(1f, label);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            finish(this.nativePointer);
            this.nativePointer = 0;
        }
    }
}