import warnings
import numpy as np
import pandas as pd
from typing import Any, Hashable, List, Optional, Tuple, Type, Union


class _Col:
    """_Col class. It refers to a column of a dataframe."""

    mapping_python_numpy = {
        int: np.integer,
        float: np.floating,
        object: np.dtype("O"),
        str: np.dtype("O"),
    }

    def __init__(
        self,
        colname: Hashable,
        expected_type: Tuple,
        min_value: Optional[Union[int, float]] = None,
        max_value: Optional[Union[int, float]] = None,
    ):
        """Initialize a _Col instance

        Args:
            colname:  The column name. (any hashable type (str/int/float/tuple/etc.))
            expected_type: The expected type of the column.
            min_value: The minimum value to which the column must be superior or equal to.
            max_value: The maximum value to which the column must be inferior or equal to.
        """
        self.colname = colname
        self.expected_type = expected_type
        self.min_value = min_value
        self.max_value = max_value

    @staticmethod
    def make_valid_name(name: str) -> str:
        """Returns a feature/namespace name that is compatible with VW (no ':' nor ' ').

        Args:
            name: The name that will be made valid.

        Returns:
            A valid VW feature name.
        """
        name = str(name)
        valid_name = name.replace(":", " ").strip().replace(" ", "_")

        if valid_name != name:
            warnings.warn(
                "Name '{name}' was not a valid feature/namespace name. It has been renamed '{valid_name}'".format(
                    name=name,
                    valid_name=valid_name,
                )
            )

        return valid_name

    def get_col(self, df: pd.DataFrame) -> pd.Series:
        """Returns the column defined in attribute 'colname' from the dataframe 'df'.

        Args:
            df: The dataframe from which to select the column.

        Raises:
            KeyError: If the column is not found in the dataframe.

        Returns:
            The column defined in attribute 'colname' from the dataframe 'df'.
        """
        try:
            out = df[self.colname]
        except KeyError:
            raise KeyError(
                "column '{colname}' not found in dataframe".format(colname=self.colname)
            )
        else:
            return out.fillna("").apply(str)

    def is_number(self, df: pd.DataFrame) -> bool:
        """Check if the column is of type number.

        Args:
            df: The dataframe from which to check the column's type.
        """
        col_type = df[self.colname].dtype
        return np.issubdtype(col_type, np.number)

    def check_col_type(self, df: pd.DataFrame) -> None:
        """Check if the type of the column is valid.

        Args:
            df: The dataframe from which to check the column.

        Raises:
            TypeError: If the type of the column is not valid.
        """
        expected_type = [
            self.mapping_python_numpy[exp_type] for exp_type in self.expected_type
        ]
        col_type = df[self.colname].dtype

        if not any(np.issubdtype(col_type, exp_type) for exp_type in expected_type):
            raise TypeError(
                "column '{colname}' should be either of the following type(s): {type_name}.".format(
                    colname=self.colname,
                    type_name=str([x.__name__ for x in self.expected_type])[1:-1],
                )
            )

    def check_col_value(self, df: pd.DataFrame) -> None:
        """Check if the value range of the column is valid.

        Args:
            df: The dataframe from which to check the column.

        Raises:
            ValueError: If the values of the column are not valid.
        """
        if self.min_value is not None and self.max_value is not None:
            col_value = df[self.colname]
            if not (
                (col_value >= self.min_value).all()
                and (col_value <= self.max_value).all()
            ):
                raise ValueError(
                    "column '{colname}' must be >= {min_value} and <= {max_value}.".format(
                        colname=self.colname,
                        min_value=self.min_value,
                        max_value=self.max_value,
                    )
                )
        elif self.min_value is not None:
            col_value = df[self.colname]
            if not (col_value >= self.min_value).all():
                raise ValueError(
                    "column '{colname}' must be >= {min_value}.".format(
                        colname=self.colname, min_value=self.min_value
                    )
                )
        else:
            pass


