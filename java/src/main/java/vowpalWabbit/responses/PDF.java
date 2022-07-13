package vowpalWabbit.responses;

import java.io.Serializable;
import java.util.Arrays;

/**
 * Created by eisber 7/5/2022.
 */
public class PDF implements Serializable {

    // Although this is modifiable it is not intended to be updated by the user.  This data structure mimics the
    // C data structure.
    private final PDFSegment[] pdfSegments;

    public PDF(final PDFSegment[] pdfSegments) {
        this.pdfSegments = pdfSegments;
    }

    public PDFSegment[] getPDFSegments() {
        return pdfSegments;
    }

    @Override
    public String toString() {
        return "PDF{" +
                "pdfSegments=" + Arrays.toString(pdfSegments) +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        PDF that = (PDF) o;

        // Probably incorrect - comparing Object[] arrays with Arrays.equals
        return Arrays.equals(pdfSegments, that.pdfSegments);

    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(pdfSegments);
    }
}
