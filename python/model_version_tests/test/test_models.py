import vowpalwabbit


def test_cb_model_with_regularization():
    vw = vowpalwabbit.Workspace(
        quiet=True, i="vw_generated_models/cb_explore_adf_model_with_regularization.vw"
    )
    assert vw.get_weight_from_name("b", "User") != 0
    assert vw.get_weight_from_name("fff", "Action") != 0
    vw.finish()
