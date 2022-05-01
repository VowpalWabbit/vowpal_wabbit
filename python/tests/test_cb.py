import pandas as pd
from os import path
import vowpalwabbit

import unittest
import platform
import math
import re


def helper_get_test_dir():
    curr_path = path.dirname(path.realpath(__file__))
    return path.join(path.dirname(path.dirname(curr_path)), "test")


def helper_get_data():
    train_data = [
        {
            "action": 1,
            "cost": 2,
            "probability": 0.4,
            "feature1": "a",
            "feature2": "c",
            "feature3": "",
        },
        {
            "action": 3,
            "cost": 0,
            "probability": 0.2,
            "feature1": "b",
            "feature2": "d",
            "feature3": "",
        },
        {
            "action": 4,
            "cost": 1,
            "probability": 0.5,
            "feature1": "a",
            "feature2": "b",
            "feature3": "",
        },
        {
            "action": 2,
            "cost": 1,
            "probability": 0.3,
            "feature1": "a",
            "feature2": "b",
            "feature3": "c",
        },
        {
            "action": 3,
            "cost": 1,
            "probability": 0.7,
            "feature1": "a",
            "feature2": "d",
            "feature3": "",
        },
    ]

    train_df = pd.DataFrame(train_data)

    train_df["index"] = range(1, len(train_df) + 1)
    train_df = train_df.set_index("index")

    test_data = [
        {"feature1": "b", "feature2": "c", "feature3": ""},
        {"feature1": "a", "feature2": "", "feature3": "b"},
        {"feature1": "b", "feature2": "b", "feature3": ""},
        {"feature1": "a", "feature2": "", "feature3": "b"},
    ]

    test_df = pd.DataFrame(test_data)

    # Add index to data frame
    test_df["index"] = range(1, len(test_df) + 1)
    test_df = test_df.set_index("index")

    return train_df, test_df


def test_getting_started_example_cb():
    return helper_getting_started_example("--cb")


def test_getting_started_example_legacy_cb():
    return helper_getting_started_example("--cb_force_legacy --cb")


# Returns true if they are close enough to be considered equal.
def are_floats_equal(float_one_str: str, float_two_str: str, epsilon: float) -> bool:
    float_one = float(float_one_str)
    float_two = float(float_two_str)

    # Special case handle these two as they will not be equal when checking absolute difference.
    # But for the purposes of comparing the diff they are equal.
    if math.isinf(float_one) and math.isinf(float_two):
        return True
    if math.isnan(float_one) and math.isnan(float_two):
        return True

    delta = abs(float_one - float_two)
    if delta < epsilon:
        return True

    # Large number comparison code migrated from Perl RunTests

    # We have a 'big enough' difference, but this difference
    # may still not be meaningful in all contexts. Big numbers should be compared by ratio rather than
    # by difference

    # Must ensure we can divide (avoid div-by-0)
    # If numbers are so small (close to zero),
    # ($delta > $Epsilon) suffices for deciding that
    # the numbers are meaningfully different
    if abs(float_two) <= 1.0:
        return False

    # Now we can safely divide (since abs($word2) > 0) and determine the ratio difference from 1.0
    ratio_delta = abs(float_one / float_two - 1.0)
    return ratio_delta < epsilon


def is_float(value: str) -> bool:
    try:
        float(value)
        return True
    except ValueError:
        return False


def is_line_different(output_line: str, ref_line: str, epsilon: float) -> bool:
    output_tokens = re.split("[ \t:,@]+", output_line)
    ref_tokens = re.split("[ \t:,@]+", ref_line)

    if len(output_tokens) != len(ref_tokens):
        return True

    for output_token, ref_token in zip(output_tokens, ref_tokens):
        output_is_float = is_float(output_token)
        ref_is_float = is_float(ref_token)
        if output_is_float and ref_is_float:
            are_equal = are_floats_equal(output_token, ref_token, epsilon)
            if not are_equal:
                return True
        else:
            if output_token != ref_token:
                return True

    return False


