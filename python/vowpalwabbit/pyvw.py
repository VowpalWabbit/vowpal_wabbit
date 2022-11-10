# -*- coding: utf-8 -*-
"""Python binding for pylibvw class"""

from __future__ import division
from typing import Any, Callable, Dict, Iterator, List, Optional, Tuple, Type, Union
import pylibvw
import warnings
import inspect
from pathlib import Path

from enum import IntEnum


# From stack overflow: https://stackoverflow.com/a/52087847/2214524
class _DeprecatedClassMeta(type):
    def __new__(cls, name, bases, classdict, *args, **kwargs):
        alias = classdict.get("_DeprecatedClassMeta__alias")

        if alias is not None:

            def new(cls, *args, **kwargs):
                alias = getattr(cls, "_DeprecatedClassMeta__alias")

                if alias is not None:
                    warnings.warn(
                        "{} has been renamed to {}, the alias will be "
                        "removed in the future".format(cls.__name__, alias.__name__),
                        DeprecationWarning,
                        stacklevel=2,
                    )

                return alias(*args, **kwargs)

            classdict["__new__"] = new
            classdict["_DeprecatedClassMeta__alias"] = alias

        fixed_bases = []

        for b in bases:
            alias = getattr(b, "_DeprecatedClassMeta__alias", None)

            if alias is not None:
                warnings.warn(
                    "{} has been renamed to {}, the alias will be "
                    "removed in the future".format(b.__name__, alias.__name__),
                    DeprecationWarning,
                    stacklevel=2,
                )

            # Avoid duplicate base classes.
            b = alias or b
            if b not in fixed_bases:
                fixed_bases.append(b)

        fixed_bases = tuple(fixed_bases)

        return super().__new__(cls, name, fixed_bases, classdict, *args, **kwargs)

    def __instancecheck__(cls, instance):
        return any(
            cls.__subclasscheck__(c) for c in {type(instance), instance.__class__}
        )

    def __subclasscheck__(cls, subclass):
        if subclass is cls:
            return True
        else:
            return issubclass(subclass, getattr(cls, "_DeprecatedClassMeta__alias"))


class LabelType(IntEnum):
    SIMPLE = pylibvw.vw.lSimple
    MULTICLASS = pylibvw.vw.lMulticlass
    COST_SENSITIVE = pylibvw.vw.lCostSensitive
    CONTEXTUAL_BANDIT = pylibvw.vw.lContextualBandit
    CONDITIONAL_CONTEXTUAL_BANDIT = pylibvw.vw.lConditionalContextualBandit
    SLATES = pylibvw.vw.lSlates
    CONTINUOUS = pylibvw.vw.lContinuous
    CONTEXTUAL_BANDIT_EVAL = pylibvw.vw.lContextualBanditEval
    MULTILABEL = pylibvw.vw.lMultilabel


class PredictionType(IntEnum):
    SCALAR = pylibvw.vw.pSCALAR
    SCALARS = pylibvw.vw.pSCALARS
    ACTION_SCORES = pylibvw.vw.pACTION_SCORES
    ACTION_PROBS = pylibvw.vw.pACTION_PROBS
    MULTICLASS = pylibvw.vw.pMULTICLASS
    MULTILABELS = pylibvw.vw.pMULTILABELS
    PROB = pylibvw.vw.pPROB
    MULTICLASSPROBS = pylibvw.vw.pMULTICLASSPROBS
    DECISION_SCORES = pylibvw.vw.pDECISION_SCORES
    ACTION_PDF_VALUE = pylibvw.vw.pACTION_PDF_VALUE
    PDF = pylibvw.vw.pPDF
    ACTIVE_MULTICLASS = pylibvw.vw.pACTIVE_MULTICLASS
    NOPRED = pylibvw.vw.pNOPRED


# baked in con py boost https://wiki.python.org/moin/boost.python/FAQ#The_constructors_of_some_classes_I_am_trying_to_wrap_are_private_because_instances_must_be_created_by_using_a_factory._Is_it_possible_to_wrap_such_classes.3F
class VWOption:
    def __init__(
        self,
        name,
        help_str,
        short_name,
        keep,
        necessary,
        allow_override,
        value,
        value_supplied,
        default_value,
        default_value_supplied,
        experimental,
    ):
        self._name = name
        self._help_str = help_str
        self._short_name = short_name
        self._keep = keep
        self._necessary = necessary
        self._allow_override = allow_override
        self._value = value
        self._value_supplied = value_supplied
        self._default_value = default_value
        self._default_value_supplied = default_value_supplied
        self._experimental = experimental

    @property
    def name(self):
        return self._name

    @property
    def help_str(self):
        return self._help_str

    @property
    def short_name(self):
        return self._short_name

    @property
    def keep(self):
        return self._keep

    @property
    def necessary(self):
        return self._necessary

    @property
    def allow_override(self):
        return self._allow_override

    @property
    def value_supplied(self):
        return self._value_supplied

    @property
    def default_value(self):
        return self._default_value

    @property
    def default_value_supplied(self):
        return self._default_value_supplied

    @property
    def experimental(self):
        return self._experimental

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, val):
        self._value_supplied = True
        self._value = val

    def is_flag(self):
        return type(self._default_value) == bool or (
            self.value_supplied and type(self.value) == bool
        )

    def __str__(self):
        if self.value_supplied:
            if self.is_flag():
                return "--{}".format(self.name)
            else:
                if isinstance(self.value, list):
                    return " ".join(map(lambda x: f"--{self.name}={x}", self.value))
                else:
                    return "--{}={}".format(self.name, self.value)
        else:
            return ""


class SearchTask:
    """Search task class"""

    # Declare types for optional variables to prevent pytype attribute-error
    _setup: Optional[Callable]
    _takedown: Optional[Callable]

    def __init__(self, vw, sch, num_actions):
        """
        Args:
            vw: The workspace instance
            sch: The search instance
            num_actions (int): The number of actions with the object can be initialized with

        See Also:
            :py:class:`~vowpalwabbit.Workspace`
        """

        self.vw = vw
        self.sch = sch
        self.bogus_example = [self.vw.example("1 | x")]

    def __del__(self):
        self.vw.finish_example(self.bogus_example)

    def _run(self, your_own_input_example):
        pass

    def _call_vw(
        self, my_example, isTest, useOracle=False
    ):  # run_fn, setup_fn, takedown_fn, isTest):
        self._output = None
        self.bogus_example[0].set_test_only(isTest)

        def run():
            self._output = self._run(my_example)

        setup = None
        takedown = None
        if callable(getattr(self, "_setup", None)):
            setup = lambda: self._setup(my_example)
        if callable(getattr(self, "_takedown", None)):
            takedown = lambda: self._takedown(my_example)
        self.sch.set_structured_predict_hook(run, setup, takedown)
        self.sch.set_force_oracle(useOracle)
        self.vw.learn(
            self.bogus_example
        )  # this will cause our ._run hook to get called

    def learn(self, data_iterator):
        """Train search task by providing an iterator of examples.

        Args:
            data_iterator: iterable objects
                Consists of examples to be learned
        """
        for my_example in data_iterator.__iter__():
            self._call_vw(my_example, isTest=False)

    def example(
        self,
        initStringOrDict: Optional[
            Union[
                str,
                Dict[str, List[Union[Tuple[Union[str, int], float], Union[str, int]]]],
                Any,
                pylibvw.example,
            ]
        ] = None,
        labelType: Optional[Union[int, LabelType]] = None,
    ) -> "Example":
        """Create an example initStringOrDict can specify example as VW
        formatted string, or a dictionary labelType can specify the desire
        label type

        Args:
            initStringOrDict: See description in :py:meth:`~vowpalwabbit.Example.__init__` for valid values
            labelType : See description in :py:meth:`~vowpalwabbit.Example.__init__` for valid values

        Returns:
            Constructed example object
        """
        if self.sch.predict_needs_example():
            return self.vw.example(initStringOrDict, labelType)
        else:
            return self.vw.example(None, labelType)

    def predict(self, my_example, useOracle: bool = False):
        """Predict on the example

        Args:
            my_example (Example):
                example used for prediction
            useOracle : Use oracle for this prediction

        Returns:
            int: prediction of this example
        """
        self._call_vw(my_example, isTest=True, useOracle=useOracle)
        return self._output