class _AttributeDescriptor(object):
    """This descriptor class add type and value checking informations to the _Col
    instance for future usage in the DFtoVW class. Indeed, the type and value checking
    can only be done once the dataframe is known (i.e in DFtoVW class). This descriptor
    class is used in the following managed class: SimpleLabel, MulticlassLabel, Feature, etc.
    """

    def __init__(
        self,
        attribute_name: str,
        expected_type: Tuple[Type, ...],
        min_value: Optional[Union[str, int, float]] = None,
        max_value: Optional[Union[str, int, float]] = None,
    ):
        """Initialize an _AttributeDescriptor instance

        Args:
            attribute_name: The name of the attribute.
            expected_type: The expected type of the attribute.
            min_value: The minimum value of the attribute.
            max_value: The maximum value of the attribute.
        Raises:
            TypeError: If one of the arguments passed is not of valid type.
        """
        if not isinstance(attribute_name, str):
            raise TypeError("Argument 'attribute_name' must be a string")
        self.attribute_name = attribute_name
        if not isinstance(expected_type, tuple):
            raise TypeError("Argument 'expected_type' must be a tuple")
        self.expected_type = expected_type
        self.min_value = min_value
        self.max_value = max_value

    def __set__(self, instance: Any, arg: str) -> None:
        """Implement set protocol to enforce type (and value range) checking
        for managed class such as SimpleLabel, MulticlassLabel, Feature, etc.

        Args:
            instance: The managed instance.
            arg: The argument to set.
        """
        # initialize empty set that register the column names
        if "columns" not in instance.__dict__:
            instance.__dict__["columns"] = set()

        if arg is None:
            instance.__dict__[self.attribute_name] = arg
        else:
            arg_is_list = isinstance(arg, list)
            colnames = arg if arg_is_list else [arg]
            instance.__dict__["columns"].update(colnames)
            instance.__dict__[self.attribute_name] = [
                _Col(
                    colname=col,
                    expected_type=self.expected_type,
                    min_value=self.min_value,
                    max_value=self.max_value,
                )
                for col in colnames
            ]

            if not arg_is_list:
                instance.__dict__[self.attribute_name] = instance.__dict__[
                    self.attribute_name
                ][0]


class SimpleLabel(object):
    """The simple label type for the constructor of DFtoVW."""

    label: Any = _AttributeDescriptor("label", expected_type=(int, float))
    """Simple label value column name"""
    weight: Any = _AttributeDescriptor("weight", expected_type=(int, float))
    """Simple label weight column name"""

    def __init__(self, label: Hashable, weight: Optional[Hashable] = None):
        """Initialize a SimpleLabel instance.

        Args:
            label: The column name with the label.
            weight: The column name with the weight.
        """
        self.label = label
        self.weight = weight

    def process(self, df: pd.DataFrame) -> pd.Series:
        """Returns the SimpleLabel string representation.

        Args:
            df: The dataframe from which to select the column.

        Returns:
            The SimpleLabel string representation.
        """
        out = self.label.get_col(df)
        if self.weight is not None:
            out += " " + self.weight.get_col(df)
        return out


class MulticlassLabel(object):
    """The multiclass label type for the constructor of DFtoVW."""

    label: Any = _AttributeDescriptor("label", expected_type=(int,), min_value=1)
    """Multiclass label value column name"""
    weight: Any = _AttributeDescriptor(
        "weight", expected_type=(int, float), min_value=0
    )
    """Multiclass label weight column name"""

    def __init__(self, label: Hashable, weight: Optional[Hashable] = None):
        """Initialize a MulticlassLabel instance.

        Args:
            label: The column name with the multi class label.
            weight: The column name with the (importance) weight of the multi class label.
        """
        self.label = label
        self.weight = weight

    def process(self, df: pd.DataFrame) -> pd.Series:
        """Returns the MulticlassLabel string representation.

        Args:
        df: The dataframe from which to select the column(s).

        Returns:
            The MulticlassLabel string representation.
        """
        out = self.label.get_col(df)
        if self.weight is not None:
            out += " " + self.weight.get_col(df)
        return out


