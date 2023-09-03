from vw_executor.vw import ExecutionStatus
import numpy as np


def majority_close(arr1, arr2, rtol, atol, threshold):
    # Check if the majority of elements are close
    close_count = np.count_nonzero(np.isclose(arr1, arr2, rtol=rtol, atol=atol))
    return close_count >= len(arr1) * threshold


def assert_prediction(job, **kwargs):
    assert job.status == ExecutionStatus.Success, "job should be successful"
    atol = kwargs.get("atol", 10e-8)
    rtol = kwargs.get("rtol", 10e-5)
    threshold = kwargs.get("threshold", 0.9)
    expected_value = kwargs["expected_value"]
    predictions = job.outputs["-p"]
    res = []
    with open(predictions[0], "r") as f:
        exampleRes = []
        while True:
            line = f.readline()
            if not line:
                break
            if line.count(":") == 0:
                res.append(exampleRes)
                exampleRes = []
                continue
            slotRes = [0] * line.count(":")
            slot = line.split(",")
            for i in range(len(slot)):
                actionInd = int(slot[i].split(":")[0])
                slotRes[i] = float(slot[actionInd].split(":")[1])
            exampleRes.append(slotRes)

        assert majority_close(
            res,
            [expected_value] * len(res),
            rtol=rtol,
            atol=atol,
            threshold=threshold,
        ), f"predicted value should be {expected_value}, \n actual values are {res}"
