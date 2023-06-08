import random
import os
from test_helper import get_function_object

script_directory = os.path.dirname(os.path.realpath(__file__))


def constant_function(no_sample, constant, lower_bound, upper_bound):
    dataFile = f"constant_func_{no_sample}_{constant}_{upper_bound}_{lower_bound}.txt"
    with open(script_directory + "/" + dataFile, "w") as f:
        random.seed(10)
        for _ in range(no_sample):
            x = random.uniform(lower_bound, upper_bound)
            f.write(f"{constant} |f x:{x}\n")
    return dataFile


def random_number_items(items):
    num_items_to_select = random.randint(1, len(items))
    return random.sample(items, num_items_to_select)


def generate_cb_data(
    num_examples, num_features, num_actions, reward_function, probability_function
):
    reward_function_obj = get_function_object(
        "reward_functions", reward_function["name"]
    )
    probability_function_obj = get_function_object(
        "probability_functions", probability_function["name"]
    )
    dataFile = f"cb_test_{num_examples}_{num_actions}_{num_features}.txt"
    features = [f"feature{index}" for index in range(1, num_features + 1)]
    with open(script_directory + "/" + dataFile, "w") as f:
        for _ in range(num_examples):
            chosen_action = random.randint(1, num_actions)
            cost = reward_function_obj(chosen_action, **reward_function["params"])
            probability = probability_function_obj(
                chosen_action, **probability_function["params"]
            )
            f.write(
                f'{chosen_action}:{cost}:{probability} | {" ".join(random_number_items(features))}\n'
            )
    return dataFile