class MultiLabel(object):
    """The multi labels type for the constructor of DFtoVW."""

    label: Any = _AttributeDescriptor("label", expected_type=(int,), min_value=1)
    """Multilabel label value column name"""

    def __init__(self, label: Union[Hashable, List[Hashable]]):
        """Initialize a MultiLabel instance.

        Args:
            label: The (list of) column name(s) of the multi label(s).
        """
        self.label = label

    def process(self, df: pd.DataFrame) -> pd.Series:
        """Returns the MultiLabel string representation.

        Args:
            df: The dataframe from which to select the column(s).

        Returns:
            The MultiLabel string representation.
        """
        labels = self.label if isinstance(self.label, list) else [self.label]
        for (i, label) in enumerate(labels):
            label_col = label.get_col(df)
            if i == 0:
                out = label_col
            else:
                out += "," + label_col
        return out


class ContextualbanditLabel(object):
    """The contextual bandit label type for the constructor of DFtoVW."""

    action: Any = _AttributeDescriptor("action", expected_type=(int,), min_value=1)
    """Contextual bandit label action column name"""

    cost: Any = _AttributeDescriptor("cost", expected_type=(float, int))
    """Contextual bandit label cost column name"""
    probability: Any = _AttributeDescriptor(
        "probability", expected_type=(float,), min_value=0, max_value=1
    )
    """Contextual bandit label probability column name"""

    def __init__(self, action: Hashable, cost: Hashable, probability: Hashable):
        """Initialize a ContextualbanditLabel instance.

        Args:
            action: The action taken where we observed the cost.
            cost: The cost observed for this action (lower is better)
            probability: The probability of the exploration policy to choose this action when collecting the data.
        """
        self.action = action
        self.cost = cost
        self.probability = probability

    def process(self, df: pd.DataFrame) -> pd.Series:
        """Returns the ContextualbanditLabel string representation.

        Args:
            df: The dataframe from which to select the column(s).

        Returns:
            The ContextualbanditLabel string representation.
        """
        out = (
            self.action.get_col(df)
            + ":"
            + self.cost.get_col(df)
            + ":"
            + self.probability.get_col(df)
        )

        return out


class Feature(object):
    """The feature type for the constructor of DFtoVW"""

    value: Any = _AttributeDescriptor("value", expected_type=(str, int, float))
    """Feature value column name"""

    def __init__(
        self,
        value: Hashable,
        rename_feature: Optional[str] = None,
        as_type: Optional[str] = None,
    ):
        """
        Initialize a Feature instance.

        Args:
            value: The column name with the value of the feature.
            rename_feature: The name to use instead of the default (which is the column name defined in the value argument).
            as_type: Enforce a specific type ('numerical' or 'categorical')
        """
        self.value = value
        self.name = _Col.make_valid_name(
            rename_feature if rename_feature is not None else self.value.colname
        )
        if as_type is not None and as_type not in ("numerical", "categorical"):
            raise ValueError(
                "Argument 'as_type' can either be 'numerical' or 'categorical'"
            )
        else:
            self.as_type = as_type

    def process(self, df: pd.DataFrame, ensure_valid_values=True) -> pd.Series:
        """Returns the Feature string representation.

        Args:
            df: The dataframe from which to select the column(s).

        Returns:
            The Feature string representation.
        """
        value_col = self.value.get_col(df)
        if self.as_type:
            sep = ":" if self.as_type == "numerical" else "="
        else:
            sep = ":" if self.value.is_number(df) else "="

        if ensure_valid_values:
            value_col = value_col.apply(_Col.make_valid_name)

        out = value_col.where(value_col == "", self.name + sep + value_col)
        return out


class _Tag(object):
    """A tag for the constructor of DFtoVW"""

    tag: Any = _AttributeDescriptor("tag", expected_type=(str, int, float))
    """Tag column name"""

    def __init__(self, tag: Hashable):
        """
        Initialize a Tag instance.

        Args:
            tag: The column name with the tag.
        """
        self.tag = tag

    def process(self, df: pd.DataFrame) -> pd.Series:
        """Returns the _Tag string representation.

        Args:
            df: The dataframe from which to select the column.

        Returns: The Tag string representation.
        """
        return self.tag.get_col(df)


