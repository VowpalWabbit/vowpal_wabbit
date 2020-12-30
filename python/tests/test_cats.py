from vowpalwabbit import pyvw

import pytest

def test_cats():
    min_value = 10
    max_value = 20

    vw = pyvw.vw("--cats 4 --min_value " + str(min_value) + " --max_value " + str(max_value) + " --bandwidth 1")
    vw_example = vw.parse("ca 15:0.657567:6.20426e-05 | f1 f2 f3 f4", pyvw.vw.lContinuous)
    vw.learn(vw_example)
    vw.finish_example(vw_example)

    assert vw.get_prediction_type() == vw.pACTION_PDF_VALUE, "prediction_type should be action_pdf_value"

    action, pdf_value = vw.predict("| f1 f2 f3 f4")
    assert action >= 10
    assert action <= 20
    vw.finish()

def test_cats_pdf():
    min_value = 10
    max_value = 20

    vw = pyvw.vw("--cats_pdf 4 --min_value " + str(min_value) + " --max_value " + str(max_value) + " --bandwidth 1")
    vw_example = vw.parse("ca 15:0.657567:6.20426e-05 | f1 f2 f3 f4", pyvw.vw.lContinuous)
    vw.learn(vw_example)
    vw.finish_example(vw_example)

    assert vw.get_prediction_type() == vw.pPDF, "prediction_type should be pdf"

    pdf_segments = vw.predict("| f1 f2 f3 f4")
    for segment in pdf_segments:
        assert len(segment) == 3
        assert segment[0] >= min_value
        assert segment[0] <= max_value
        assert segment[1] >= min_value
        assert segment[1] <= max_value
    vw.finish()
