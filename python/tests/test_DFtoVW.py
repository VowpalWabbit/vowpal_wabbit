
import pytest
import pandas as pd

from vowpalwabbit.DFtoVW import (
    DFtoVW,
    SimpleLabel,
    MulticlassLabel,
    MultiLabel,
    Feature,
    Namespace,
)

def test_from_colnames_constructor():
    df = pd.DataFrame({"y": [1], "x": [2]})
    conv = DFtoVW.from_colnames(y="y", x=["x"], df=df)
    lines_list = conv.convert_df()
    first_line = lines_list[0]
    assert first_line == "1 | 2"


def test_feature_column_renaming_and_tag():
    df = pd.DataFrame({"idx": ["id_1"], "y": [1], "x": [2]})
    conv = DFtoVW(
        label=SimpleLabel("y"),
        tag="idx",
        features=Feature(value="x", rename_feature="col_x"),
        df=df,
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 id_1| col_x:2"


def test_feature_value_with_empty_name():
    df = pd.DataFrame({"idx": ["id_1"], "y": [1], "x": [2]})
    conv = DFtoVW(
        label=SimpleLabel("y"),
        tag="idx",
        features=Feature(value="x", rename_feature=""),
        df=df,
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 id_1| :2"


def test_multiple_lines():
    df = pd.DataFrame({"y": [1, -1], "x": [1, 2]})
    conv = DFtoVW(label=SimpleLabel("y"), features=Feature(value="x"), df=df,)
    lines_list = conv.convert_df()
    assert lines_list == ["1 | 1", "-1 | 2"]


def test_multiple_named_namespaces():
    df = pd.DataFrame({"y": [1], "a": [2], "b": [3]})
    conv = DFtoVW(
        df=df,
        label=SimpleLabel("y"),
        namespaces=[
            Namespace(name="FirstNameSpace", features=Feature("a")),
            Namespace(name="DoubleIt", value=2, features=Feature("b")),
        ],
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 |FirstNameSpace 2 |DoubleIt:2 3"


def test_without_target_multiple_features():
    df = pd.DataFrame({"a": [2], "b": [3]})
    conv = DFtoVW(df=df, features=[Feature(col) for col in ["a", "b"]])
    first_line = conv.convert_df()[0]
    assert first_line == "| 2 3"


def test_multiclasslabel():
    df = pd.DataFrame({"a": [1], "b": [0.5], "c": ["x"]})
    conv = DFtoVW(
        df=df, label=MulticlassLabel(label="a", weight="b"), features=Feature("c")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 0.5 | x"


def test_multilabel():
    df = pd.DataFrame({"y1": [1], "y2": [2], "x": [3]})
    conv = DFtoVW(
        df=df, label=MultiLabel(["y1", "y2"]), features=Feature("x")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1,2 | 3"


def test_multilabel_list_of_len_1():
    df = pd.DataFrame({"y": [1], "x": [2]})
    conv1 = DFtoVW(
        df=df, label=MultiLabel(["y"]), features=Feature("x")
    )
    conv2 = DFtoVW(
        df=df, label=MultiLabel("y"), features=Feature("x")
    )
    assert conv1.convert_df()[0] == conv2.convert_df()[0]


def test_absent_col_error():
    with pytest.raises(ValueError) as value_error:
        df = pd.DataFrame({"a": [1]})
        DFtoVW(
            df=df,
            label=SimpleLabel("a"),
            features=[Feature(col) for col in ["a", "c", "d"]],
        )
    expected = "In 'Feature': column(s) 'c', 'd' not found in dataframe."
    assert expected == str(value_error.value)


def test_non_numerical_simplelabel_error():
    df = pd.DataFrame({"y": ["a"], "x": ["featX"]})
    with pytest.raises(TypeError) as type_error:
        DFtoVW(df=df, label=SimpleLabel(label="y"), features=Feature("x"))
    expected = "In argument 'label' of 'SimpleLabel', column 'y' should be either of the following type(s): 'int', 'float', 'int64'."
    assert expected == str(type_error.value)


def test_wrong_feature_type_error():
    df = pd.DataFrame({"y": [1], "x": [2]})
    with pytest.raises(TypeError) as type_error:
        DFtoVW(df=df, label=SimpleLabel("y"), features="x")
    expected = "Argument 'features' should be a Feature or a list of Feature."
    assert expected == str(type_error.value)


def test_multiclasslabel_non_positive_label_error():
    df = pd.DataFrame({"a": [0], "b": [0.5], "c": ["x"]})
    with pytest.raises(ValueError) as value_error:
        DFtoVW(
            df=df,
            label=MulticlassLabel(label="a", weight="b"),
            features=Feature("c"),
        )
    expected = "In argument 'label' of 'MulticlassLabel', column 'a' must be >= 1."
    assert expected == str(value_error.value)


def test_multiclasslabel_negative_weight_error():
    df = pd.DataFrame({"y": [1], "w": [-0.5], "x": [2]})
    with pytest.raises(ValueError) as value_error:
        DFtoVW(
            df=df,
            label=MulticlassLabel(label="y", weight="w"),
            features=Feature("x"),
        )
    expected = "In argument 'weight' of 'MulticlassLabel', column 'w' must be >= 0."
    assert expected == str(value_error.value)


def test_multilabel_non_positive_label_error():
    df = pd.DataFrame({"y": [0], "b": [1]})
    with pytest.raises(ValueError) as value_error:
        DFtoVW(
            df=df,
            label=MultiLabel(label="y"),
            features=Feature("b"),
        )
    expected = "In argument 'label' of 'MultiLabel', column 'y' must be >= 1."
    assert expected == str(value_error.value)