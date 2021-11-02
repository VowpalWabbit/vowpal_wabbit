package org.vowpalwabbit.spark;

import common.Native;
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

    private static native long create(int port, boolean quiet);
    private native void delete();
    public native void start();
    public native void stop();
    public native int getPort();

    private long nativePointer;

    public ClusterSpanningTree(int port, boolean quiet) {
        this.nativePointer = create(port, quiet);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            delete();
            this.nativePointer = 0;
        }
    }
}