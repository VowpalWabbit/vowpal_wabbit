
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
    assert first_line == "1 | x:2"


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


def test_empty_rename_feature():
    df = pd.DataFrame({"a":[1]})
    first_line = DFtoVW(df=df, features=Feature("a", rename_feature="")).convert_df()[0]
    assert  first_line == "| :1"


def test_mixed_type_features():
    df = pd.DataFrame({"y": [1], "x1": ["a"], "x2": [2]})
    conv = DFtoVW(label=SimpleLabel("y"),
                  features=[Feature(value=colname) for colname in ["x1", "x2"]],
                  df=df)
    first_line = conv.convert_df()[0]
    assert first_line == "1 | x1=a x2:2"


def test_as_type_in_features():
    df = pd.DataFrame({"y":[1], "a": [2], "b": [3], "c": ["4"]})
    features = [Feature("a", as_type="categorical"),
                Feature("b"),
                Feature("c", as_type="numerical")]
    conv = DFtoVW(label=SimpleLabel("y"), features=features,
                  df=df)
    first_line = conv.convert_df()[0]
    assert first_line == "1 | a=2 b:3 c:4"


def test_dirty_colname_feature():
    df = pd.DataFrame({
            "target :": [1],
            " my first feature:": ["x"],
            "white space at the end ": [2]
            })
    features = [Feature(colname)
                for colname in [" my first feature:", "white space at the end "]]
    conv = DFtoVW(label=SimpleLabel("target :"), features=features, df=df)
    first_line = conv.convert_df()[0]
    assert first_line == "1 | my_first_feature=x white_space_at_the_end:2"


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
    return lines_list
    assert lines_list == ['1 |FirstNameSpace a:2 |DoubleIt:2 b=x1 c:36.4',
                          '-1 |FirstNameSpace a:3 |DoubleIt:2 b=x2 c:47.8']


def test_without_target_multiple_features():
    df = pd.DataFrame({"a": [2], "b": [3]})
    conv = DFtoVW(df=df, features=[Feature(col) for col in ["a", "b"]])
    first_line = conv.convert_df()[0]
    assert first_line == "| a:2 b:3"


def test_multiclasslabel():
    df = pd.DataFrame({"a": [1], "b": [0.5], "c": [-3]})
    conv = DFtoVW(
        df=df, label=MulticlassLabel(label="a", weight="b"), features=Feature("c")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1 0.5 | c:-3"


def test_multilabel():
    df = pd.DataFrame({"y1": [1], "y2": [2], "x": [3]})
    conv = DFtoVW(
        df=df, label=MultiLabel(["y1", "y2"]), features=Feature("x")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "1,2 | x:3"


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


def test_non_default_index():
    df = pd.DataFrame({"y":[0], "x":[1]}, index=[1])
    conv = DFtoVW(
            df=df,
            label=SimpleLabel("y"),
            features=Feature("x")
    )
    first_line = conv.convert_df()[0]
    assert first_line == "0 | x:1"


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

    