class Namespace(object):
    """The namespace type for the constructor of DFtoVW.
    The Namespace is a container for Feature object(s), and thus must
    be composed of a Feature object or a list of Feature objects.
    """

    expected_type = dict(
        name=(str, int, float), value=(int, float), features=(Feature,)
    )

    def __init__(
        self,
        features: Union[Feature, List[Feature]],
        name: Optional[Union[str, int, float]] = None,
        value: Optional[Union[int, float]] = None,
    ):
        """Initialize a Namespace instance.

        Args:
            features: A (list of) Feature object(s) that form the namespace.
            name: The name of the namespace.
            value: A constant that specify the scaling factor for the features of this
                namespace.

        Examples:
            >>> from vowpalwabbit.dftovw import Namespace, Feature
            >>> ns_one_feature = Namespace(Feature("a"))
            >>> ns_multi_features = Namespace([Feature("a"), Feature("b")])
            >>> ns_one_feature_with_name = Namespace(Feature("a"), name="FirstNamespace")
            >>> ns_one_feature_with_name_and_value = Namespace(Feature("a"), name="FirstNamespace", value=2)
        """

        if (value is not None) and (name is None or name == ""):
            raise ValueError(
                "Namespace can't have a 'value' argument without a 'name' argument or an empty string 'name' argument"
            )
        self.name = _Col.make_valid_name(name) if name else None
        self.value = value
        self.features = (
            list(features) if isinstance(features, (list, set)) else [features]
        )
        self.check_attributes_type()

    def check_attributes_type(self):
        """Check if attributes are of valid type.

        Raises:
            TypeError: If one of the attribute is not valid.
        """
        for attribute_name in ["name", "value"]:
            attribute_value = getattr(self, attribute_name)
            if attribute_value is not None and not isinstance(
                attribute_value, self.expected_type[attribute_name]
            ):
                raise TypeError(
                    "In Namespace, argument '{attribute_name}' should be either of the following type(s): {types}".format(
                        attribute_name=attribute_name,
                        types=repr(
                            [x.__name__ for x in self.expected_type[attribute_name]]
                        )[1:-1],
                    )
                )

        valid_feature = all(
            [
                isinstance(feature, self.expected_type["features"])
                for feature in self.features
            ]
        )
        if not valid_feature:
            raise TypeError(
                "In Namespace, argument 'features' should be a Feature or a list of Feature."
            )

    def process(self) -> str:
        """Returns the Namespace string representation

        Returns:
            The Namespace string representation.
        """
        out = ["|"]
        if self.name is not None:
            out += str(self.name)
            if self.value is not None:
                out += [":", str(self.value)]

        return "".join(out)