def get_prediction(ec, prediction_type: Union[int, PredictionType]):
    """Get specified type of prediction from example

    .. deprecated:: 9.0.0
        Use :py:meth:`~vowpalwabbit.Example.get_prediction` instead

    Args:
        ec : Example
        prediction_type : either the integer value of the :py:obj:`~vowpalwabbit.PredictionType` enum or the enum itself

    Examples:
        >>> from vowpalwabbit import Workspace, PredictionType, pyvw
        >>> vw = Workspace(quiet=True)
        >>> ex = vw.example('1 |a two features |b more features here')
        >>> pyvw.get_prediction(ex, PredictionType.SCALAR)
        0.0

    Returns:
        Prediction according to parameter prediction_type
    """
    warnings.warn(
        "get_prediction is deprecated, use Example.get_prediction instead.",
        DeprecationWarning,
    )
    return ec.get_prediction(prediction_type)


def get_label_class_from_enum(
    label_type: LabelType,
) -> Type[
    Union[
        "SimpleLabel",
        "MulticlassLabel",
        "CostSensitiveLabel",
        "CBLabel",
        "CCBLabel",
        "SlatesLabel",
        "CBContinuousLabel",
        "CBEvalLabel",
        "MultilabelLabel",
    ]
]:
    switch_label_type = {
        LabelType.SIMPLE: SimpleLabel,
        LabelType.MULTICLASS: MulticlassLabel,
        LabelType.COST_SENSITIVE: CostSensitiveLabel,
        LabelType.CONTEXTUAL_BANDIT: CBLabel,
        LabelType.CONDITIONAL_CONTEXTUAL_BANDIT: CCBLabel,
        LabelType.SLATES: SlatesLabel,
        LabelType.CONTINUOUS: CBContinuousLabel,
        LabelType.CONTEXTUAL_BANDIT_EVAL: CBEvalLabel,
        LabelType.MULTILABEL: MultilabelLabel,
    }

    return switch_label_type[label_type]


def get_all_vw_options():
    temp = Workspace("--dry_run")
    config = temp.get_config(filtered_enabled_reductions_only=False)
    temp.finish()
    return config


class _log_forward:
    def __init__(self):
        self.current_message = ""
        self.messages = []

    def log(self, msg):
        if "\n" in msg:
            components = msg.split("\n")
            self.current_message += components[0]
            self.messages.append(self.current_message)
            for component in components[1:-1]:
                self.messages.append(component)
            self.current_message = components[-1]
        else:
            self.current_message += msg


def _build_command_line(
    arg_str: Optional[str] = None, arg_list: Optional[List[str]] = None, **kw
):
    def format_key(key: str) -> str:
        prefix = "-" if len(key) == 1 else "--"
        return f"{prefix}{key}"

    def format_input(
        key: str,
        val: Union[int, float, str, bool, List[int], List[float], List[str]],
    ) -> List[str]:
        res = [format_key(key)]
        if isinstance(val, list):
            # if a list is passed as a parameter value - create a key for
            # each list element
            for v in val:
                if isinstance(v, bool):
                    raise ValueError(
                        f"List of bool values not supported. Argument: {key}"
                    )
                res.append(str(v))
        elif isinstance(val, bool):
            if val == False:
                return []
        elif isinstance(val, (int, float, str)):
            res.append(str(val))
        return res

    merged_arg_list = []
    if arg_str is not None:
        if not isinstance(arg_str, str):
            raise TypeError("arg_str must be a string")
        # Maintain old behavior of space split strings
        merged_arg_list.extend(arg_str.split())

    if arg_list is not None:
        if len(arg_list) > 0 and not isinstance(arg_list[0], str):
            raise TypeError("arg_list must be a list of strings")
        merged_arg_list.extend(arg_list)

    for key, val in kw.items():
        merged_arg_list.extend(format_input(key, val))
    return merged_arg_list


