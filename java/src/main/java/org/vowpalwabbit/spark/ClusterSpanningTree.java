package org.vowpalwabbit.spark;

import java.io.Closeable;

/**
 * Wraps the spanning tree coordinator native code used to orchestrate multipe VW instances.
 * 
 * @author Markus Cozowicz
 */
public class ClusterSpanningTree implements Closeable {
    static {
        Native.load();
    }

    private static native long create(int port);
    private native void delete();
    public native void start();
    public native void stop();
    public native int getPort();

    private long nativePointer;

    public ClusterSpanningTree(int port) {
        this.nativePointer = create(port);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            delete();
            this.nativePointer = 0;
        }
    }
}