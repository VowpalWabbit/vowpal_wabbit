import random
import os
from test_helper import get_function_object

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


def generate_cb_data(
    num_examples,
    num_features,
    num_actions,
    reward_function,
    logging_policy,
    no_context=1,
    context_name=None,
):

    dataFile = f"cb_test_{num_examples}_{num_actions}_{num_features}.txt"

    reward_function_obj = get_function_object(
        "reward_functions", reward_function["name"]
    )
    logging_policy_obj = get_function_object("logging_policies", logging_policy["name"])
    features = [f"feature{index}" for index in range(1, num_features + 1)]
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for _ in range(num_examples):
            if no_context > 1:
                context = random.randint(1, no_context)
                if not context_name:
                    context_name = [f"{index}" for index in range(1, no_context + 1)]

            def return_cost_probability(chosen_action, context=1):
                cost = reward_function_obj(
                    chosen_action, context, **reward_function["params"]
                )
                logging_policy["params"]["chosen_action"] = chosen_action
                probability = logging_policy_obj(**logging_policy["params"])
                return cost, probability

            chosen_action = random.randint(1, num_actions)
            if no_context > 1:
                f.write(f"shared | User s_{context_name[context-1]}\n")
                for action in range(1, num_actions + 1):

                    cost, probability = return_cost_probability(action, context)
                    if action == chosen_action:
                        f.write(
                            f'{action}:{cost}:{probability} | {" ".join(random_number_items(features))}\n'
                        )
                    else:
                        f.write(f'| {" ".join(random_number_items(features))}\n')

            else:

                cost, probability = return_cost_probability(chosen_action)
                f.write(
                    f'{chosen_action}:{cost}:{probability} | {" ".join(random_number_items(features))}\n'
                )
            f.write("\n")
    return os.path.join(script_directory, dataFile)


def generate_classification_data(
    num_sample,
    num_classes,
    num_features,
    classify_func,
    bounds=None,
):
    dataFile = f"classification_{num_classes}_{num_features}_{num_sample}.txt"
    classify_func_obj = get_function_object(
        "classification_functions", classify_func["name"]
    )
    if not bounds:
        bounds = [[0, 1] for _ in range(num_features)]
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for _ in range(num_sample):
            x = [
                random.uniform(bounds[index][0], bounds[index][1])
                for index in range(num_features)
            ]
            y = classify_func_obj(x, **classify_func["params"])
            f.write(
                f"{y} |f {' '.join([f'x{i}:{x[i]}' for i in range(num_features)])}\n"
            )
    return os.path.join(script_directory, dataFile)
