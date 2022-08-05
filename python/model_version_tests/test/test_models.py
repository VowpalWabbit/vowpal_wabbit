from vowpalwabbit import pyvw

def test_cb_model_with_regularization():
    vw = pyvw.vw(quiet=True, i="cb_explore_adf_model_with_regularization.vw")
    assert vw.get_weight_from_name("b", "User") != 0
    assert vw.get_weight_from_name("d", "Action") != 0
    assert vw.get_weight_from_name("e", "Action") != 0
    assert vw.get_weight_from_name("f", "Action") != 0
    assert vw.get_weight_from_name("ff", "Action") != 0


