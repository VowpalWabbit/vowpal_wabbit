package vowpalwabbit.spark;

import java.io.Closeable;

public class ClusterSpanningTree implements Closeable {
    static {
        Native.load();
    }

    private static native long create();
    private static native void delete(long nativePointer);
    private static native void start(long nativePointer);
    private static native void stop(long nativePointer);

    private long nativePointer;

    public ClusterSpanningTree() {
        this.nativePointer = create();
    }

    public void start() {
        start(this.nativePointer);
    }

    public void stop() {
        stop(this.nativePointer);
    }

    @Override
    final public void close() {
        if (this.nativePointer != 0) {
            delete(this.nativePointer);
            this.nativePointer = 0;
        }
    }
}