class Workspace(pylibvw.vw):
    """Workspace exposes most of the library functionality. It wraps the native code. The Workspace Python class should always be used instead of the binding glue class."""

    _log_wrapper: Optional[pylibvw.vw_log]
    parser_ran: bool
    init: bool
    finished: bool
    _log_fwd: Optional[_log_forward]

    def __init__(
        self,
        arg_str: Optional[str] = None,
        enable_logging: bool = False,
        arg_list: Optional[List[str]] = None,
        **kw,
    ):
        """Initialize the Workspace object. arg_str, arg_list and the kwargs will be merged together. Duplicates will result in duplicate values in the command line.

        Args:
            arg_str: The command line arguments to initialize VW with, for example "--audit". This list is naively split by spaces. To control the splitting behavior please pass a list of strings to arg_list instead.
            enable_logging: Enable captured logging. By default is False. This must be True to be able to call :py:meth:`~vowpalwabbit.Workspace.get_log`
            arg_list: List of tokens that comprise the command line.
            **kw : Using key/value pairs for different options available. Using this append an option to the command line in the form of "--key value", or in the case of a bool "--key" if true.

        Examples:
            >>> from vowpalwabbit import Workspace
            >>> vw1 = Workspace('--audit')
            >>> vw2 = Workspace(audit=True, b=24, k=True, c=True, l2=0.001)
            >>> vw3 = Workspace("--audit", b=26)
            >>> vw4 = Workspace(q=["ab", "ac"])
            >>> vw4 = Workspace(arg_list=["--audit", "--interactions", "ab"])
        """

        self._log_wrapper = None
        self.parser_ran = False
        self.init = False
        self.finished = False
        self._log_fwd = None

        if enable_logging:
            self._log_fwd = _log_forward()
            self._log_wrapper = pylibvw.vw_log(self._log_fwd)

        merged_arg_list = _build_command_line(arg_str, arg_list, **kw)
        if self._log_wrapper:
            super().__init__(merged_arg_list, self._log_wrapper)
        else:
            super().__init__(merged_arg_list)
        self.init = True

        # check to see if native parser needs to run
        ext_file_args = ["-d", "--data", "--passes"]
        run_parser = False
        for arg in merged_arg_list:
            for ext_file_arg in ext_file_args:
                if arg.startswith(ext_file_arg):
                    run_parser = True
                    break

        if run_parser:
            pylibvw.vw.run_parser(self)
            self.parser_ran = True

    def get_config(self, filtered_enabled_reductions_only=True):
        return self.get_options(VWOption, filtered_enabled_reductions_only)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.finish()

    def parse(
        self, str_ex, labelType: Optional[Union[int, LabelType]] = None
    ) -> Union["Example", List["Example"]]:
        """Returns a collection of examples for a multiline example learner or
        a single example for a single example learner.

        Args:
            str_ex : str/list of str
                string representing examples. If the string is multiline then each
                line is considered as an example. In case of list, each string
                element is considered as an example
            labelType : The direct integer value of the :py:obj:`~vowpalwabbit.LabelType` enum can be used or the enum directly. Supplying 0 or None means to use the default label type based on the setup VW learner.

        Examples:
            >>> from vowpalwabbit import Workspace
            >>> model = Workspace(quiet=True)
            >>> ex = model.parse("0:0.1:0.75 | a:0.5 b:1 c:2")
            >>> type(ex)
            <class 'vowpalwabbit.pyvw.Example'>
            >>> model = Workspace(quiet=True, cb_adf=True)
            >>> ex = model.parse(["| a:1 b:0.5", "0:0.1:0.75 | a:0.5 b:1 c:2"])
            >>> type(ex)
            <class 'list'>
            >>> len(ex) # Shows the multiline example is parsed
            2

        Returns:
            Either a single example or list of examples.
        """

        # check if already parsed
        if isinstance(str_ex, Example) and getattr(str_ex, "setup_done", None):
            return str_ex

        elif isinstance(str_ex, list):
            if all(
                [
                    isinstance(ex, Example) and getattr(ex, "setup_done", None)
                    for ex in str_ex
                ]
            ):
                str_ex: List[
                    Example
                ] = str_ex  # pytype: disable=annotation-type-mismatch
                return str_ex

        if not isinstance(str_ex, (list, str)):
            raise TypeError("Unsupported type. List or string object must be passed.")

        if isinstance(str_ex, list):
            str_ex = "\n".join(str_ex)

        str_ex = str_ex.replace("\r", "")
        str_ex = str_ex.strip()
        ec = self._parse(str_ex)
        ec = [Example(self, x, labelType) for x in ec]
        for ex in ec:
            ex.setup_done = True
        if not self._is_multiline():
            if len(ec) == 1:
                ec = ec[0]
            else:
                raise TypeError(
                    "expecting single line example, got multi_ex of len %i" % len(ec)
                )
        return ec

    def finish_example(self, ex: Union["Example", List["Example"]]) -> None:
        """Every example that is created with :py:meth:`~vowpalwabbit.Workspace.parse`, :py:meth:`~vowpalwabbit.Workspace.example`, or :py:class:`~vowpalwabbit.Example`, should be passed to this method when you are finished with them.

        This will return them to the Workspace instance to be reused and it will update internal statistics. If you care about statistics of used Examples then you should only use them once before passing them to finish.

        Args:
            ex: example or examples to be finished
        """

        if isinstance(ex, Example):
            if self._is_multiline():
                raise ValueError(
                    "Learner is multiline but single example was passed to "
                    "finish_example. Use the list of examples instead?"
                )
            if not ex.finished:
                pylibvw.vw._finish_example(self, ex)
                ex.finished = True
        elif isinstance(ex, list):
            if not self._is_multiline():
                raise ValueError(
                    "Learner is singleline but multi example was passed to "
                    "finish_example. Use a single example instead?"
                )
            if all(x.finished == False for x in ex):
                pylibvw.vw._finish_example_multi_ex(self, ex)
                for x in ex:
                    x.finished = True

    def num_weights(self) -> int:
        """Get length of weight vector."""
        return pylibvw.vw.num_weights(self)

    def get_weight(self, index, offset=0) -> float:
        """Get the weight at a particular position in the (learned) weight
        vector.

        Args:
            index (int): position in the learned  weight vector
            offset (int): By default is 0

        Returns:
            float: Weight at the given index
        """
        return pylibvw.vw.get_weight(self, index, offset)

    def get_weight_from_name(
        self, feature_name: str, namespace_name: str = " "
    ) -> float:
        """Get the weight based on the feature name and the namespace name.

        Args
            feature_name: The name of the feature
            namespace_name: The name of the namespace where the feature lives

        Returns:
            float: Weight for the given feature and namespace name
        """
        space_hash = self.hash_space(namespace_name)
        feat_hash = self.hash_feature(feature_name, space_hash)
        return self.get_weight(feat_hash)

    def get_label_type(self) -> LabelType:
        return LabelType(super()._get_label_type())

    def get_prediction_type(self) -> PredictionType:
        return PredictionType(super()._get_prediction_type())

    def learn(self, ec: Union["Example", List["Example"], str, List[str]]) -> None:
        """Perform an online update

        Args:
            ec: Examples on which the model gets updated. If using a single object the learner must be a single line learner. If using a list of objects the learner must be a multiline learner. If passing strings they are parsed using :py:meth:`~vowpalwabbit.Workspace.parse` before being learned from. If passing Example objects then they must be given to :py:meth:`~vowpalwabbit.Workspace.finish_example` at a later point.
        """
        # If a string was given, parse it before passing to learner.
        new_example = False
        if isinstance(ec, str):
            ec = self.parse(ec)
            new_example = True

        elif isinstance(ec, list):
            if not self._is_multiline():
                raise TypeError("Expecting a mutiline Learner.")
            if len(ec) == 0:
                raise ValueError("An empty list is invalid")
            if isinstance(ec[0], str):
                ec = self.parse(ec)
                new_example = True

        if not isinstance(ec, Example) and not (
            isinstance(ec, list) and isinstance(ec[0], Example)
        ):
            raise TypeError(
                "expecting string, example object, or list of example objects"
                " as ec argument for learn, got %s" % type(ec)
            )

        if isinstance(ec, Example):
            if not getattr(ec, "setup_done", None):
                ec.setup_example()
            pylibvw.vw.learn(self, ec)

        elif isinstance(ec, list):
            pylibvw.vw.learn_multi(self, ec)

        else:
            raise TypeError(
                "expecting string or example object as ec argument for learn,"
                " got %s" % type(ec)
            )

        if new_example:
            self.finish_example(ec)

    def predict(
        self,
        ec: Union["Example", List["Example"], str, List[str]],
        prediction_type: Optional[Union[int, PredictionType]] = None,
    ):
        """Make a prediction on the example

        Args:
            ec: Examples of which to get a prediction from. If using a single object the learner must be a single line learner. If using a list of objects the learner must be a multiline learner. If passing strings they are parsed using :py:meth:`~vowpalwabbit.Workspace.parse` before being predicted on. If passing Example objects then they must be given to :py:meth:`~vowpalwabbit.Workspace.finish_example` at a later point.
            prediction_type: If none, use the prediction type of the example object. This is usually what is wanted. To request a specific type a value can be supplied here.

        Returns:
            Prediction based on the given example
        """

        new_example = False
        if isinstance(ec, dict):
            ec = self.example(ec)
            ec.setup_done = True
            new_example = True

        # If a string was given, parse it before passing to learner.
        elif isinstance(ec, str):
            ec = self.parse(ec)
            new_example = True

        elif isinstance(ec, list):
            if not self._is_multiline():
                raise TypeError("Expecting a multiline Learner.")
            if len(ec) == 0:
                raise ValueError("An empty list is invalid")
            if isinstance(ec[0], str):
                ec = self.parse(ec)
                new_example = True

        if not isinstance(ec, Example) and not (
            isinstance(ec, list) and isinstance(ec[0], Example)
        ):
            raise TypeError(
                "expecting string, example object, or list of example objects"
                " as ec argument for predict, got %s" % type(ec)
            )

        if isinstance(ec, Example) and not getattr(ec, "setup_done", True):
            ec.setup_example()

        if isinstance(ec, Example):
            pylibvw.vw.predict(self, ec)
        else:
            pylibvw.vw.predict_multi(self, ec)

        if prediction_type is None:
            prediction_type = self.get_prediction_type()

        if isinstance(ec, Example):
            prediction = ec.get_prediction(prediction_type)
        else:
            prediction = ec[0].get_prediction(prediction_type)  # type: ignore

        if new_example:
            self.finish_example(ec)

        return prediction

    def save(self, filename: Union[str, Path]) -> None:
        """save model to disk"""
        pylibvw.vw.save(self, str(filename))

    def finish(self) -> None:
        """stop VW by calling finish (and, eg, write weights to disk)"""
        if not self.finished and self.init:
            pylibvw.vw.finish(self)
            self.init = False
            self.finished = True

    def get_log(self) -> List[str]:
        """Get all log messages produced. One line per item in the list. To get the complete log including run results, this should be called after :func:`~vowpalwabbit.vw.finish`

        Raises:
            Exception: Raises an exception if this function is called but the init function was called without setting enable_logging to True

        Returns:
            A list of strings, where each item is a line in the log
        """
        if self._log_fwd:
            return self._log_fwd.messages + [self._log_fwd.current_message]
        else:
            raise Exception("enable_logging set to false")

    def example(
        self,
        stringOrDict: Optional[
            Union[
                str,
                Dict[str, List[Union[Tuple[Union[str, int], float], Union[str, int]]]],
                Any,
                pylibvw.example,
            ]
        ] = None,
        labelType: Optional[Union[int, LabelType]] = None,
    ) -> "Example":
        """Helper function to create an example object associated with this Workspace instance.

        Args:
            stringOrDict: See description in :py:meth:`~vowpalwabbit.Example.__init__` for valid values
            labelType : See description in :py:meth:`~vowpalwabbit.Example.__init__` for valid values
        Returns:
            Constructed Example

        """
        return Example(self, stringOrDict, labelType)

    def __del__(self):
        self.finish()

    def init_search_task(self, search_task, task_data=None):
        sch = self.get_search_ptr()

        def predict(
            examples,
            my_tag,
            oracle,
            condition=None,
            allowed=None,
            learner_id=0,
        ):
            """The basic (via-reduction) prediction mechanism

            Args:

                examples : Example/list
                    can be a single example (interpreted as
                    non-LDF mode) or a list of examples (interpreted as
                    LDF mode).  it can also be a lambda function that
                    returns a single example or list of examples, and in
                    that list, each element can also be a lambda function
                    that returns an example. this is done for lazy
                    example construction (aka speed).

                my_tag : integer
                    should be an integer id, specifying this prediction

                oracle : label/list
                    can be a single label (or in LDF mode a single
                    array index in 'examples') or a list of such labels if
                    the oracle policy is indecisive; if it is None, then
                    the oracle doesn't care

                condition : Optional, by default is None
                    should be either: (1) a (tag,char) pair, indicating
                    to condition on the given tag with identifier from the char;
                    or (2) a (tag,len,char) triple, indicating to condition on
                    tag, tag-1, tag-2, ..., tag-len with identifiers char,
                    char+1, char+2, ..., char+len. or it can be a (heterogenous)
                    list of such things.

                allowed : optional, by default id None
                    can be None, in which case all actions are allowed;
                    or it can be list of valid actions (in LDF mode, this should
                    be None and you should encode the valid actions in 'examples')

                learner_id : integer
                    specifies the underlying learner id

            Returns:
                integer: a single prediction.

            """
            P = sch.get_predictor(my_tag)
            if sch.is_ldf():
                # we need to know how many actions there are, even if we don't
                # know their identities
                while hasattr(examples, "__call__"):
                    examples = examples()
                if not isinstance(examples, list):
                    raise TypeError(
                        "expected example _list_ in LDF mode for SearchTask.predict()"
                    )
                P.set_input_length(len(examples))
                if sch.predict_needs_example():
                    for n in range(len(examples)):
                        ec = examples[n]
                        while hasattr(ec, "__call__"):
                            ec = ec()  # unfold the lambdas
                        if not isinstance(ec, Example) and not isinstance(
                            ec, pylibvw.example
                        ):
                            raise TypeError(
                                "non-example in LDF example list in SearchTask.predict()"
                            )
                        if hasattr(ec, "setup_done") and not ec.setup_done:
                            ec.setup_example()
                        P.set_input_at(n, ec)
                else:
                    pass  # TODO: do we need to set the examples even though they're not used?
            else:
                if sch.predict_needs_example():
                    while hasattr(examples, "__call__"):
                        examples = examples()
                    if hasattr(examples, "setup_done") and not examples.setup_done:
                        examples.setup_example()
                    P.set_input(examples)
                else:
                    pass  # TODO: do we need to set the examples even though they're not used?

            # if (isinstance(examples, list) and all([isinstance(ex, example) or
            #       isinstance(ex, pylibvw.example) for ex in examples])) or \
            #    isinstance(examples, example) or isinstance(examples, pylibvw.example):
            #     if isinstance(examples, list): # LDF
            #         P.set_input_length(len(examples))
            #         for n in range(len(examples)):
            #             P.set_input_at(n, examples[n])
            #     else: # non-LDF
            #         P.set_input(examples)
            if oracle is None:
                pass
            elif isinstance(oracle, list):
                assert 0 not in oracle, "multiclass labels are from 1..., "
                "please do not use zero or bad things will happen!"
                if len(oracle) > 0:
                    P.set_oracles(oracle)
            elif isinstance(oracle, int):
                assert oracle > 0, "multiclass labels are from 1...,"
                " please do not use zero or bad things will happen!"
                P.set_oracle(oracle)
            else:
                raise TypeError("expecting oracle to be a list or an integer")

            if condition is not None:
                if not isinstance(condition, list):
                    condition = [condition]
                for c in condition:
                    if not isinstance(c, tuple):
                        raise TypeError(
                            "item " + str(c) + " in condition list is malformed"
                        )
                    if (
                        len(c) == 2
                        and isinstance(c[0], int)
                        and isinstance(c[1], str)
                        and len(c[1]) == 1
                    ):
                        P.add_condition(max(0, c[0]), c[1])
                    elif (
                        len(c) == 3
                        and isinstance(c[0], int)
                        and isinstance(c[1], int)
                        and isinstance(c[2], str)
                        and len(c[2]) == 1
                    ):
                        P.add_condition_range(max(0, c[0]), max(0, c[1]), c[2])
                    else:
                        raise TypeError(
                            "item " + str(c) + " in condition list malformed"
                        )

            if allowed is None:
                pass
            elif isinstance(allowed, list):
                assert 0 not in allowed, "multiclass labels are from 1...,"
                "please do not use zero or bad things will happen!"
                P.set_alloweds(allowed)
            else:
                raise TypeError("allowed argument wrong type")

            if learner_id != 0:
                P.set_learner_id(learner_id)

            p = P.predict()
            return p

        sch.predict = predict
        num_actions = sch.get_num_actions()
        return (
            search_task(self, sch, num_actions)
            if task_data is None
            else search_task(self, sch, num_actions, task_data)
        )