class _ListLabel(object):
    """An helper class that handles a list of labels.

    The class is to be used in the `DFtoVW` class. It parses the list of labels
    that the user could provide in the `label` argument of `DFtoVW`. The class only accepts
    a pre-defined set of label classes (defined in the class attribute `available_labels`).

    Args:
        label_list: The list of labels that the user passed in the attribute label of DFtoVW.

    Raises:
        ValueError: If the list passed has mixed types or if the labels should not be used in a list.
    """

    available_labels = (ContextualbanditLabel, MultiLabel)
    sep_by_label = dict(ContextualbanditLabel=" ", MultiLabel=",")

    def __init__(self, label_list: List[Union[ContextualbanditLabel, MultiLabel]]):

        instance_classes = set(
            [type(label_instance).__name__ for label_instance in label_list]
        )
        if len(instance_classes) > 1:
            raise TypeError("The list passed in 'label' has mixed label types.")

        if not all(
            isinstance(label_instance, self.available_labels)
            for label_instance in label_list
        ):
            raise TypeError(
                "The only labels that can be used with list are {accepted}.".format(
                    accepted=repr([cls.__name__ for cls in self.available_labels])[1:-1]
                )
            )

        # Unpack columns
        columns = set()
        # Override the type to Any as labels have complex machinery that is not yet communicated via its type.
        label_list: List[Any] = label_list

        for label in label_list:
            columns.update(label.columns)
        self.columns = columns

        self.label_list = label_list

        # Unpack attributes that are of type _Col
        label_cols = []
        for label_instance in label_list:
            label_cols += [
                value
                for key, value in vars(label_instance).items()
                if isinstance(value, _Col)
            ]
        self.label_cols = label_cols

        self.sep = self.sep_by_label[list(instance_classes)[0]]

        # Dynamically change class_name
        _ListLabel.__name__ = "List[{typename}]".format(
            typename=type(label_list[0]).__name__
        )

    def __getitem__(self, idx):
        return self.label_list[idx]

    def __iter__(self):
        return iter(self.label_list)

    def __len__(self):
        return len(self.label_list)

    def process(self, df: pd.DataFrame) -> pd.Series:
        """Return the string representation of the labels of the underlying list, separated by a pre-defined character.

        Args:
            df: The dataframe from which to select the columns.

        Returns:
            The _ListLabel string representation.
        """
        for (i, label) in enumerate(self):
            if i == 0:
                out = label.process(df)
            else:
                out += self.sep + label.process(df)

        return out


