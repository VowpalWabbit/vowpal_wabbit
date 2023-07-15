import random
import os

script_directory = os.path.dirname(os.path.realpath(__file__))
random.seed(10)


def constant_function(no_sample, constant, lower_bound, upper_bound):
    dataFile = f"constant_func_{no_sample}_{constant}_{upper_bound}_{lower_bound}.txt"
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for _ in range(no_sample):
            x = random.uniform(lower_bound, upper_bound)
            f.write(f"{constant} |f x:{x}\n")
    return os.path.join(script_directory, dataFile)


def random_number_items(items):
    num_items_to_select = random.randint(1, len(items))
    return random.sample(items, num_items_to_select)