class NamespaceId:
    """The NamespaceId class is simply a wrapper to convert between
    hash spaces referred to by character (eg 'x') versus their index
    in a particular example. Mostly used internally, you shouldn't
    really need to touch this."""

    id: Optional[int]
    """Index into the list of example feature groups for this given namespace"""
    ns: str
    """Single character respresenting the namespace index"""
    ord_ns: int
    """Integer representation of the ns field"""

    def __init__(self, ex: "Example", id: Union[int, str]):
        """Given an example and an id, construct a NamespaceId.

        Args:
            ex: example used to create a namespace id
            id:
                - If int, uses that as an index into this Examples list of feature groups to get the namespace id character
                - If str, uses the first character as the namespace id character
        """
        if isinstance(id, int):  # you've specified a namespace by index
            if id < 0 or id >= ex.num_namespaces():
                raise Exception("namespace " + str(id) + " out of bounds")
            self.id = id
            self.ord_ns = ex.namespace(id)
            self.ns = chr(self.ord_ns)
        elif isinstance(id, str):  # you've specified a namespace by string
            if len(id) == 0:
                id = " "
            self.id = None  # we don't know and we don't want to do the linear search required to find it
            self.ns = id[0]
            self.ord_ns = ord(self.ns)
        else:
            raise Exception(
                "ns_to_characterord failed because id type is unknown: " + str(type(id))
            )


class ExampleNamespace:
    """The ExampleNamespace class is a helper class that allows you
    to extract namespaces from examples and operate at a namespace
    level rather than an example level. Mainly this is done to enable
    indexing like ex['x'][0] to get the 0th feature in namespace 'x'
    in example ex."""

    def __init__(self, ex: "Example", ns: NamespaceId, ns_hash: Optional[int] = None):
        """Construct an ExampleNamespace

        Args:
            ex: examples from which namespace is to be extracted
            ns: Target namespace
            ns_hash: The hash of the namespace
        """
        if not isinstance(ns, NamespaceId):
            raise TypeError("ns should an instance of NamespaceId.")
        self.ex = ex
        self.ns = ns
        self.ns_hash = ns_hash

    def num_features_in(self) -> int:
        """Return the total number of features in this namespace."""
        return self.ex.num_features_in(self.ns)

    def __getitem__(self, i) -> Tuple[int, float]:
        """Get the feature/value pair for the ith feature in this
        namespace."""
        f = self.ex.feature(self.ns, i)
        v = self.ex.feature_weight(self.ns, i)
        return (f, v)

    def iter_features(self) -> Iterator[Tuple[int, float]]:
        """iterate over all feature/value pairs in this namespace."""
        for i in range(self.num_features_in()):
            yield self[i]

    def push_feature(self, feature: Union[str, int], v: float = 1.0):
        """Add an unhashed feature to the current namespace (fails if
        setup has already run on this example).

        Args:
            feature: Feature to be pushed to current namespace
            v: Feature value, by default is 1.0
        """
        if self.ns_hash is None:
            self.ns_hash = self.ex.vw.hash_space(self.ns)
        self.ex.push_feature(self.ns, feature, v, self.ns_hash)

    def pop_feature(self):
        """Remove the top feature from the current namespace; returns True
        if a feature was removed, returns False if there were no
        features to pop."""
        return self.ex.pop_feature(self.ns)

    def push_features(self, feature_list, feature_list_legacy=None):
        """Push a list of features to a given namespace.

        Args:
            feature_list (List[Union[Tuple[Union[str, int], float], Union[str, int]]]): Each feature in the list can either be an integer (already hashed)
                or a string (to be hashed) and may be paired with a value or not
                (if not, the value is assumed to be 1.0).


        Examples:
            See :py:meth:`vowpalwabbit.Example.push_features` for examples.

            This function used to have a `ns` argument that never did anything and a `featureList` argument. The `ns` argument has been removed, so inly a feature list should be passed now.
            The function checks if the old way of calling was used and issues a warning.
        """

        if feature_list_legacy is not None:
            warnings.warn(
                "push_features only accepts a single positional argument now. Please remove the first unused ns argument.",
                DeprecationWarning,
            )
            feature_list = feature_list_legacy

        self.ex.push_features(self.ns, feature_list)