class DFtoVW:
    """Convert a pandas DataFrame to a suitable VW format.
    Instances of this class are built with classes such as SimpleLabel,
    MulticlassLabel, Feature or Namespace.

    The class also provided a convenience constructor to initialize the class
    based on the target/features column names only.
    """

    def __init__(
        self,
        df: pd.DataFrame,
        features: Optional[Union[Feature, List[Feature]]] = None,
        namespaces: Optional[Union[Namespace, List[Namespace]]] = None,
        label: Optional[
            Union[
                SimpleLabel,
                MulticlassLabel,
                MultiLabel,
                ContextualbanditLabel,
                List[MultiLabel],
                List[ContextualbanditLabel],
            ]
        ] = None,
        tag: Optional[Hashable] = None,
    ):
        """Initialize a DFtoVW instance.

        Args:
            df: The dataframe to convert to VW input format.
            features: One or more Feature object(s).
            namespaces: One or more Namespace object(s), each of being composed of one or
                more Feature object(s).
            label: One or more label objects used to build the label string
            tag: The tag column name (used as identifiers for examples).

        Examples:
            >>> from vowpalwabbit.dftovw import DFtoVW, SimpleLabel, Feature, Namespace
            >>> import pandas as pd

            >>> df = pd.DataFrame({"y": [1], "a": [2], "b": [3], "c": [4]})
            >>> conv1 = DFtoVW(df=df,
            ...                label=SimpleLabel("y"),
            ...                features=Feature("a"))
            >>> conv1.convert_df()
            ['1 | a:2']

            >>> conv2 = DFtoVW(df=df,
            ...                label=SimpleLabel("y"),
            ...                features=[Feature(col) for col in ["a", "b"]])
            >>> conv2.convert_df()
            ['1 | a:2 b:3']

            >>> conv3 = DFtoVW(df=df,
            ...                label=SimpleLabel("y"),
            ...                namespaces=Namespace(
            ...                        name="DoubleIt", value=2,
            ...                        features=Feature(value="a", rename_feature="feat_a")))
            >>> conv3.convert_df()
            ['1 |DoubleIt:2 feat_a:2']

            >>> conv4 = DFtoVW(df=df,
            ...                label=SimpleLabel("y"),
            ...                namespaces=[Namespace(name="NS1", features=[Feature(col) for col in ["a", "c"]]),
            ...                            Namespace(name="NS2", features=Feature("b"))])
            >>> conv4.convert_df()
            ['1 |NS1 a:2 c:4 |NS2 b:3']
        """
        self.df = df
        self.n_rows = df.shape[0]
        if isinstance(label, list):
            self.label = _ListLabel(label)
        else:
            self.label = label
        self.tag = _Tag(tag) if tag else None

        if label is not None:
            self.check_label_type()

        if features is not None:
            self.check_features_type(features)

        self.set_namespaces(namespaces, features)
        self.check_namespaces_type()

        self.check_missing_columns_df()
        self.check_columns_type_and_values()

    @classmethod
    def from_colnames(
        cls,
        y: Union[Hashable, List[Hashable]],
        x: Union[Hashable, List[Hashable]],
        df: pd.DataFrame,
        label_type: str = "simple_label",
    ) -> "DFtoVW":
        """Build DFtoVW instance using column names only.

            .. deprecated:: 9.2.0
                Use :meth:`DFtoVW.from_column_names` instead.

        Args:
            y: (list of) any hashable type (str/int/float/tuple/etc.) representing a column name
                The column for the label.
            x: (list of) any hashable type (str/int/float/tuple/etc.) representing a column name
                The column(s) for the feature(s).
            df: The dataframe used.
            label_type: The type of the label. Available labels: 'simple_label', 'multiclass_label', 'multi_label'. (default: 'simple_label')

        Raises:
            TypeError: If argument label is not of valid type.
            ValueError: If argument label_type is not valid.

        Examples:
            >>> from vowpalwabbit.dftovw import DFtoVW
            >>> import pandas as pd
            >>> df = pd.DataFrame({"y": [1], "x": [2]})
            >>> conv = DFtoVW.from_colnames(y="y", x="x", df=df)
            >>> conv.convert_df()
            ['1 | x:2']

            >>> df2 = pd.DataFrame({"y": [1], "x1": [2], "x2": [3], "x3": [4]})
            >>> conv2 = DFtoVW.from_colnames(y="y", x=sorted(list(set(df2.columns) - set("y"))), df=df2)
            >>> conv2.convert_df()
            ['1 | x1:2 x2:3 x3:4']

        Returns:
            An initialized DFtoVW instance.
        """

        warnings.warn(
            "DFtoVW.from_colnames is deprecated. Use DFtoVW.from_column_names instead.",
            DeprecationWarning,
        )
        return cls.from_column_names(y=y, x=x, df=df, label_type=label_type)

    @classmethod
    def from_column_names(
        cls,
        *,
        y: Optional[Union[Hashable, List[Hashable]]] = None,
        x: Union[Hashable, List[Hashable]],
        df: pd.DataFrame,
        label_type: Optional[str] = "simple_label",
    ) -> "DFtoVW":
        """Build DFtoVW instance using column names only. Compared to :meth:`DFtoVW.from_colnames`, this method allows for y and label_type to be optional and args are named and cannot be positional.

        Args:
            y: (list of) any hashable type (str/int/float/tuple/etc.) representing a column name
                The column for the label. Optional.
            x: (list of) any hashable type (str/int/float/tuple/etc.) representing a column name
                The column(s) for the feature(s).
            df: The dataframe used.
            label_type: The type of the label. Available labels: 'simple_label', 'multiclass_label', 'multi_label'. (default: 'simple_label'). Optional.

        Raises:
            TypeError: If argument label is not of valid type.
            ValueError: If argument label_type is not valid.

        Examples:
            >>> from vowpalwabbit.dftovw import DFtoVW
            >>> import pandas as pd
            >>> df = pd.DataFrame({"y": [1], "x": [2]})
            >>> conv = DFtoVW.from_column_names(y="y", x="x", df=df)
            >>> conv.convert_df()
            ['1 | x:2']

            >>> df2 = pd.DataFrame({"y": [1], "x1": [2], "x2": [3], "x3": [4]})
            >>> conv2 = DFtoVW.from_column_names(y="y", x=sorted(list(set(df2.columns) - set("y"))), df=df2)
            >>> conv2.convert_df()
            ['1 | x1:2 x2:3 x3:4']

        Returns:
            An initialized DFtoVW instance.
        """

        dict_label_type = {
            "simple_label": SimpleLabel,
            "multiclass_label": MulticlassLabel,
            "multi_label": MultiLabel,
        }

        if label_type not in dict_label_type:
            raise ValueError(
                "'label_type' should be either of the following string: {label_types}".format(
                    label_types=repr(list(dict_label_type.keys()))[1:-1]
                )
            )

        x = x if isinstance(x, list) else [x]
        namespaces = Namespace(features=[Feature(value=colname) for colname in x])
        if not y:
            return cls(namespaces=namespaces, label=None, df=df)
        else:
            y = y if isinstance(y, list) else [y]
            if label_type not in ["multi_label"]:
                if len(y) > 1:
                    raise TypeError(
                        "When label_type is 'simple_label' or 'multiclass', argument 'y' should be a string (or any hashable type) "
                        + "or a list of exactly one string (or any hashable type)."
                    )
                else:
                    y = y[0]
            label = dict_label_type[label_type](y)
            return cls(namespaces=namespaces, label=label, df=df)

    def check_features_type(self, features: Union[Feature, List[Feature]]):
        """Check if the features argument is of type Feature.

        Args:
            features: (list of) Feature. The features argument to check.

        Raises:
            TypeError: If the features is not a Feature of a list of Feature.
        """
        if isinstance(features, list):
            valid_feature = all([isinstance(feature, Feature) for feature in features])
        else:
            valid_feature = isinstance(features, Feature)
        if not valid_feature:
            raise TypeError(
                "Argument 'features' should be a Feature or a list of Feature."
            )

    def set_namespaces(
        self,
        namespaces: Optional[Union[Namespace, List[Namespace]]],
        features: Optional[Union[Feature, List[Feature]]],
    ):
        """Set namespaces attributes. Only one of namespaces or features should be passed when being called.

        Args:
            namespaces: The namespaces argument.
            features: The features argument.

        Raises:
            ValueError: If argument 'features' or 'namespaces' are not valid.
        """
        if (features is None) and (namespaces is None):
            raise ValueError("Missing 'features' or 'namespace' argument")
        if (features is not None) and (namespaces is not None):
            raise ValueError(
                "Arguments supplied for both 'features' and 'namespaces', only one of the these arguments should be supplied."
            )

        if features is not None:
            namespaces = Namespace(features=features)

        namespaces = (
            list(namespaces) if isinstance(namespaces, (list, set)) else [namespaces]
        )

        self.namespaces = namespaces

    def check_label_type(self):
        """Check label type.

        Raises:
            TypeError: If label is not of type SimpleLabel, MulticlassLabel, Multilabel, ContextualbanditLabel.
        """
        available_labels = (
            SimpleLabel,
            MulticlassLabel,
            MultiLabel,
            ContextualbanditLabel,
        )

        label = self.label[0] if isinstance(self.label, _ListLabel) else self.label

        if not isinstance(label, available_labels):
            raise TypeError(
                "Argument 'label' should be either of the following type: {label_types}.".format(
                    label_types=repr([x.__name__ for x in available_labels])[1:-1]
                )
            )

    def check_namespaces_type(self):
        """Check if namespaces arguments are of type Namespace.

        Raises:
            TypeError: If namespaces are not of type Namespace or list of Namespace.
        """
        wrong_type_namespaces = [
            not isinstance(namespace, Namespace) for namespace in self.namespaces
        ]
        if any(wrong_type_namespaces):
            raise TypeError(
                "Argument 'namespaces' should be a Namespace or a list of Namespace."
            )

    def check_missing_columns_df(self):
        """Check if the columns are in the dataframe."""
        missing_cols = {}
        df_colnames = set(self.df.columns)

        # pytype: disable=attribute-error
        try:
            label_columns = self.label.columns
        except AttributeError:
            pass
        else:
            type_label = type(self.label).__name__
            missing_cols[type_label] = label_columns - df_colnames

        try:
            tag_columns = self.tag.columns
        except AttributeError:
            pass
        else:
            missing_cols["tag"] = tag_columns - df_colnames
        # pytype: enable=attribute-error

        all_features = [
            feature for namespace in self.namespaces for feature in namespace.features
        ]
        missing_features_cols = set()
        for feature in all_features:
            missing_features_cols.update(feature.columns - df_colnames)

        missing_cols["Feature"] = missing_features_cols

        missing_cols = {
            key: sorted(list(value))
            for (key, value) in missing_cols.items()
            if len(value) > 0
        }
        if missing_cols:
            self.raise_missing_col_error(missing_cols)

    def raise_missing_col_error(self, missing_cols_dict):
        """Raises error if some columns are missing.

        Raises:
            ValueError: If one or more columns are not in the dataframe.
        """
        error_msg = ""
        for attribute_name, missing_cols in missing_cols_dict.items():
            missing_cols_str = repr(missing_cols)[1:-1]
            if len(error_msg) > 0:
                error_msg += " "
            error_msg += (
                "In '{attribute}': column(s) {colnames} not found in dataframe.".format(
                    attribute=attribute_name,
                    colnames=missing_cols_str,
                )
            )
        raise ValueError(error_msg)

    def check_columns_type_and_values(self):
        """Check columns type and values range."""
        for instance in [self.tag, self.label]:
            if instance is not None:
                self.check_instance_columns(instance)

        for namespace in self.namespaces:
            for feature in namespace.features:
                if instance is not None:
                    self.check_instance_columns(feature)

    def check_instance_columns(self, instance):
        """Check the columns type and values of a given instance.
        The method iterate through the attributes and look for _Col type
        attribute. Once found, the method use the _Col methods to check the
        type and the value range of the column. Also, the instance type in
        which the errors occur are prepend to the error message to be more
        explicit about where the error occurs in the formula.

        Raises:
            TypeError: If a column is not of valid type.
            ValueError: If a column values are not in the valid range.
        """
        class_name = type(instance).__name__

        for attribute_name, attribute_value in vars(instance).items():

            if not isinstance(attribute_value, list):
                attribute_value = [attribute_value]

            if not all([isinstance(x, _Col) for x in attribute_value]):
                continue

            # Testing columns type
            try:
                [x.check_col_type(self.df) for x in attribute_value]
            except TypeError as type_error:
                type_error_msg = (
                    "In argument '{attribute}' of '{class_name}', {error}".format(
                        class_name=class_name,
                        attribute=attribute_name,
                        error=str(type_error),
                    )
                )
                raise TypeError(type_error_msg)

            # Testing columns value range
            try:
                [x.check_col_value(self.df) for x in attribute_value]
            except ValueError as value_error:
                value_error_msg = (
                    "In argument '{attribute}' of '{class_name}', {error}".format(
                        class_name=class_name,
                        attribute=attribute_name,
                        error=str(value_error),
                    )
                )
                raise ValueError(value_error_msg)

    def convert_df(self) -> List[str]:
        """Main method that converts the dataframe to the VW format.

        Returns:
            The list of parsed lines in VW format.
        """
        self.out = self.empty_col()

        if not all([x is None for x in [self.label, self.tag]]):
            self.out += self.process_label_and_tag()

        for (i, namespace) in enumerate(self.namespaces):
            to_add = namespace.process() + self.process_features(namespace.features)
            self.out += (to_add + " ") if (i < len(self.namespaces) - 1) else to_add

        return self.out.to_list()

    def empty_col(self) -> pd.Series:
        """Create an empty string column.

        Returns:
            A column of empty string with as much rows as the input dataframe.
        """
        return pd.Series(data=[""] * self.n_rows, index=self.df.index)

    def process_label_and_tag(self) -> pd.Series:
        """Process the label and tag into a unique column.

        Returns:
            A column where each row is the processed label and tag.
        """
        out = self.empty_col()
        if self.label is not None:
            out += self.label.process(self.df) + " "
        if self.tag is not None:
            out += self.tag.process(self.df)
        return out

    def process_features(self, features: List[Feature]) -> pd.Series:
        """Process the features (of a namespace) into a unique column.

        Args:
            features: The list of Feature objects.

        Returns:
            The column of the processed features.
        """
        out = self.empty_col()
        for feature in features:
            out += " " + feature.process(self.df)
        return out
