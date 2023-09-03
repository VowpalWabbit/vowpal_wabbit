import random
import os
from test_helper import get_function_object

script_directory = os.path.dirname(os.path.realpath(__file__))


def random_number_items(items):
    num_items_to_select = random.randint(1, len(items))
    return random.sample(items, num_items_to_select)


def generate_cb_data(
    f,
    num_examples,
    num_features,
    num_actions,
    reward_function,
    logging_policy,
    context_name=["1"],
    seed=random.randint(0, 100),
):
    random.seed(seed)

    reward_function_obj = get_function_object(
        "cb.reward_functions", reward_function["name"]
    )
    logging_policy_obj = get_function_object(
        "cb.logging_policies", logging_policy["name"]
    )
    features = [f"feature{index}" for index in range(1, num_features + 1)]
    for _ in range(num_examples):
        no_context = len(context_name)
        if no_context > 1:
            context = random.randint(1, no_context)
        else:
            context = 1

        def return_cost_probability(chosen_action, context=1):
            cost = -reward_function_obj(
                chosen_action, context, **reward_function["params"]
            )
            if "params" not in logging_policy:
                logging_policy["params"] = {}
            logging_policy["params"]["chosen_action"] = chosen_action
            logging_policy["params"]["num_actions"] = num_actions
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