class AbstractLabel:
    """An abstract class for a VW label."""

    def __init__(self):
        pass

    @staticmethod
    def from_example(ex: "Example") -> "AbstractLabel":
        """Extract a label from the given example.

        Args:
            ex: example from which label is to be extracted
        """
        raise Exception("from_example not yet implemented")

    def __str__(self):
        raise Exception("__str__ not yet implemented")


class SimpleLabel(AbstractLabel):
    """Class for simple VW label"""

    def __init__(
        self,
        label: float = 0.0,
        weight: float = 1.0,
        initial: float = 0.0,
        prediction: float = 0.0,
    ):
        AbstractLabel.__init__(self)
        self.label = label
        self.weight = weight
        self.initial = initial
        self.prediction = prediction

    @staticmethod
    def from_example(ex: "Example"):
        label = ex.get_simplelabel_label()
        weight = ex.get_simplelabel_weight()
        initial = ex.get_simplelabel_initial()
        prediction = ex.get_simplelabel_prediction()
        return SimpleLabel(label, weight, initial, prediction)

    def __str__(self):
        s = str(self.label)
        if self.weight != 1.0:
            s += ":" + str(self.weight)
        return s


class MulticlassLabel(AbstractLabel):
    """Class for multiclass VW label with prediction"""

    def __init__(self, label: int = 1, weight: float = 1.0, prediction: int = 1):
        AbstractLabel.__init__(self)
        self.label = label
        self.weight = weight
        self.prediction = prediction

    @staticmethod
    def from_example(ex: "Example"):
        label = ex.get_multiclass_label()
        weight = ex.get_multiclass_weight()
        prediction = ex.get_multiclass_prediction()
        return MulticlassLabel(label, weight, prediction)

    def __str__(self):
        s = str(self.label)
        if self.weight != 1.0:
            s += ":" + str(self.weight)
        return s


class MulticlassProbabilitiesLabel(AbstractLabel):
    """Class for multiclass VW label with probabilities"""

    def __init__(self, prediction: Optional[float] = None):
        AbstractLabel.__init__(self)
        self.prediction = prediction

    @staticmethod
    def from_example(ex: "Example"):
        prediction = get_prediction(ex, PredictionType.MULTICLASSPROBS)
        return MulticlassProbabilitiesLabel(prediction)

    def __str__(self):
        s = []
        for label, prediction in enumerate(self.prediction):
            s.append("{l}:{p}".format(l=label + 1, p=prediction))
        return " ".join(s)


class CostSensitiveElement:
    def __init__(
        self,
        label: int,
        cost: float = 0.0,
        partial_prediction: float = 0.0,
        wap_value: float = 0.0,
    ):
        self.label = label
        self.cost = cost
        self.partial_prediction = partial_prediction
        self.wap_value = wap_value


class CostSensitiveLabel(AbstractLabel):
    """Class for cost sensative VW label"""

    def __init__(
        self,
        costs: List[CostSensitiveElement] = [],
        prediction: float = 0.0,
    ):
        AbstractLabel.__init__(self)
        self.costs = costs
        self.prediction = prediction

    @staticmethod
    def from_example(ex: "Example"):
        prediction = ex.get_costsensitive_prediction()
        costs = []
        for i in range(ex.get_costsensitive_num_costs()):
            cs = CostSensitiveElement(
                ex.get_costsensitive_class(i),
                ex.get_costsensitive_cost(i),
                ex.get_costsensitive_partial_prediction(i),
                ex.get_costsensitive_wap_value(i),
            )
            costs.append(cs)
        return CostSensitiveLabel(costs, prediction)

    def __str__(self):
        return " ".join(["{}:{}".format(c.label, c.cost) for c in self.costs])


class CBLabelElement:
    def __init__(
        self,
        action: Optional[int] = None,
        cost: float = 0.0,
        partial_prediction: float = 0.0,
        probability: float = 0.0,
        **kwargs,
    ):
        if kwargs.get("label", False):
            action = kwargs["label"]
            warnings.warn(
                "label has been deprecated. Please use 'action' instead.",
                DeprecationWarning,
            )
        self.label = action
        self.action = action
        self.cost = cost
        self.partial_prediction = partial_prediction
        self.probability = probability


class CBLabel(AbstractLabel):
    """Class for contextual bandits VW label"""

    def __init__(
        self,
        costs: List[CBLabelElement] = [],
        weight: float = 1.0,
    ):
        AbstractLabel.__init__(self)
        self.costs = costs
        self.weight = weight

    @staticmethod
    def from_example(ex: "Example"):
        weight = ex.get_cbandits_weight()
        costs = []
        for i in range(ex.get_cbandits_num_costs()):
            cb = CBLabelElement(
                ex.get_cbandits_class(i),
                ex.get_cbandits_cost(i),
                ex.get_cbandits_partial_prediction(i),
                ex.get_cbandits_probability(i),
            )
            costs.append(cb)
        return CBLabel(costs, weight)

    def __str__(self):
        return " ".join(
            ["{}:{}:{}".format(c.action, c.cost, c.probability) for c in self.costs]
        )


class CBEvalLabel(AbstractLabel):
    """Class for contextual bandits eval VW label"""

    def __init__(
        self,
        action: int,
        cb_label: CBLabel,
    ):
        AbstractLabel.__init__(self)
        self.action = action
        self.cb_label = cb_label

    @staticmethod
    def from_example(ex: "Example"):
        action = ex.get_cb_eval_action()
        weight = ex.get_cb_eval_weight()
        costs = []
        for i in range(ex.get_cb_eval_num_costs()):
            cb = CBLabelElement(
                ex.get_cb_eval_class(i),
                ex.get_cb_eval_cost(i),
                ex.get_cb_eval_partial_prediction(i),
                ex.get_cb_eval_probability(i),
            )
            costs.append(cb)
        cb_label = CBLabel(costs, weight)
        return CBEvalLabel(action, cb_label)

    def __str__(self):
        return f"{self.action} " + " ".join(
            [
                "{}:{}:{}".format(c.action, c.cost, c.probability)
                for c in self.cb_label.costs
            ]
        )


class CCBLabelType(IntEnum):
    UNSET = pylibvw.vw.tUNSET
    SHARED = pylibvw.vw.tSHARED
    ACTION = pylibvw.vw.tACTION
    SLOT = pylibvw.vw.tSLOT


class SlatesLabelType(IntEnum):
    UNSET = pylibvw.vw.tUNSET
    SHARED = pylibvw.vw.tSHARED
    ACTION = pylibvw.vw.tACTION
    SLOT = pylibvw.vw.tSLOT


class ActionScore:
    def __init__(self, action: int, score: float):
        self.action = action
        self.score = score


class CCBSlotOutcome:
    def __init__(self, cost: float, action_probs: List[ActionScore]):
        self.cost = cost
        self.action_probs = action_probs

    def __str__(self):
        top_action, top_score = self.action_probs[0].action, self.action_probs[0].score
        out = "{}:{}:{}".format(top_action, round(self.cost, 2), round(top_score, 2))
        for action_score in self.action_probs[1:]:
            out += f",{action_score.action}:{action_score.score}"
        return out