@unittest.skipIf(
    platform.machine() == "aarch64", "skipping due to floating-point error on aarch64"
)
def helper_getting_started_example(which_cb):
    train_df, test_df = helper_get_data()

    vw = vowpalwabbit.Workspace(
        which_cb + " 4 --log_level off --cb_type mtr", enable_logging=True
    )

    for i in train_df.index:
        action = train_df.loc[i, "action"]
        cost = train_df.loc[i, "cost"]
        probability = train_df.loc[i, "probability"]
        feature1 = train_df.loc[i, "feature1"]
        feature2 = train_df.loc[i, "feature2"]
        feature3 = train_df.loc[i, "feature3"]

        learn_example = (
            str(action)
            + ":"
            + str(cost)
            + ":"
            + str(probability)
            + " | "
            + str(feature1)
            + " "
            + str(feature2)
            + " "
            + str(feature3)
        )
        vw.learn(learn_example)

    assert (
        vw.get_prediction_type() == vw.pMULTICLASS
    ), "prediction_type should be multiclass"

    for j in test_df.index:
        feature1 = test_df.loc[j, "feature1"]
        feature2 = test_df.loc[j, "feature2"]
        feature3 = test_df.loc[j, "feature3"]
        choice = vw.predict(
            "| " + str(feature1) + " " + str(feature2) + " " + str(feature3)
        )
        assert isinstance(choice, int), "choice should be int"
        assert choice == 3, "predicted action should be 3 instead of " + str(choice)

    # test that metrics is empty since "--extra_metrics filename" was not supplied
    assert len(vw.get_learner_metrics()) == 0

    vw.finish()

    output = vw.get_log()

    if which_cb.find("legacy") != -1:
        test_file = "test-sets/ref/python_test_cb_legacy.stderr"
    else:
        test_file = "test-sets/ref/python_test_cb.stderr"

    print("Output received:")
    print("----------------")
    print("\n".join(output))
    print("----------------")

    with open(path.join(helper_get_test_dir(), test_file), "r") as file:
        expected = file.readlines()
        for expected_line, output_line in zip(expected, output):
            output_line = output_line.replace("...", "").strip()
            expected_line = expected_line.replace("...", "").strip()
            assert not is_line_different(output_line, expected_line, 0.001)


def test_getting_started_example_with():
    train_df, test_df = helper_get_data()

    # with syntax calls into vw.finish() automatically.
    # you actually want to use 'with vowpalwabbit.Workspace("--cb 4") as vw:'
    # but we need to assert on vw.finished for test purposes
    vw = vowpalwabbit.Workspace("--cb 4")
    with vw as vw:
        for i in train_df.index:
            action = train_df.loc[i, "action"]
            cost = train_df.loc[i, "cost"]
            probability = train_df.loc[i, "probability"]
            feature1 = train_df.loc[i, "feature1"]
            feature2 = train_df.loc[i, "feature2"]
            feature3 = train_df.loc[i, "feature3"]

            learn_example = (
                str(action)
                + ":"
                + str(cost)
                + ":"
                + str(probability)
                + " | "
                + str(feature1)
                + " "
                + str(feature2)
                + " "
                + str(feature3)
            )
            vw.learn(learn_example)

        assert (
            vw.get_prediction_type() == vw.pMULTICLASS
        ), "prediction_type should be multiclass"

        for j in test_df.index:
            feature1 = test_df.loc[j, "feature1"]
            feature2 = test_df.loc[j, "feature2"]
            feature3 = test_df.loc[j, "feature3"]
            choice = vw.predict(
                "| " + str(feature1) + " " + str(feature2) + " " + str(feature3)
            )
            assert isinstance(choice, int), "choice should be int"
            assert choice == 3, "predicted action should be 3"

    assert vw.finished == True, "with syntax should finish() vw instance"
