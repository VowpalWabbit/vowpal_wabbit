import random
import os

script_directory = os.path.dirname(os.path.realpath(__file__))


def constant_function(
    f, no_sample, constant, x_lower_bound, x_upper_bound, seed=random.randint(0, 100)
):
    random.seed(seed)
    for _ in range(no_sample):
        x = random.uniform(x_lower_bound, x_upper_bound)
        f.write(f"{constant} |f x:{x}\n")
