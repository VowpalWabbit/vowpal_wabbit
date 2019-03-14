
package vowpalwabbit.spark;

import java.io.Closeable;

public class VowpalWabbitExample implements Closeable {
    private static native long initialize(long vwNativePointer, boolean isEmpty);

    private static native long finish(long nativePointer);

    private static native void clear(long nativePointer);

    private static native void addToNamespaceDense(long nativePointer, char ns, int baseIndex, double[] values);

    private static native void addToNamespaceSparse(long nativePointer, char ns, int[] indices, double[] values);

    // https://github.com/VowpalWabbit/vowpal_wabbit/blob/master/cs/cli/vw_label.h
    // TODO: support others too (e.g. multiclass)

    private static native void setLabel(long nativePointer, float label);

    private static native void setLabelWithWeight(long nativePointer, float weight, float label);

    private static native void learn(long nativePointer);
    
    private static native void predict(long nativePointer);

    private static native Object getPrediction(long nativePointer);

    // this points to a C++ example wrapper class
    private long nativePointer;
        
    VowpalWabbitExample(long vwNativePointer, boolean isEmpty) {
        this.nativePointer = initialize(vwNativePointer, isEmpty);
    }

    public void clear() {
        clear(this.nativePointer);
    }

    public void addToNamespaceDense(char ns, int baseIndex, double[] values) {
        addToNamespaceDense(this.nativePointer, ns, baseIndex, values);
    }

    public void addToNamespaceSparse(char ns, int[] indices, double[] values) {
        addToNamespaceSparse(this.nativePointer, ns, indices, values);
    }

    public void setLabel(float weight, float label) {
        setLabelWithWeight(this.nativePointer, weight, label);
    }

    public void setLabel(float label) {
        setLabel(this.nativePointer, label);
    }

    public void learn() {
        learn(this.nativePointer);
    }

    public void predict() {
        predict(this.nativePointer);
    }

    public Object getPrediction() {
        return getPrediction(this.nativePointer);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            finish(this.nativePointer);
            this.nativePointer = 0;
        }
    }
}