class CCBLabel(AbstractLabel):
    """Class for conditional contextual bandits VW label"""

    def __init__(
        self,
        type: CCBLabelType = CCBLabelType.UNSET,
        explicit_included_actions: List[int] = [],
        weight: float = 1,
        outcome: Optional[CCBSlotOutcome] = None,
    ):
        AbstractLabel.__init__(self)
        self.type = type
        self.explicit_included_actions = explicit_included_actions
        self.weight = weight
        self.outcome = outcome

    @staticmethod
    def from_example(ex: "Example"):
        type = ex.get_ccb_type()
        explicit_included_actions = ex.get_ccb_explicitly_included_actions()
        weight = ex.get_ccb_weight()
        outcome = None
        if ex.get_ccb_has_outcome():
            action_probs = []
            for i in range(ex.get_ccb_num_probabilities()):
                action_probs.append(
                    ActionScore(ex.get_ccb_action(i), ex.get_ccb_probability(i))
                )
            outcome = CCBSlotOutcome(ex.get_ccb_cost(), action_probs)
        return CCBLabel(type, explicit_included_actions, weight, outcome)

    def __str__(self):
        ret = "ccb "
        if self.type == CCBLabelType.SHARED:
            ret += "shared"
        elif self.type == CCBLabelType.ACTION:
            ret += "action"
        elif self.type == CCBLabelType.SLOT:
            ret += "slot"

        if self.outcome is not None:
            ret += (
                " "
                + str(self.outcome)
                + " "
                + ",".join(map(str, self.explicit_included_actions))
            )
        return ret


class SlatesLabel(AbstractLabel):
    """Class for slates VW label"""

    def __init__(
        self,
        type: SlatesLabelType = SlatesLabelType.UNSET,
        weight: float = 1.0,
        labeled: bool = False,
        cost: float = 0.0,
        slot_id: int = 0,
        probabilities: List[ActionScore] = [],
    ):
        abstract_label.__init__(self)
        self.type = type
        self.weight = weight
        self.labeled = labeled
        self.cost = cost
        self.slot_id = slot_id
        self.probabilities = probabilities

    @staticmethod
    def from_example(ex: "Example"):
        type = ex.get_slates_type()
        weight = ex.get_slates_weight()
        labeled = ex.get_slates_labeled()
        cost = ex.get_slates_cost()
        slot_id = ex.get_slates_slot_id()
        probabilities = []
        for i in range(ex.get_slates_num_probabilities()):
            probabilities.append(
                ActionScore(ex.get_slates_action(i), ex.get_slates_probability(i))
            )
        return SlatesLabel(type, weight, labeled, cost, slot_id, probabilities)

    def __str__(self):
        ret = "slates "
        if self.type == SlatesLabelType.SHARED:
            ret += "shared {}".format(round(self.cost, 2))
        elif self.type == SlatesLabelType.ACTION:
            ret += "action {}".format(self.slot_id)
        elif self.type == SlatesLabelType.SLOT:
            ret += "slot " + ",".join(
                [
                    "{}:{}".format(a_s.action, round(a_s.score, 2))
                    for a_s in self.probabilities
                ]
            )
        return ret


class CBContinuousLabelElement:
    def __init__(
        self, action: Optional[int] = None, cost: float = 0.0, pdf_value: float = 0.0
    ):
        self.action = action
        self.cost = cost
        self.pdf_value = pdf_value


class CBContinuousLabel(AbstractLabel):
    """Class for cb_continuous VW label"""

    def __init__(self, costs: List[CBContinuousLabelElement] = []):
        AbstractLabel.__init__(self)
        self.costs = costs

    @staticmethod
    def from_example(ex: "Example"):
        costs = []
        for i in range(ex.get_cb_continuous_num_costs()):
            elem = CBContinuousLabelElement(
                ex.get_cb_continuous_class(i),
                ex.get_cb_continuous_cost(i),
                ex.get_cb_continuous_pdf_value(i),
            )
            costs.append(elem)
        return CBContinuousLabel(costs)

    def __str__(self):
        return "ca " + " ".join(
            ["{}:{}:{}".format(c.action, c.cost, c.pdf_value) for c in self.costs]
        )


class MultilabelLabel(AbstractLabel):
    """Class for multilabel VW label"""

    def __init__(self, labels: List[int]):
        AbstractLabel.__init__(self)
        self.labels = labels

    @staticmethod
    def from_example(ex: "Example"):
        labels = ex.get_multilabel_labels()
        return MultilabelLabel(labels)

    def __str__(self):
        return ",".join(f"{l}" for l in self.labels)


