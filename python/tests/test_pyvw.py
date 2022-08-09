import os
from pathlib import Path

import vowpalwabbit
from vowpalwabbit import Workspace
import pytest
import warnings
import filecmp

BIT_SIZE = 18

# Since these tests still run with Python 2, this is required.
# Otherwise we could use math.isclose
def isclose(a, b, rel_tol=1e-05, abs_tol=0.0):
    return abs(a - b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)


class TestVW:

    model = Workspace(quiet=True, b=BIT_SIZE)

    def test_constructor(self):
        assert isinstance(self.model, Workspace)

    def test_learn_predict(self):
        ex = self.model.example("1 | a b c")
        init = self.model.predict(ex)
        assert init == 0
        self.model.learn(ex)
        assert self.model.predict(ex) > init
        ex = ["| a", "| b"]
        check_error_raises(TypeError, lambda: self.model.predict(ex))
        check_error_raises(TypeError, lambda: self.model.learn(ex))

    def test_get_tag(self):
        ex = self.model.example("1 foo| a b c")
        assert ex.get_tag() == "foo"
        ex = self.model.example("1 1.0 bar| a b c")
        assert ex.get_tag() == "bar"
        ex = self.model.example("1 'baz | a b c")
        assert ex.get_tag() == "baz"

    def test_num_weights(self):
        assert self.model.num_weights() == 2**BIT_SIZE

    def test_get_weight(self):
        assert self.model.get_weight(0, 0) == 0

    def test_finish(self):
        assert not self.model.finished
        self.model.finish()
        assert self.model.finished


def test_delete():
    model = Workspace(quiet=True, b=BIT_SIZE)
    assert "model" in locals()
    del model
    assert "model" not in locals()


# Test prediction types


def test_scalar_prediction_type():
    model = Workspace(quiet=True)
    model.learn("1 | a b c")
    assert model.get_prediction_type() == model.pSCALAR
    prediction = model.predict(" | a b c")
    assert isinstance(prediction, float)
    del model


def test_scalars_prediction_type():
    n = 3
    model = Workspace(loss_function="logistic", oaa=n, probabilities=True, quiet=True)
    model.learn("1 | a b c")
    assert model.get_prediction_type() == vowpalwabbit.PredictionType.SCALARS
    prediction = model.predict(" | a b c")
    assert isinstance(prediction, list)
    assert len(prediction) == n
    del model


def test_multiclass_prediction_type():
    n = 3
    model = Workspace(loss_function="logistic", oaa=n, quiet=True)
    model.learn("1 | a b c")
    assert model.get_prediction_type() == vowpalwabbit.PredictionType.MULTICLASS
    prediction = model.predict(" | a b c")
    assert isinstance(prediction, int)
    del model


def test_prob_prediction_type():
    model = Workspace(
        loss_function="logistic",
        csoaa_ldf="mc",
        probabilities=True,
        quiet=True,
    )
    multi_ex = [
        model.example("1:0.2 | a b c"),
        model.example("2:0.8  | a b c"),
    ]
    model.learn(multi_ex)
    assert model.get_prediction_type() == vowpalwabbit.PredictionType.PROB
    multi_ex = [model.example("1 | a b c"), model.example("2 | a b c")]
    prediction = model.predict(multi_ex)
    assert isinstance(prediction, float)
    del model


def test_action_scores_prediction_type():
    model = Workspace(loss_function="logistic", csoaa_ldf="m", quiet=True)
    multi_ex = [model.example("1:1 | a b c"), model.example("2:-1  | a b c")]
    model.learn(multi_ex)
    assert model.get_prediction_type() == model.pMULTICLASS
    multi_ex = [model.example("1 | a b c"), model.example("2 | a b c")]
    prediction = model.predict(multi_ex)
    assert isinstance(prediction, int)
    del model


def test_action_probs_prediction_type():
    model = Workspace(cb_explore=2, ngram=2, quiet=True)
    model.learn("1 | a b c")
    assert model.get_prediction_type() == model.pACTION_PROBS
    prediction = model.predict(" | a b c")
    assert isinstance(prediction, list)
    del model


def test_multilabel_prediction_type():
    model = Workspace(multilabel_oaa=4, quiet=True)
    model.learn("1 | a b c")
    assert model.get_prediction_type() == model.pMULTILABELS
    prediction = model.predict(" | a b c")
    assert isinstance(prediction, list)
    del model


