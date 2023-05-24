import numpy as np
from numpy.testing import assert_allclose
from vw_executor.vw import ExecutionStatus

def assert_weight(job, expected_weights, tolerance):
    assert job.status == ExecutionStatus.Success, "job should be successful"
    data = job.outputs["--readable_model"]
    with open(data[0], "r") as f:
        data = f.readlines()
    data = [i.strip() for i in data]
    weights = job[0].model9('--readable_model').weights
    weights = weights["weight"].to_list()
    assert_allclose(weights, expected_weights, atol=tolerance), f"weights should be {expected_weights}"

def assert_prediction(job, constant, tolerance):
        predictions = job.outputs['-p']
        with open(predictions[0], "r") as f:
            predictions = f.readlines()
            predictions = [float(i) for i in predictions[1:]]
            assert assert_allclose(predictions, [constant]*len(predictions), atol=tolerance), f"predicted value should be {constant}"



def assert_functions():
    return 