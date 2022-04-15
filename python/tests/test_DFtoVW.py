import pytest
import pandas as pd

from vowpalwabbit.dftovw import (
    ContextualbanditLabel,
    DFtoVW,
    Feature,
    MulticlassLabel,
    MultiLabel,
    Namespace,
    SimpleLabel,
)


# Tests when constructor is used without label
def test_no_label():
    df = pd.DataFrame({"a": [1]})
    first_line = DFtoVW(df=df, features=Feature("a")).convert_df()[0]
    assert first_line == "| a:1"


def test_no_label_rename_feature():
    df = pd.DataFrame({"a": [1]})
    first_line = DFtoVW(df=df, features=Feature("a", rename_feature="")).convert_df()[0]
    assert first_line == "| :1"


def test_no_label_multiple_features():
    df = pd.DataFrame({"a": [2], "b": [3]})
    conv = DFtoVW(df=df, features=[Feature(col) for col in ["a", "b"]])
    first_line = conv.convert_df()[0]
    assert first_line == "| a:2 b:3"


# Exception tests for Namespace
def test_namespace_with_value_but_no_name():
    with pytest.raises(ValueError) as value_error:
        Namespace(features=Feature("a"), value=10)
    expected = "Namespace can't have a 'value' argument without a 'name' argument or an empty string 'name' argument"
    assert expected == str(value_error.value)


# Tests for SimpleLabel
def test_from_column_names_constructor():
    df = pd.DataFrame({"y": [1], "x": [2]})
    conv = DFtoVW.from_column_names(y="y", x=["x"], df=df)
    lines_list = conv.convert_df()
    first_line = lines_list[0]
    assert first_line == "1 | x:2"


def test_from_column_names_no_label_constructor():
    df = pd.DataFrame({"x": [2]})
    conv = DFtoVW.from_column_names(x=["x"], df=df)
    lines_list = conv.convert_df()
    first_line = lines_list[0]
    assert first_line == "| x:2"


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


def test_multiple_lines():
    df = pd.DataFrame({"y": [1, -1], "x": [1, 2]})
    conv = DFtoVW(label=SimpleLabel("y"), features=Feature(value="x"), df=df)
    lines_list = conv.convert_df()
    assert lines_list == ["1 | x:1", "-1 | x:2"]