def test_CBLabel():
    model = Workspace(cb=4, quiet=True)
    cbl = vowpalwabbit.CBLabel.from_example(model.example("1:10:0.5 |"))
    assert cbl.weight == 1.0
    assert cbl.costs[0].action == 1
    assert cbl.costs[0].probability == 0.5
    assert cbl.costs[0].partial_prediction == 0
    assert cbl.costs[0].cost == 10.0
    assert str(cbl) == "1:10.0:0.5"
    del model


def test_CBEvalLabel():
    model = Workspace(cb=4, eval=True, quiet=True)
    cbel = vowpalwabbit.CBEvalLabel.from_example(model.example("3 1:10:0.5 |"))
    assert cbel.action == 3
    assert cbel.cb_label.weight == 1.0
    assert cbel.cb_label.costs[0].action == 1
    assert cbel.cb_label.costs[0].probability == 0.5
    assert cbel.cb_label.costs[0].partial_prediction == 0
    assert cbel.cb_label.costs[0].cost == 10.0
    assert str(cbel) == "3 1:10.0:0.5"
    del model


def test_CBContinuousLabel():
    model = Workspace(
        cats=4, min_value=185, max_value=23959, bandwidth=3000, quiet=True
    )
    cb_contl = vowpalwabbit.CBContinuousLabel.from_example(
        model.example("ca 1:10:0.5 |")
    )
    assert cb_contl.costs[0].action == 1
    assert cb_contl.costs[0].pdf_value == 0.5
    assert cb_contl.costs[0].cost == 10.0
    assert str(cb_contl) == "ca 1:10.0:0.5"
    del model


def test_CostSensitiveLabel():
    model = Workspace(csoaa=4, quiet=True)
    csl = vowpalwabbit.CostSensitiveLabel.from_example(model.example("2:5 |"))
    assert csl.costs[0].label == 2
    assert csl.costs[0].wap_value == 0.0
    assert csl.costs[0].partial_prediction == 0.0
    assert csl.costs[0].cost == 5.0
    assert str(csl) == "2:5.0"
    del model


def test_MulticlassProbabilitiesLabel():
    n = 4
    model = vowpalwabbit.Workspace(
        loss_function="logistic", oaa=n, probabilities=True, quiet=True
    )
    ex = model.example("1 | a b c d", 2)
    model.learn(ex)
    mpl = vowpalwabbit.MulticlassProbabilitiesLabel.from_example(ex)
    assert str(mpl) == "1:0.25 2:0.25 3:0.25 4:0.25"
    mpl = vowpalwabbit.MulticlassProbabilitiesLabel([0.4, 0.3, 0.3])
    assert str(mpl) == "1:0.4 2:0.3 3:0.3"


def test_ccb_label():
    model = Workspace(ccb_explore_adf=True, quiet=True)
    ccb_shared_label = vowpalwabbit.CCBLabel.from_example(
        model.example("ccb shared | shared_0 shared_1")
    )
    ccb_action_label = vowpalwabbit.CCBLabel.from_example(
        model.example("ccb action | action_1 action_3")
    )
    ccb_slot_label = vowpalwabbit.CCBLabel.from_example(
        model.example("ccb slot 0:0.8:1.0 0 | slot_0")
    )
    ccb_slot_pred_label = vowpalwabbit.CCBLabel.from_example(
        model.example("ccb slot |")
    )
    assert ccb_shared_label.type == vowpalwabbit.CCBLabelType.SHARED
    assert len(ccb_shared_label.explicit_included_actions) == 0
    assert ccb_shared_label.outcome is None
    assert str(ccb_shared_label) == "ccb shared"
    assert ccb_action_label.type == vowpalwabbit.CCBLabelType.ACTION
    assert len(ccb_action_label.explicit_included_actions) == 0
    assert ccb_action_label.weight == 1.0
    assert ccb_action_label.outcome is None
    assert str(ccb_action_label) == "ccb action"
    assert ccb_slot_label.type == vowpalwabbit.CCBLabelType.SLOT
    assert ccb_slot_label.explicit_included_actions[0] == 0
    assert ccb_slot_label.outcome.action_probs[0].action == 0
    assert isclose(ccb_slot_label.outcome.action_probs[0].score, 1.0)
    assert isclose(ccb_slot_label.outcome.cost, 0.8)
    assert str(ccb_slot_label) == "ccb slot 0:0.8:1.0 0"
    assert ccb_slot_pred_label.type == vowpalwabbit.CCBLabelType.SLOT
    assert len(ccb_slot_pred_label.explicit_included_actions) == 0
    assert ccb_slot_pred_label.outcome is None
    assert str(ccb_slot_pred_label) == "ccb slot"
    del model