class Example(pylibvw.example):
    """The example class is a wrapper around
    pylibvw.example. pylibvw.example should not be used. Most of the wrapping is to make the interface
    easier to use (by making the types safer via NamespaceId) and
    also with added python-specific functionality."""

    labelType: LabelType
    vw: Workspace
    stride: int
    setup_done: bool
    finished: bool

    def __init__(
        self,
        vw: Workspace,
        initStringOrDictOrRawExample: Optional[
            Union[
                str,
                Dict[str, List[Union[Tuple[Union[str, int], float], Union[str, int]]]],
                Dict[str, Dict[Union[str, int], float]],
                Any,
                pylibvw.example,
            ]
        ] = None,
        labelType: Optional[Union[int, LabelType]] = None,
    ):
        """Construct a new example from vw.

        Args:
            vw : Owning workspace of this example object
            initStringOrDictOrRawExample: Content to initialize the example with.

                - If None, created as an empty example
                - If a string, parsed as a VW example string
                - If a pylibvw.example, wraps the native example. This is advanced and should rarely be used.
                - If is a callable object will be called until it is no longer callable. At that point it should be another of the supported types.

                    .. deprecated:: 9.0.0
                        Using a callable object is no longer supported.

                - If a dict, the keys are the namespace names and the values are the namespace features. Namespace features can either be represented as a list or a dict. When using a list items are either keys (i.e., an int or string) in which case the value is assumed to be 1 or a key-value tuple. When using a dict the all features are represented as key-value pairs.
            labelType: Which label type this example contains. If None (or 0), the label type is inferred from the workspace configuration.

                .. deprecated:: 9.0.0
                        Supplying an integer is no longer supported. Use the LabelType enum instead.
        See Also:
            :py:class:`~vowpalwabbit.Workspace`
        """

        while hasattr(initStringOrDictOrRawExample, "__call__"):
            warnings.warn(
                "Passing a callable object for initStringOrDictOrRawExample is deprecated and will be removed in a future version.",
                DeprecationWarning,
            )
            initStringOrDictOrRawExample = initStringOrDictOrRawExample()

        if labelType is None:
            self.labelType = vw.get_label_type()
        elif isinstance(labelType, LabelType):
            self.labelType = labelType
        elif isinstance(labelType, int):
            warnings.warn(
                "labelType should be a LabelType enum value. Using an integer is deprecated.",
                DeprecationWarning,
            )
            if labelType == 0:
                self.labelType = vw.get_label_type()
            else:
                self.labelType = LabelType(labelType)
        else:
            raise ValueError(
                "labelType must be a LabelType enum value, integer or None"
            )

        if initStringOrDictOrRawExample is None:
            pylibvw.example.__init__(self, vw, self.labelType.value)
            self.setup_done = False
        elif isinstance(initStringOrDictOrRawExample, str):
            pylibvw.example.__init__(
                self, vw, self.labelType.value, initStringOrDictOrRawExample
            )
            self.setup_done = True
        elif isinstance(initStringOrDictOrRawExample, pylibvw.example):
            pylibvw.example.__init__(
                self, vw, self.labelType.value, initStringOrDictOrRawExample
            )
        elif isinstance(initStringOrDictOrRawExample, dict):
            pylibvw.example.__init__(self, vw, self.labelType.value)
            self.vw = vw
            self.stride = vw.get_stride()
            self.finished = False
            self.push_feature_dict(vw, initStringOrDictOrRawExample)
            self.setup_done = False
        else:
            raise TypeError(
                "expecting string or dict as argument for example construction"
            )

        self.vw = vw
        self.stride = vw.get_stride()
        self.finished = False

    def get_ns(self, id: Union[NamespaceId, str, int]) -> NamespaceId:
        """Construct a NamespaceId

        Argss:
            id (NamespaceId/str/integer): id used to create namespace

        Returns:
             NamespaceId created using parameter passed(if id was NamespaceId,
            just return it directly)
        """
        if isinstance(id, NamespaceId):
            return id
        else:
            return NamespaceId(self, id)

    def __getitem__(self, id: Union[NamespaceId, str, int]) -> ExampleNamespace:
        """Get an ExampleNamespace object associated with the given
        namespace id."""
        return ExampleNamespace(self, self.get_ns(id))

    def feature(self, ns: Union[NamespaceId, str, int], i: int) -> int:
        """Get the i-th hashed feature id in a given namespace

        Args:
            ns: namespace used to get the feature
            i: to get i-th hashed feature id in a given ns. It must range from
                0 to self.num_features_in(ns)-1

        Returns:
            i-th hashed feature-id in a given ns
        """
        ns = self.get_ns(ns)  # guaranteed to be a single character
        f = pylibvw.example.feature(self, ns.ord_ns, i)
        if self.setup_done:
            f = (f - self.get_ft_offset()) // self.stride
        return f

    def feature_weight(self, ns: Union[NamespaceId, str, int], i: int) -> float:
        """Get the value(weight) associated with a given feature id

        Args:
            ns: namespace used to get the feature id
            i: to get the weight of i-th feature in the given ns. It must range
                from 0 to self.num_features_in(ns)-1

        Returns:
            weight(value) of the i-th feature of given ns
        """
        return pylibvw.example.feature_weight(self, self.get_ns(ns).ord_ns, i)

    def set_label_string(self, string: str) -> None:
        """Give this example a new label

        Args:
            string: a new label to this example, formatted as a string (ala the VW data
                file format)
        """
        label_int = 0 if self.labelType is None else self.labelType.value
        pylibvw.example.set_label_string(self, self.vw, string, label_int)

    def setup_example(self):
        """If this example hasn't already been setup (ie, quadratic
        features constructed, etc.), do so."""
        if self.setup_done:
            raise Exception(
                "Trying to setup_example on an example that is already setup"
            )
        self.vw.setup_example(self)
        self.setup_done = True

    def unsetup_example(self):
        """If this example has been setup, reverse that process so you can
        continue editing the examples."""
        if not self.setup_done:
            raise Exception("Trying to unsetup_example that has not yet been setup")
        self.vw.unsetup_example(self)
        self.setup_done = False

    def learn(self):
        """Learn on this example (and before learning, automatically
        call setup_example if the example hasn't yet been setup)."""
        if not self.setup_done:
            self.setup_example()
        self.vw.learn(self)

    def sum_feat_sq(self, ns: Union[NamespaceId, str, int]) -> float:
        """Get the total sum feature-value squared for a given
        namespace

        Args:
            ns : namespace
                Get the total sum feature-value squared of this namespace

        Returns:
            Total sum feature-value squared of the given ns
        """
        return pylibvw.example.sum_feat_sq(self, self.get_ns(ns).ord_ns)

    def num_features_in(self, ns: Union[NamespaceId, str, int]) -> int:
        """Get the total number of features in a given namespace

        Args:
            ns : namespace
                Get the total features of this namespace

        Returns:
            Total number of features in the given ns
        """
        return pylibvw.example.num_features_in(self, self.get_ns(ns).ord_ns)

    def get_feature_id(
        self,
        ns: Union[NamespaceId, str, int],
        feature: Union[int, str],
        ns_hash: Optional[int] = None,
    ) -> int:
        """Get the hashed feature id for a given feature in a given
        namespace. feature can either be an integer (already a feature
        id) or a string, in which case it is hashed.

        Args:
            ns: namespace used to get the feature
            feature: If integer the already a feature else will be hashed
            ns_hash: The hash of the namespace. Optional.

        Returns:
            Hashed feature id

        .. note::
            If --hash all is on, then get_feature_id(ns,"5") !=
            get_feature_id(ns, 5). If you've already hashed the namespace,
            you can optionally provide that value to avoid re-hashing it.
        """
        if isinstance(feature, int):
            return feature
        if isinstance(feature, str):
            if ns_hash is None:
                ns_hash = self.vw.hash_space(self.get_ns(ns).ns)
            return self.vw.hash_feature(feature, ns_hash)
        raise Exception("cannot extract feature of type: " + str(type(feature)))

    def push_hashed_feature(
        self, ns: Union[NamespaceId, str, int], f: int, v: float = 1.0
    ) -> None:
        """Add a hashed feature to a given namespace.

        Args:
            ns : namespace
                namespace in which the feature is to be pushed
            f : integer
                feature
            v : float
                The value of the feature, be default is 1.0
        """
        if self.setup_done:
            self.unsetup_example()
        pylibvw.example.push_hashed_feature(self, self.get_ns(ns).ord_ns, f, v)

    def push_feature(
        self,
        ns: Union[NamespaceId, str, int],
        feature: Union[str, int],
        v: float = 1.0,
        ns_hash: Optional[int] = None,
    ) -> None:
        """Add an unhashed feature to a given namespace

        Args:
            ns: namespace in which the feature is to be pushed
            f: feature
            v: The value of the feature, be default is 1.0
            ns_hash : Optional, by default is None
                The hash of the namespace
        """
        f = self.get_feature_id(ns, feature, ns_hash)
        self.push_hashed_feature(ns, f, v)

    def pop_feature(self, ns: Union[NamespaceId, str, int]) -> bool:
        """Remove the top feature from a given namespace

        Args:
            ns: namespace from which feature is popped

        Returns:
            True if feature was removed else False as no feature was there to
            pop
        """
        if self.setup_done:
            self.unsetup_example()
        return pylibvw.example.pop_feature(self, self.get_ns(ns).ord_ns)

    def push_namespace(self, ns: Union[NamespaceId, str, int]) -> None:
        """Push a new namespace onto this example.
        You should only do this if you're sure that this example doesn't
        already have the given namespace

        Args:
            ns: namespace which is to be pushed onto example

        """
        if self.setup_done:
            self.unsetup_example()
        pylibvw.example.push_namespace(self, self.get_ns(ns).ord_ns)

    def pop_namespace(self) -> bool:
        """Remove the top namespace from an example

        Returns:
            True if namespace was removed else False as no namespace was there
            to pop
        """
        if self.setup_done:
            self.unsetup_example()
        return pylibvw.example.pop_namespace(self)

    def ensure_namespace_exists(self, ns: Union[NamespaceId, str, int]):
        """Check to see if a namespace already exists.

        Args:
            ns: If namespace exists does, do nothing. If it doesn't, add it.
        """
        if self.setup_done:
            self.unsetup_example()
        return pylibvw.example.ensure_namespace_exists(self, self.get_ns(ns).ord_ns)

    def push_features(
        self,
        ns: Union[NamespaceId, str, int],
        featureList: List[Union[Tuple[Union[str, int], float], Union[str, int]]],
    ):
        """Push a list of features to a given namespace.

        Args:
            ns: namespace in which the features are pushed
            featureList : Each feature in the list can either be an integer
                (already hashed) or a string (to be hashed) and may be
                paired with a value or not (if not, the value is assumed to be 1.0

        Examples:
            >>> from vowpalwabbit import Workspace
            >>> vw = Workspace(quiet=True)
            >>> ex = vw.example('1 |a two features |b more features here')
            >>> ex.push_features('x', ['a', 'b'])
            >>> ex.push_features('y', [('c', 1.), 'd'])
            >>> space_hash = vw.hash_space('x')
            >>> feat_hash  = vw.hash_feature('a', space_hash)
            >>> ex.push_features('x', [feat_hash]) #'x' should match the space_hash!
            >>> ex.num_features_in('x')
            3
            >>> ex.num_features_in('y')
            2
        """
        ns = self.get_ns(ns)
        self.ensure_namespace_exists(ns)
        self.push_feature_list(
            self.vw, ns.ord_ns, featureList
        )  # much faster just to do it in C++
        # ns_hash = self.vw.hash_space( ns.ns )
        # for feature in featureList:
        #     if isinstance(feature, int) or isinstance(feature, str):
        #         f = feature
        #         v = 1.
        #     elif isinstance(feature, tuple) and len(feature) == 2 and
        #       (isinstance(feature[0], int) or isinstance(feature[0], str))
        #    and (isinstance(feature[1], int) or isinstance(feature[1], float)):
        #         f = feature[0]
        #         v = feature[1]
        #     else:
        #         raise Exception('malformed feature to push of type: '
        #                        + str(type(feature)))
        #     self.push_feature(ns, f, v, ns_hash)

    def iter_features(self) -> Iterator[Tuple[int, float]]:
        """Iterate over all feature/value pairs in this example (all
        namespace included)."""
        for ns_id in range(self.num_namespaces()):  # iterate over every namespace
            ns = self.get_ns(ns_id)
            for i in range(self.num_features_in(ns)):
                f = self.feature(ns, i)
                v = self.feature_weight(ns, i)
                yield f, v

    def get_label(
        self, label_class: Optional[Union[int, LabelType, Type[AbstractLabel]]] = None
    ) -> Union[
        "AbstractLabel",
        "SimpleLabel",
        "MulticlassLabel",
        "CostSensitiveLabel",
        "CBLabel",
        "CCBLabel",
        "SlatesLabel",
        "CBContinuousLabel",
    ]:
        """Get the label object of this example.

        Args:
            label_class :
                - If None, self.labelType will be used.
                - If int then corresponding :py:obj:`~vowpalwabbit.pyvw.LabelType` for the label type to be retrieved.
                - The ability to pass an AbstractLabel or an int are legacy requirements and are deprecated. All new usage of this function should pass a LabelType.

        See Also:
            :meth:`vowpalwabbit.Workspace.get_label_type`

        """

        if label_class is None:
            label_class = self.labelType

        label_class_type = None
        if inspect.isclass(label_class) and issubclass(label_class, AbstractLabel):
            warnings.warn(
                "Passing an AbstractLabel to get_prediction is deprecated and will be removed in a future release. Please pass a LabelType instead.",
                DeprecationWarning,
            )
            label_class_type = label_class
        else:
            if not isinstance(label_class, LabelType) and isinstance(label_class, int):
                warnings.warn(
                    "Passing an integer to get_prediction is deprecated and will be removed in a future release. Please pass a LabelType instead.",
                    DeprecationWarning,
                )
                if label_class == 0:
                    label_class = None
                else:
                    label_class = LabelType(label_class)

            label_class_type = get_label_class_from_enum(label_class)

        return label_class_type.from_example(self)

    def get_prediction(
        self, prediction_type: Optional[Union[int, PredictionType]] = None
    ) -> Union[
        float,
        List[float],
        int,
        List[int],
        float,
        List[List[Tuple[int, float]]],
        Tuple[int, float],
        List[Tuple[float, float, float]],
        Tuple[int, List[int]],
        str,
    ]:

        """Get prediction object from this example.

        Args:
            prediction_type:
                - If None, the label type of the example's owning Workspace instance will be used.
                - If int then corresponding :py:obj:`~vowpalwabbit.pyvw.PredictionType` for the prediction type to be retrieved.
                - Supplying an int is deprecated and will be removed in a future release.

        Returns:
            Prediction according to parameter prediction_type
                - :py:obj:`~vowpalwabbit.PredictionType.SCALAR`: float
                - :py:obj:`~vowpalwabbit.PredictionType.SCALARS`: List[float]
                - :py:obj:`~vowpalwabbit.PredictionType.ACTION_SCORES`: List[float]
                - :py:obj:`~vowpalwabbit.PredictionType.ACTION_PROBS`: List[float]
                - :py:obj:`~vowpalwabbit.PredictionType.MULTICLASS`: int
                - :py:obj:`~vowpalwabbit.PredictionType.MULTILABELS`: List[int]
                - :py:obj:`~vowpalwabbit.PredictionType.PROB`: float
                - :py:obj:`~vowpalwabbit.PredictionType.MULTICLASSPROBS`: List[float]
                - :py:obj:`~vowpalwabbit.PredictionType.DECISION_SCORES`: List[List[Tuple[int, float]]]
                - :py:obj:`~vowpalwabbit.PredictionType.ACTION_PDF_VALUE`: Tuple[int, float]
                - :py:obj:`~vowpalwabbit.PredictionType.PDF`: List[Tuple[float, float, float]]
                - :py:obj:`~vowpalwabbit.PredictionType.ACTIVE_MULTICLASS`: Tuple[int, List[int]]
                - :py:obj:`~vowpalwabbit.PredictionType.NOPRED`: str

        Examples:
            >>> from vowpalwabbit import Workspace, PredictionType
            >>> vw = Workspace(quiet=True)
            >>> ex = vw.example('1 |a two features |b more features here')
            >>> ex.get_prediction()
            0.0

        See Also:
            :meth:`vowpalwabbit.Workspace.get_prediction_type`
        """

        if prediction_type is None:
            prediction_type = self.vw.get_prediction_type()

        if not isinstance(prediction_type, PredictionType) and isinstance(
            prediction_type, int
        ):
            warnings.warn(
                "Passing an integer to get_prediction is deprecated and will be removed in a future release. Please pass a PredictionType instead.",
                DeprecationWarning,
            )
            prediction_type = PredictionType(prediction_type)

        switch_prediction_type = {
            PredictionType.SCALAR: self.get_simplelabel_prediction,
            PredictionType.SCALARS: self.get_scalars,
            PredictionType.ACTION_SCORES: self.get_action_scores,
            PredictionType.ACTION_PROBS: self.get_action_scores,
            PredictionType.MULTICLASS: self.get_multiclass_prediction,
            PredictionType.MULTILABELS: self.get_multilabel_predictions,
            PredictionType.PROB: self.get_prob,
            PredictionType.MULTICLASSPROBS: self.get_scalars,
            PredictionType.DECISION_SCORES: self.get_decision_scores,
            PredictionType.ACTION_PDF_VALUE: self.get_action_pdf_value,
            PredictionType.PDF: self.get_pdf,
            PredictionType.ACTIVE_MULTICLASS: self.get_active_multiclass,
            PredictionType.NOPRED: self.get_nopred,
        }
        return switch_prediction_type[prediction_type]()


