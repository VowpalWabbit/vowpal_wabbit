package vowpalWabbit.responses;

import java.io.Serializable;

/**
 * Created by eisber 7/5/2022.
 */
public class PDFSegment implements Serializable {

    // Although this is modifiable it is not intended to be updated by the user.  This data structure mimics the
    // C data structure.
    private final float left;
    private final float right;
    private final float pdfValue;

    public PDFSegment(float left, float right, float pdfValue) {
        this.left = left;
        this.right = right;
        this.pdfValue = pdfValue;
    }

    public float getPDFValue() {
        return pdfValue;
    }

    public float getLeft() {
        return left;
    }

    public float getRight() {
        return right;
    }

    @Override
    public String toString() {
        return "PDFSegment{" +
                "left=" + left + ", " + 
                "right=" + right + ", " + 
                "pdfValue=" + pdfValue +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        PDFSegment that = (PDFSegment) o;

        return this.left == that.left &&
            this.right == that.right &&
            this.pdfValue == that.pdfValue;
    }

    @Override
    public int hashCode() {
        // TODO: who needs this!?
        return 
            (left != +0.0f ? Float.floatToIntBits(left) : 0) + 
            (right != +0.0f ? Float.floatToIntBits(right) : 0) + 
            (pdfValue != +0.0f ? Float.floatToIntBits(pdfValue) : 0);
    }
}