def test_slates_label():
    model = Workspace(slates=True, quiet=True)
    slates_shared_label = vowpalwabbit.SlatesLabel.from_example(
        model.example("slates shared 0.8 | shared_0 shared_1")
    )
    slates_action_label = vowpalwabbit.SlatesLabel.from_example(
        model.example("slates action 1 | action_3")
    )
    slates_slot_label = vowpalwabbit.SlatesLabel.from_example(
        model.example("slates slot 1:0.8,0:0.1,2:0.1 | slot_0")
    )
    assert slates_shared_label.type == vowpalwabbit.SlatesLabelType.SHARED
    assert slates_shared_label.labeled == True
    assert isclose(slates_shared_label.cost, 0.8)
    assert str(slates_shared_label) == "slates shared 0.8"
    assert slates_action_label.type == vowpalwabbit.SlatesLabelType.ACTION
    assert slates_action_label.labeled == False
    assert slates_action_label.weight == 1.0
    assert slates_action_label.slot_id == 1
    assert str(slates_action_label) == "slates action 1"
    assert slates_slot_label.type == vowpalwabbit.SlatesLabelType.SLOT
    assert slates_slot_label.labeled == True
    assert slates_slot_label.probabilities[0].action == 1
    assert isclose(slates_slot_label.probabilities[0].score, 0.8)
    assert slates_slot_label.probabilities[1].action == 0
    assert isclose(slates_slot_label.probabilities[1].score, 0.1)
    assert slates_slot_label.probabilities[2].action == 2
    assert isclose(slates_slot_label.probabilities[2].score, 0.1)
    assert str(slates_slot_label) == "slates slot 1:0.8,0:0.1,2:0.1"
    del model


def test_multilabel_label():
    model = Workspace(multilabel_oaa=5, quiet=True)
    multil = vowpalwabbit.MultilabelLabel.from_example(model.example("1,2,3 |"))
    assert len(multil.labels) == 3
    assert multil.labels[0] == 1
    assert multil.labels[1] == 2
    assert multil.labels[2] == 3
    assert str(multil) == "1,2,3"


def test_regressor_args():
    # load and parse external data file
    data_file = os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "resources", "train.dat"
    )
    model = Workspace(oaa=3, data=data_file, passes=30, c=True, k=True)
    assert model.predict("| feature1:2.5") == 1

    # update model in memory
    for _ in range(10):
        model.learn("3 | feature1:2.5")
    assert model.predict("| feature1:2.5") == 3

    # save model
    model.save("tmp.model")
    del model

    # load initial regressor and confirm updated prediction
    new_model = Workspace(i="tmp.model", quiet=True)
    assert new_model.predict("| feature1:2.5") == 3
    del new_model

    # clean up
    os.remove("{}.cache".format(data_file))
    os.remove("tmp.model")


def test_save_to_Path():
    model = Workspace(quiet=True)
    model.learn("1 | a b c")
    model.save(Path("tmp1.model"))
    model.save("tmp2.model")
    assert filecmp.cmp("tmp1.model", "tmp2.model")
    os.remove("tmp1.model")
    os.remove("tmp2.model")


def test_command_line_with_space_and_escape_kwargs():
    # load and parse external data file
    test_file_dir = Path(__file__).resolve().parent
    data_file = test_file_dir / "resources" / "train file.dat"

    model = Workspace(oaa=3, data=str(data_file), final_regressor="test model.vw")
    assert model.predict("| feature1:2.5") == 1
    del model

    model_file = Path("test model.vw")
    assert model_file.is_file()
    model_file.unlink()


def test_command_line_using_arg_list():
    # load and parse external data file
    test_file_dir = Path(__file__).resolve().parent
    data_file = test_file_dir / "resources" / "train file.dat"

    args = [
        "--oaa",
        "3",
        "--data",
        str(data_file),
        "--final_regressor",
        "test model2.vw",
    ]
    model = Workspace(arg_list=args)
    assert model.predict("| feature1:2.5") == 1
    del model

    model_file = Path("test model2.vw")
    assert model_file.is_file()
    model_file.unlink()