def merge_models(base_model: Optional[Workspace], models: List[Workspace]) -> Workspace:
    """Merge the models loaded into separate workspaces into a single workspace which can then be serialized to a model.

    All of the given workspaces must use the exact same arguments, and only differ in training. `--preserve_performance_counters` should be used if models are loaded from disk and then given to this call.

    Args:
        base_model (Optional[Workspace]): Base model the other models were based on. None, if they are trained from scratch.
        models (List[Workspace]): List of models to merge.

    Returns:
        Workspace: The merged workspace

    Example:
        >>> from vowpalwabbit import Workspace, merge_models
        >>> model1 = Workspace(quiet=True, preserve_performance_counters=True, initial_regressor='model1.model') # doctest: +SKIP
        >>> model2 = Workspace(quiet=True, preserve_performance_counters=True, initial_regressor='model2.model') # doctest: +SKIP
        >>> merged_model = merge_models([model1, model2]) # doctest: +SKIP
        >>> merged_model.save('merged.model') # doctest: +SKIP

    .. note::
        This is an experimental feature and may change in future releases.
    """

    # This needs to be promoted to the Workspace object defined in Python.
    result = pylibvw._merge_models_impl(base_model, models)
    result.__class__ = Workspace
    result._log_wrapper = None
    result.parser_ran = False
    result.init = True
    result.finished = False
    result._log_fwd = None
    return result


############################ DEPREECATED CLASSES ############################


class abstract_label(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.AbstractLabel`. `abstract_label` is now deprecated.
    """

    _DeprecatedClassMeta__alias = AbstractLabel


class simple_label(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.SimpleLabel`. `simple_label` is now deprecated.
    """

    _DeprecatedClassMeta__alias = SimpleLabel


class multiclass_label(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.MulticlassLabel`. `multiclass_label` is now deprecated.
    """

    _DeprecatedClassMeta__alias = MulticlassLabel


class multiclass_probabilities_label(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.MulticlassProbabilitiesLabel`. `multiclass_probabilities_label` is now deprecated.
    """

    _DeprecatedClassMeta__alias = MulticlassProbabilitiesLabel


class cost_sensitive_label(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.CostSensitiveLabel`. `cost_sensitive_label` is now deprecated.
    """

    _DeprecatedClassMeta__alias = CostSensitiveLabel


class cbandits_label(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.CBLabel`. `cbandits_label` is now deprecated.
    """

    _DeprecatedClassMeta__alias = CBLabel


class namespace_id(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.NamespaceId`. `namespace_id` is now deprecated.
    """

    _DeprecatedClassMeta__alias = NamespaceId


class example_namespace(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.ExampleNamespace`. `example_namespace` is now deprecated.
    """

    _DeprecatedClassMeta__alias = ExampleNamespace


class vw(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.Workspace`. `vw` is now deprecated.
    """

    _DeprecatedClassMeta__alias = Workspace


class example(metaclass=_DeprecatedClassMeta):
    """
    .. deprecated:: 9.0.0
        This has been renamed to :py:obj:`~vowpalwabbit.Example`. `example` is now deprecated.
    """

    _DeprecatedClassMeta__alias = Example
