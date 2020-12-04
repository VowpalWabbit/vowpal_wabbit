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