def test_command_line_with_double_space_in_str():
    # Test regression for double space in string breaking splitting
    model = Workspace(arg_str="--oaa 3 -q ::    ")
    del model


def test_keys_with_list_of_values():
    # No exception in creating and executing model with a key/list pair
    model = Workspace(quiet=True, q=["fa", "fb"])
    model.learn("1 | a b c")
    prediction = model.predict(" | a b c")
    assert isinstance(prediction, float)
    del model


def helper_parse(examples):
    model = Workspace(quiet=True, cb_adf=True)
    ex = model.parse(examples)
    assert len(ex) == 2
    model.learn(ex)
    model.finish_example(ex)
    model.finish()


def test_parse():
    helper_parse("| a:1 b:0.5\n0:0.1:0.75 | a:0.5 b:1 c:2")

    helper_parse(
        """| a:1 b:0.5
    0:0.1:0.75 | a:0.5 b:1 c:2"""
    )

    helper_parse(
        """
    | a:1 b:0.5
    0:0.1:0.75 | a:0.5 b:1 c:2
    """
    )

    helper_parse(["| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"])


def test_parse_2():
    model = Workspace(quiet=True, cb_adf=True)
    ex = model.parse("| a:1 b:0.5\n0:0.1:0.75 | a:0.5 b:1 c:2")
    assert len(ex) == 2
    model.learn(ex)
    model.finish_example(ex)
    model.finish()

    model = Workspace(quiet=True, cb_adf=True)
    ex = model.parse(["| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"])
    assert len(ex) == 2
    model.learn(ex)
    model.finish_example(ex)
    model.finish()


def test_learn_predict_multiline():
    model = Workspace(quiet=True, cb_adf=True)
    ex = model.parse(["| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"])
    assert model.predict(ex) == [0.0, 0.0]
    model.finish_example(ex)
    ex = ["| a", "| b"]
    model.learn(ex)
    assert model.predict(ex) == [0.0, 0.0]


def test_namespace_id():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    nm1 = vowpalwabbit.NamespaceId(ex, 0)
    nm2 = vowpalwabbit.NamespaceId(ex, 1)
    nm3 = vowpalwabbit.NamespaceId(ex, 2)
    assert nm1.id == 0
    assert nm1.ord_ns == 97
    assert nm1.ns == "a"
    assert nm2.id == 1
    assert nm2.ord_ns == 98
    assert nm2.ns == "b"
    assert nm3.id == 2
    assert nm3.ord_ns == 128
    assert nm3.ns == "\x80"  # Represents string of ord_ns


def test_example_namespace():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    ns_id = vowpalwabbit.NamespaceId(ex, 1)
    ex_nm = vowpalwabbit.ExampleNamespace(ex, ns_id, ns_hash=vw_ex.hash_space(ns_id.ns))
    assert isinstance(ex_nm.ex, vowpalwabbit.Example)
    assert isinstance(ex_nm.ns, vowpalwabbit.NamespaceId)
    assert ex_nm.ns_hash == 2514386435
    assert ex_nm.num_features_in() == 3
    assert ex_nm[2] == (11617, 1.0)  # represents (feature, value)
    iter_obj = ex_nm.iter_features()
    for i in range(ex_nm.num_features_in()):
        assert ex_nm[i] == next(iter_obj)
    assert ex_nm.pop_feature()
    ex_nm.push_features(ns_id, ["c", "d"])
    assert ex_nm.num_features_in() == 4


def test_SimpleLabel():
    sl = vowpalwabbit.SimpleLabel(2.0, weight=0.5)
    assert sl.label == 2.0
    assert sl.weight == 0.5
    assert sl.prediction == 0.0
    assert sl.initial == 0.0
    assert str(sl) == "2.0:0.5"


def test_SimpleLabel_example():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    sl2 = vowpalwabbit.SimpleLabel.from_example(ex)
    assert sl2.label == 1.0
    assert sl2.weight == 1.0
    assert sl2.prediction == 0.0
    assert sl2.initial == 0.0
    assert str(sl2) == "1.0"


def test_MulticlassLabel():
    ml = vowpalwabbit.MulticlassLabel(2, weight=0.2)
    assert ml.label == 2
    assert ml.weight == 0.2
    assert ml.prediction == 1
    assert str(ml) == "2:0.2"


