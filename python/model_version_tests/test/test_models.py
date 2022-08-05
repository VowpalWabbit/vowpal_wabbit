import vowpalwabbit

def test_cb_model_with_regularization():
    vw = vowpalwabbit.Workspace(quiet=True, model="vw_generated_models/cb_explore_adf_model_with_regularization.vw")
    assert vw.get_weight_from_name("b", "User") != 0
    assert vw.get_weight_from_name("d", "Action") != 0
    assert vw.get_weight_from_name("e", "Action") != 0
    assert vw.get_weight_from_name("f", "Action") != 0
    assert vw.get_weight_from_name("ff", "Action") != 0


