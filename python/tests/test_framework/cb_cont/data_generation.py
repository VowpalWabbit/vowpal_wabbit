import random
import os
from test_helper import get_function_object

script_directory = os.path.dirname(os.path.realpath(__file__))
random.seed(10)


def random_number_items(items):
    num_items_to_select = random.randint(1, len(items))
    return random.sample(items, num_items_to_select)


def generate_cb_data(
    num_examples,
    num_features,
    action_range,
    reward_function,
    logging_policy,
    context_name=["1"],
):
    num_actions = int(abs(action_range[1] - action_range[0]))
    dataFile = f"cb_cont_test_{num_examples}_{num_actions}_{num_features}.txt"

    reward_function_obj = get_function_object(
        "cb_cont.reward_functions", reward_function["name"]
    )
    logging_policy_obj = get_function_object(
        "cb_cont.logging_policies", logging_policy["name"]
    )
    features = [f"feature{index}" for index in range(1, num_features + 1)]
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for _ in range(num_examples):
            no_context = len(context_name)
            if no_context > 1:
                context = random.randint(1, no_context)
            else:
                context = 1

            def return_cost_probability(chosen_action, context):
                cost = -reward_function_obj(
                    chosen_action, context, **reward_function["params"]
                )
                if "params" not in logging_policy:
                    logging_policy["params"] = {}
                logging_policy["params"]["chosen_action"] = chosen_action
                logging_policy["params"]["num_actions"] = num_actions
                probability = logging_policy_obj(**logging_policy["params"])
                return cost, probability

            chosen_action = round(random.uniform(0, num_actions), 2)
            cost, probability = return_cost_probability(chosen_action, context)
            if no_context == 1:
                f.write(
                    f'ca {chosen_action}:{cost}:{probability} | {" ".join(random_number_items(features))}\n'
                )
            else:
                f.write(
                    f'ca {chosen_action}:{cost}:{probability} | {"s_" + context_name[context-1]} {" ".join(random_number_items(features))}\n'
                )
            f.write("\n")
    return os.path.join(script_directory, dataFile)