def test_MulticlassLabel_example():
    n = 4
    model = vowpalwabbit.Workspace(loss_function="logistic", oaa=n, quiet=True)
    ex = model.example("1 | a b c d", 2)
    ml2 = vowpalwabbit.MulticlassLabel.from_example(ex)
    assert ml2.label == 1
    assert ml2.weight == 1.0
    assert ml2.prediction == 0
    assert str(ml2) == "1"


def test_example_namespace_id():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    ns = vowpalwabbit.NamespaceId(ex, 1)
    assert isinstance(ex.get_ns(1), vowpalwabbit.NamespaceId)
    assert isinstance(ex[2], vowpalwabbit.ExampleNamespace)
    assert ex.setup_done is True
    assert ex.num_features_in(ns) == 3


def test_example_learn():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    ex.learn()
    assert ex.setup_done is True
    ex.unsetup_example()  # unsetup an example as it is already setup
    assert ex.setup_done is False


def test_example_label():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    ex.set_label_string("1.0")
    assert isinstance(ex.get_label(), vowpalwabbit.SimpleLabel)


def test_example_features():
    vw_ex = Workspace(quiet=True)
    ex = vw_ex.example("1 |a two features |b more features here")
    ns = vowpalwabbit.NamespaceId(ex, 1)
    assert ex.get_feature_id(ns, "a") == 127530
    ex.push_hashed_feature(ns, 1122)
    ex.push_features("x", [("c", 1.0), "d"])
    ex.push_feature(ns, 11000)
    assert ex.num_features_in("x") == 2
    assert ex.sum_feat_sq(ns) == 5.0
    ns2 = vowpalwabbit.NamespaceId(ex, 2)
    ex.push_namespace(ns2)
    assert ex.pop_namespace()


def test_example_features_dict():
    vw = Workspace(quiet=True)
    ex = vw.example(
        {"a": {"two": 1, "features": 1.0}, "b": {"more": 1, "features": 1, 5: 1.5}}
    )
    fs = list(ex.iter_features())

    assert (ex.get_feature_id("a", "two"), 1) in fs
    assert (ex.get_feature_id("a", "features"), 1) in fs
    assert (ex.get_feature_id("b", "more"), 1) in fs
    assert (ex.get_feature_id("b", "features"), 1) in fs
    assert (5, 1.5) in fs


def test_example_features_dict_long_long_index():
    vw = Workspace(quiet=True)
    ex = vw.example({"a": {2**40: 2}})
    fs = list(ex.iter_features())

    assert (2**40, 2) in fs


def test_get_weight_name():
    model = Workspace(quiet=True)
    model.learn("1 | a a b c |ns x")
    assert model.get_weight_from_name("a") != 0.0
    assert model.get_weight_from_name("b") != 0.0
    assert model.get_weight_from_name("b") == model.get_weight_from_name("c")
    assert model.get_weight_from_name("a") != model.get_weight_from_name("b")
    assert model.get_weight_from_name("x") == 0.0
    assert model.get_weight_from_name("x", "ns") != 0.0
    assert model.get_weight_from_name("x", "ns") == model.get_weight_from_name("b")


def test_runparser_cmd_string():
    vw = vowpalwabbit.Workspace("--data ./test/train-sets/rcv1_small.dat")
    assert vw.parser_ran == True, "vw should set parser_ran to true if --data present"
    vw.finish()


def test_runparser_cmd_string_short():
    vw = vowpalwabbit.Workspace("-d ./test/train-sets/rcv1_small.dat")
    assert vw.parser_ran == True, "vw should set parser_ran to true if --data present"
    vw.finish()


def test_not_runparser_cmd_string():
    vw = vowpalwabbit.Workspace("")
    assert vw.parser_ran == False, "vw should set parser_ran to false"
    vw.finish()


def check_error_raises(type, argument):
    """
    This function is used to check whether the exception is raised or not.

    Parameter
    ---------

    type: Type of Error raised
    argument: lambda function with no parameters.

    Example:
    >>> ex = ["|a", "|b"]
    >>> vw = vowpalwabbit.Workspace(quiet=True)
    >>> check_error_raises(TypeError, lambda: vw.learn(ex))

    """
    with pytest.raises(type) as error:
        argument()


