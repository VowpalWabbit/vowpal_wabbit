import random
import os

script_directory = os.path.dirname(os.path.realpath(__file__))


def constant_function(
    no_sample, constant, x_lower_bound, x_upper_bound, seed=random.randint(0, 100)
):
    random.seed(seed)
    dataFile = (
        f"constant_func_{no_sample}_{constant}_{x_upper_bound}_{x_lower_bound}.txt"
    )
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for _ in range(no_sample):
            x = random.uniform(x_lower_bound, x_upper_bound)
            f.write(f"{constant} |f x:{x}\n")
    return os.path.join(script_directory, dataFile)