def test_mixed_type_features():
    df = pd.DataFrame({"y": [1], "x1": ["a"], "x2": [2]})
    conv = DFtoVW(
        label=SimpleLabel("y"),
        features=[Feature(value=colname) for colname in ["x1", "x2"]],
        df=df,
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 | x1=a x2:2"


def test_as_type_in_features():
    df = pd.DataFrame({"y": [1], "a": [2], "b": [3], "c": ["4"]})
    features = [
        Feature("a", as_type="categorical"),
        Feature("b"),
        Feature("c", as_type="numerical"),
    ]
    conv = DFtoVW(label=SimpleLabel("y"), features=features, df=df)
    first_line = conv.convert_df()[0]
    assert first_line == "1 | a=2 b:3 c:4"


def test_dirty_colname_feature():
    df = pd.DataFrame(
        {"target :": [1], " my first feature:": ["x"], "white space at the end ": [2]}
    )
    features = [
        Feature(colname)
        for colname in [" my first feature:", "white space at the end "]
    ]
    conv = DFtoVW(label=SimpleLabel("target :"), features=features, df=df)
    first_line = conv.convert_df()[0]
    assert first_line == "1 | my_first_feature=x white_space_at_the_end:2"


def test_feature_with_nan():
    df = pd.DataFrame({"y": [-1, 1, 1], "x1": [1, 2, None], "x2": [3, None, 2]})
    conv = DFtoVW(
        df=df, features=[Feature("x1"), Feature("x2")], label=SimpleLabel("y")
    )
    lines = conv.convert_df()
    assert lines == ["-1 | x1:1.0 x2:3.0", "1 | x1:2.0 ", "1 |  x2:2.0"]


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
    assert first_line == "1 |FirstNameSpace a:2 |DoubleIt:2 b:3"


def test_multiple_named_namespaces_multiple_features_multiple_lines():
    df = pd.DataFrame({"y": [1, -1], "a": [2, 3], "b": ["x1", "x2"], "c": [36.4, 47.8]})
    ns1 = Namespace(name="FirstNameSpace", features=Feature("a"))
    ns2 = Namespace(name="DoubleIt", value=2, features=[Feature("b"), Feature("c")])
    label = SimpleLabel("y")
    conv = DFtoVW(df=df, label=label, namespaces=[ns1, ns2])
    lines_list = conv.convert_df()
    assert lines_list == [
        "1 |FirstNameSpace a:2 |DoubleIt:2 b=x1 c:36.4",
        "-1 |FirstNameSpace a:3 |DoubleIt:2 b=x2 c:47.8",
    ]


def test_multiple_lines_with_weight():
    df = pd.DataFrame({"y": [1, 2, -1], "w": [2.5, 1.2, 3.75], "x": ["a", "b", "c"]})
    conv = DFtoVW(
        df=df, label=SimpleLabel(label="y", weight="w"), features=Feature("x")
    )
    lines_list = conv.convert_df()
    assert lines_list == ["1 2.5 | x=a", "2 1.2 | x=b", "-1 3.75 | x=c"]


# Exception tests for SimpleLabel
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


def test_non_default_index():
    df = pd.DataFrame({"y": [0], "x": [1]}, index=[1])
    conv = DFtoVW(df=df, label=SimpleLabel("y"), features=Feature("x"))
    first_line = conv.convert_df()[0]
    assert first_line == "0 | x:1"


def test_non_numerical_error():
    df = pd.DataFrame({"y": ["a"], "x": ["featX"]})
    with pytest.raises(TypeError) as type_error:
        DFtoVW(df=df, label=SimpleLabel(label="y"), features=Feature("x"))
    expected = "In argument 'label' of 'SimpleLabel', column 'y' should be either of the following type(s): 'int', 'float'."
    assert expected == str(type_error.value)


def test_wrong_feature_type_error():
    df = pd.DataFrame({"y": [1], "x": [2]})
    with pytest.raises(TypeError) as type_error:
        DFtoVW(df=df, label=SimpleLabel("y"), features="x")
    expected = "Argument 'features' should be a Feature or a list of Feature."
    assert expected == str(type_error.value)


def test_wrong_weight_type_error():
    df = pd.DataFrame({"y": [1], "x": [2], "w": ["a"]})
    with pytest.raises(TypeError) as type_error:
        DFtoVW(df=df, label=SimpleLabel(label="y", weight="w"), features=Feature("x"))
    expected = "In argument 'weight' of 'SimpleLabel', column 'w' should be either of the following type(s): 'int', 'float'."
    assert expected == str(type_error.value)


# Tests for MulticlassLabel
def test_multiclasslabel():
    df = pd.DataFrame({"a": [1], "b": [0.5], "c": [-3]})
    conv = DFtoVW(
        df=df, label=MulticlassLabel(label="a", weight="b"), features=Feature("c")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 0.5 | c:-3"


# Exception tests for MulticlassLabel
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


# Tests for MultiLabel
def test_multilabel():
    df = pd.DataFrame({"y1": [1], "y2": [2], "x": [3]})
    conv = DFtoVW(df=df, label=MultiLabel(["y1", "y2"]), features=Feature("x"))
    first_line = conv.convert_df()[0]
    assert first_line == "1,2 | x:3"


def test_multilabel_with_listlabel_builder():
    df = pd.DataFrame({"y1": [1], "y2": [2], "x": [3]})
    conv = DFtoVW(
        df=df, label=[MultiLabel("y1"), MultiLabel("y2")], features=Feature("x")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1,2 | x:3"


def test_multilabel_list_of_len_1():
    df = pd.DataFrame({"y": [1], "x": [2]})
    conv1 = DFtoVW(df=df, label=MultiLabel(["y"]), features=Feature("x"))
    conv2 = DFtoVW(df=df, label=MultiLabel("y"), features=Feature("x"))
    assert conv1.convert_df()[0] == conv2.convert_df()[0]


# Exception tests for MultiLabel
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


# Tests for ContextualbanditLabel
def test_contextualbanditlabel_one_label():
    df = pd.DataFrame({"a": [1], "c": [-0.5], "p": [0.1], "x": [1]})
    conv = DFtoVW(
        df=df, label=ContextualbanditLabel("a", "c", "p"), features=Feature("x")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1:-0.5:0.1 | x:1"


def test_contextualbanditlabel_multiple_label():
    df = pd.DataFrame(
        {
            "a1": [1],
            "c1": [-0.5],
            "p1": [0.1],
            "a2": [2],
            "c2": [-1.5],
            "p2": [0.6],
            "x": [1],
        }
    )
    conv = DFtoVW(
        df=df,
        label=[
            ContextualbanditLabel("a1", "c1", "p1"),
            ContextualbanditLabel("a2", "c2", "p2"),
        ],
        features=Feature("x"),
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1:-0.5:0.1 2:-1.5:0.6 | x:1"


# Exception tests for ContextualbanditLabel
def test_contextualbanditlabel_over_one_proba_error():
    df = pd.DataFrame({"a": [1], "c": [-0.5], "p": [1.1], "x": [1]})
    with pytest.raises(ValueError) as value_error:
        DFtoVW(
            df=df,
            label=ContextualbanditLabel("a", "c", "p"),
            features=Feature("x"),
        )
    expected = "In argument 'probability' of 'ContextualbanditLabel', column 'p' must be >= 0 and <= 1."
    assert expected == str(value_error.value)


def test_contextualbanditlabel_negative_proba_error():
    df = pd.DataFrame({"a": [1], "c": [-0.5], "p": [-0.1], "x": [1]})
    with pytest.raises(ValueError) as value_error:
        DFtoVW(
            df=df,
            label=ContextualbanditLabel("a", "c", "p"),
            features=Feature("x"),
        )
    expected = "In argument 'probability' of 'ContextualbanditLabel', column 'p' must be >= 0 and <= 1."
    assert expected == str(value_error.value)


def test_contextualbanditlabel_non_float_proba_error():
    df = pd.DataFrame({"a": [1], "c": [-0.5], "p": [1], "x": [1]})
    with pytest.raises(TypeError) as value_error:
        DFtoVW(
            df=df,
            label=ContextualbanditLabel("a", "c", "p"),
            features=Feature("x"),
        )
    expected = "In argument 'probability' of 'ContextualbanditLabel', column 'p' should be either of the following type(s): 'float'."
    assert expected == str(value_error.value)


def test_contextualbanditlabel_non_positive_action():
    df = pd.DataFrame({"a": [0], "c": [-0.5], "p": [0.5], "x": [1]})
    with pytest.raises(ValueError) as value_error:
        DFtoVW(
            df=df,
            label=ContextualbanditLabel("a", "c", "p"),
            features=Feature("x"),
        )
    expected = (
        "In argument 'action' of 'ContextualbanditLabel', column 'a' must be >= 1."
    )
    assert expected == str(value_error.value)


# Exception tests for _ListLabel
def test_listlabel_mixed_label_error():
    with pytest.raises(TypeError) as type_error:
        df = pd.DataFrame({"a": [1], "c": [-0.5], "p": [0.1], "b": [2], "x": [1]})
        conv = DFtoVW(
            df=df,
            label=[ContextualbanditLabel("a", "c", "p"), MultiLabel("b")],
            features=Feature("x"),
        )
    expected = "The list passed in 'label' has mixed label types."
    assert expected == str(type_error.value)


def test_listlabel_not_allowed_label_error():
    with pytest.raises(TypeError) as type_error:
        df = pd.DataFrame({"a": [1], "b": [2], "x": [1]})
        conv = DFtoVW(
            df=df,
            label=[SimpleLabel("a"), SimpleLabel("b")],
            features=Feature("x"),
        )
    expected = "The only labels that can be used with list are 'ContextualbanditLabel', 'MultiLabel'."
    assert expected == str(type_error.value)