def test_dsjson():
    vw = vowpalwabbit.Workspace("--cb_explore_adf --epsilon 0.2 --dsjson")

    ex_l_str = '{"_label_cost":-1.0,"_label_probability":0.5,"_label_Action":1,"_labelIndex":0,"o":[{"v":1.0,"EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","ActionTaken":false}],"Timestamp":"2020-11-15T17:09:31.8350000Z","Version":"1","EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","a":[1,2],"c":{ "GUser":{"id":"person5","major":"engineering","hobby":"hiking","favorite_character":"spock"}, "_multi": [ { "TAction":{"topic":"SkiConditions-VT"} }, { "TAction":{"topic":"HerbGarden"} } ] },"p":[0.5,0.5],"VWState":{"m":"N/A"}}\n'
    ex_l = vw.parse(ex_l_str)
    vw.learn(ex_l)
    pred = ex_l[0].get_action_scores()
    expected = [0.5, 0.5]
    assert len(pred) == len(expected)
    for a, b in zip(pred, expected):
        assert isclose(a, b)
    vw.finish_example(ex_l)

    ex_p = '{"_label_cost":-1.0,"_label_probability":0.5,"_label_Action":1,"_labelIndex":0,"o":[{"v":1.0,"EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","ActionTaken":false}],"Timestamp":"2020-11-15T17:09:31.8350000Z","Version":"1","EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","a":[1,2],"c":{ "GUser":{"id":"person5","major":"engineering","hobby":"hiking","favorite_character":"spock"}, "_multi": [ { "TAction":{"topic":"SkiConditions-VT"} }, { "TAction":{"topic":"HerbGarden"} } ] },"p":[0.5,0.5],"VWState":{"m":"N/A"}}\n'
    pred = vw.predict(ex_p)
    expected = [0.9, 0.1]
    assert len(pred) == len(expected)
    for a, b in zip(pred, expected):
        assert isclose(a, b)


def test_dsjson_with_metrics():
    vw = vowpalwabbit.Workspace(
        "--extra_metrics metrics.json --cb_explore_adf --epsilon 0.2 --dsjson"
    )

    ex_l_str = '{"_label_cost":-0.9,"_label_probability":0.5,"_label_Action":1,"_labelIndex":0,"o":[{"v":1.0,"EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","ActionTaken":false}],"Timestamp":"2020-11-15T17:09:31.8350000Z","Version":"1","EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","a":[1,2],"c":{ "GUser":{"id":"person5","major":"engineering","hobby":"hiking","favorite_character":"spock"}, "_multi": [ { "TAction":{"topic":"SkiConditions-VT"} }, { "TAction":{"topic":"HerbGarden"} } ] },"p":[0.5,0.5],"VWState":{"m":"N/A"}}\n'
    ex_l = vw.parse(ex_l_str)
    vw.learn(ex_l)
    pred = ex_l[0].get_action_scores()
    expected = [0.5, 0.5]
    assert len(pred) == len(expected)
    for a, b in zip(pred, expected):
        assert isclose(a, b)
    vw.finish_example(ex_l)

    ex_p = '{"_label_cost":-1.0,"_label_probability":0.5,"_label_Action":1,"_labelIndex":0,"o":[{"v":1.0,"EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","ActionTaken":false}],"Timestamp":"2020-11-15T17:09:31.8350000Z","Version":"1","EventId":"38cbf24f-70b2-4c76-aa0c-970d0c8d388e","a":[1,2],"c":{ "GUser":{"id":"person5","major":"engineering","hobby":"hiking","favorite_character":"spock"}, "_multi": [ { "TAction":{"topic":"SkiConditions-VT"} }, { "TAction":{"topic":"HerbGarden"} } ] },"p":[0.5,0.5],"VWState":{"m":"N/A"}}\n'
    pred = vw.predict(ex_p)
    expected = [0.9, 0.1]
    assert len(pred) == len(expected)
    for a, b in zip(pred, expected):
        assert isclose(a, b)

    learner_metric_dict = vw.get_learner_metrics()
    assert len(vw.get_learner_metrics()) == 17

    assert learner_metric_dict["total_predict_calls"] == 2
    assert learner_metric_dict["total_learn_calls"] == 1
    assert learner_metric_dict["cbea_labeled_ex"] == 1
    assert learner_metric_dict["cbea_predict_in_learn"] == 0
    assert learner_metric_dict["cbea_label_first_action"] == 1
    assert learner_metric_dict["cbea_label_not_first"] == 0
    assert pytest.approx(learner_metric_dict["cbea_sum_cost"]) == -0.9
    assert pytest.approx(learner_metric_dict["cbea_sum_cost_baseline"]) == -0.9
    assert learner_metric_dict["cbea_non_zero_cost"] == 1
    assert pytest.approx(learner_metric_dict["cbea_avg_feat_per_event"]) == 24
    assert pytest.approx(learner_metric_dict["cbea_avg_actions_per_event"]) == 2
    assert pytest.approx(learner_metric_dict["cbea_avg_ns_per_event"]) == 16
    assert pytest.approx(learner_metric_dict["cbea_avg_feat_per_action"]) == 12
    assert pytest.approx(learner_metric_dict["cbea_avg_ns_per_action"]) == 8
    assert learner_metric_dict["cbea_min_actions"] == 2
    assert learner_metric_dict["cbea_max_actions"] == 2
    assert learner_metric_dict["sfm_count_learn_example_with_shared"] == 1


