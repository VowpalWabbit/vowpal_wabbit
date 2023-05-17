from vw_executor.vw import Vw
from vw_executor.vw_opts import Grid
from numpy.testing import assert_allclose
import pandas as pd
import numpy as np
import random

with open("regression1.txt", "w") as f:
    for i in range(10000):
        x = random.uniform(1, 100)
        y = 5
        f.write(f"{y} |f x:{x}\n")


def core_test(files, grid, outputs, job_assert):
    vw = Vw(".vw_cache", reset=True)
    result = vw.train(files, grid, outputs)
    job_assert(result)


def test_regression_weight():
    options = Grid(
        {
            "#base": ["-P 50000 --preserve_performance_counters --save_resume "],
            "#reg": ["", "--coin", "--ftrl", "--pistol"],
        }
    )

    def assert_job(job):
        tolerance = 1
        constant = 5
        for j in job:
            assert j.status.value == 3
            data = j.outputs["--readable_model"]
            with open(data[0], "r") as f:
                data = f.readlines()
            data = [i.strip() for i in data]
            weight_b = float(data[-1].split(":")[1].split(" ")[0])
            weight_x1 = float(data[-2].split(":")[1].split(" ")[0])
            print(f"Running for command {j.opts}")
            print(weight_x1, weight_b)
            err_msg = (
                lambda w, we, wName: f"assert fail for {wName}: actual: {w}, expected {we}"
            )
            assert np.isclose(weight_b, constant, atol=tolerance), err_msg(
                weight_b, constant, "Bias"
            )
            assert np.isclose(weight_x1, 0, atol=tolerance), err_msg(
                weight_x1, 0, "w_x1"
            )

            # checking prediction
            # predictions = j.outputs['-p']
            # with open(predictions[0], "r") as f:
            #     predictions = f.readlines()
            #     predictions = [float(i) for i in predictions[1:]]
            # assert assert_allclose(predictions, [5]*len(predictions)), "predicted value should be 5"

    core_test("regression1.txt", options, ["--readable_model", "-p"], assert_job)


test_regression_weight()
