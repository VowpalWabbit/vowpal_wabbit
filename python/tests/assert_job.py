import numpy as np
from numpy.testing import assert_allclose, assert_almost_equal
from vw_executor.vw import ExecutionStatus


def remove_non_digits(string):
    return "".join(char for char in string if char.isdigit() or char == ".")


def get_from_kwargs(kwargs, key, default=None):
    if key in kwargs:
        return kwargs[key]
    else:
        return default


def majority_close(arr1, arr2, rtol, atol, threshold):
    # Check if the majority of elements are close
    close_count = np.count_nonzero(np.isclose(arr1, arr2, rtol=rtol, atol=atol))
    return close_count >= len(arr1) * threshold


def assert_weight(job, **kwargs):
    atol = get_from_kwargs(kwargs, "atol", 10e-8)
    rtol = get_from_kwargs(kwargs, "rtol", 10e-5)
    expected_weights = kwargs["expected_weights"]
    assert job.status == ExecutionStatus.Success, f"{job.opts} job should be successful"
    data = job.outputs["--readable_model"]
    with open(data[0], "r") as f:
        data = f.readlines()
    data = [i.strip() for i in data]
    weights = job[0].model9("--readable_model").weights
    weights = weights["weight"].to_list()
    assert_allclose(
        weights, expected_weights, atol=atol, rtol=rtol
    ), f"weights should be {expected_weights}"


def assert_prediction(job, **kwargs):
    assert job.status == ExecutionStatus.Success, "job should be successful"
    atol = kwargs.get("atol", 10e-8)
    rtol = kwargs.get("rtol", 10e-5)
    threshold = kwargs.get("threshold", 0.9)
    expected_value = kwargs["expected_value"]
    predictions = job.outputs["-p"]
    with open(predictions[0], "r") as f:
        prediction = [i.strip() for i in f.readlines()]
        prediction = [i for i in prediction if i != ""]
        if ":" in prediction[0]:
            prediction = [[j.split(":")[1] for j in i.split(",")] for i in prediction]
        if type(prediction[0]) == list:
            prediction = [[float(remove_non_digits(j)) for j in i] for i in prediction]
        else:
            prediction = [float(remove_non_digits(i)) for i in prediction]
        assert majority_close(
            prediction,
            [expected_value] * len(prediction),
            rtol=rtol,
            atol=atol,
            threshold=threshold,
        ), f"predicted value should be {expected_value}, \n actual values are {prediction}"


def assert_loss(job, **kwargs):
    assert job.status == ExecutionStatus.Success, "job should be successful"
    assert type(job[0].loss) == float, "loss should be an float"
    if job[0].loss < kwargs["expected_loss"]:
        return
    assert_almost_equal(job[0].loss, kwargs["expected_loss"], decimal=2)