def test_constructor_exception_is_safe():
    try:
        vw = vowpalwabbit.Workspace("--invalid_option")
    except:
        pass


def test_deceprecated_labels():
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        vowpalwabbit.pyvw.abstract_label()
        vowpalwabbit.pyvw.simple_label()
        vowpalwabbit.pyvw.multiclass_label()
        vowpalwabbit.pyvw.multiclass_probabilities_label()
        vowpalwabbit.pyvw.cost_sensitive_label()
        vowpalwabbit.pyvw.cbandits_label()


def test_random_weights_seed():
    # TODO: why do we need min_prediction and max_prediction?
    shared_args = "--random_weights --quiet --min_prediction -50 --max_prediction 50"

    a = Workspace(f"--random_seed 1 {shared_args}")
    b = Workspace(f"--random_seed 2 {shared_args}")

    dummy_ex_str = " | foo=bar"
    assert a.predict(dummy_ex_str) != b.predict(dummy_ex_str)


def test_merge_models():
    model1 = vowpalwabbit.Workspace(quiet=True)
    model1.learn("1 | foo")
    model1.learn("1 | foo")
    model2 = vowpalwabbit.Workspace(quiet=True)
    model2.learn("1 | bar")
    model2.learn("1 | bar")
    model2.learn("1 | bar")

    merged_model = vowpalwabbit.merge_models(None, [model1, model2])
    assert (
        merged_model.get_weighted_examples()
        == model1.get_weighted_examples() + model2.get_weighted_examples()
    )
    assert model1.get_weight_from_name("foo") != 0
    assert model1.get_weight_from_name("bar") == 0
    assert merged_model.get_weight_from_name("foo") != 0
    assert model2.get_weight_from_name("foo") == 0
    assert model2.get_weight_from_name("bar") != 0
    assert merged_model.get_weight_from_name("bar") != 0


def test_merge_models_with_base():
    model_base = vowpalwabbit.Workspace(quiet=True)
    model_base.learn("1 | foobar")
    model_base.learn("1 | foobar")
    model_base.learn("1 | foobar")
    model_base.save("test_merge_models_with_base.model")

    model1 = vowpalwabbit.Workspace(
        quiet=True,
        preserve_performance_counters=True,
        initial_regressor="test_merge_models_with_base.model",
    )
    model1.learn("1 | foo")
    model1.learn("1 | foo")
    model2 = vowpalwabbit.Workspace(
        quiet=True,
        preserve_performance_counters=True,
        initial_regressor="test_merge_models_with_base.model",
    )
    model2.learn("1 | bar")
    model2.learn("1 | bar")
    model2.learn("1 | bar")

    merged_model = vowpalwabbit.merge_models(model_base, [model1, model2])
    assert merged_model.get_weighted_examples() == (
        model_base.get_weighted_examples()
        + (model1.get_weighted_examples() - model_base.get_weighted_examples())
        + (model2.get_weighted_examples() - model_base.get_weighted_examples())
    )

    assert model_base.get_weight_from_name("foobar") != 0
    assert model1.get_weight_from_name("foobar") != 0
    assert model2.get_weight_from_name("foobar") != 0
    assert merged_model.get_weight_from_name("foobar") != 0

    assert model1.get_weight_from_name("foo") != 0
    assert model1.get_weight_from_name("bar") == 0
    assert merged_model.get_weight_from_name("foo") != 0
    assert model2.get_weight_from_name("foo") == 0
    assert model2.get_weight_from_name("bar") != 0
    assert merged_model.get_weight_from_name("bar") != 0
