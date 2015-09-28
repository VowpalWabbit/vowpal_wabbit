package vw;

public final class LDA extends VWGenericBase<float[]> {
    public LDA(String command) {
        super(command);
    }

    protected native float[] predict(String example, boolean learn, long nativePointer);
}
