package vowpalWabbit.responses;

import java.io.Serializable;

/**
 * Created by eisber 7/5/2022.
 */
public class PDFValue implements Serializable {

    // Although this is modifiable it is not intended to be updated by the user.  This data structure mimics the
    // C data structure.
    private final float action;
    private final float pdfValue;

    public PDFValue(float action, float pdfValue) {
        this.action = action;
        this.pdfValue = pdfValue;
    }

    public float getPDFValue() {
        return pdfValue;
    }

    public float getAction() {
        return action;
    }

    @Override
    public String toString() {
        return "PDFValue{" +
                "action=" + action + ", " + 
                "pdfValue=" + pdfValue  + 
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        PDFValue that = (PDFValue) o;

        return this.action == that.action &&
            this.pdfValue == that.pdfValue;
    }

    @Override
    public int hashCode() {
        // TODO: who needs this!?
        return 
            (action != +0.0f ? Float.floatToIntBits(action) : 0) + 
            (pdfValue != +0.0f ? Float.floatToIntBits(pdfValue) : 0);
    }
}
