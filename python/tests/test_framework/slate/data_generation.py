import random
import os
from test_helper import get_function_object

script_directory = os.path.dirname(os.path.realpath(__file__))
random.seed(10)


def random_number_items(items):
    num_items_to_select = random.randint(1, len(items))
    return random.sample(items, num_items_to_select)


def generate_slate_data(
    num_examples,
    num_actions,
    reward_function,
    logging_policy,
    action_space,
    num_slots=1,
    num_context=1,
    context_name=None,
    slot_name=None,
):

    dataFile = f"slate_test_{num_examples}_{len(num_actions)}_{num_slots}.txt"

    reward_function_obj = get_function_object(
        "slate.reward_functions", reward_function["name"]
    )
    logging_policy_obj = get_function_object(
        "slate.logging_policies", logging_policy["name"]
    )

    action_space_obj = get_function_object("slate.action_space", action_space["name"])

    def return_cost_probability(chosen_action, chosen_slot, context=1):
        cost = reward_function_obj(
            chosen_action, context, chosen_slot, **reward_function["params"]
        )
        logging_policy["params"]["num_action"] = num_actions[chosen_slot - 1]
        logging_policy["params"]["chosen_action"] = chosen_action
        probability = logging_policy_obj(**logging_policy["params"])
        return cost, probability

    if not slot_name:
        slot_name = [f"slot_{index}" for index in range(1, num_slots + 1)]
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for i in range(num_examples):
            chosen_actions = []
            if num_context > 1:
                context = random.randint(1, num_context)
                if not context_name:
                    context_name = [f"{index}" for index in range(1, num_context + 1)]
            for s in range(num_slots):
                chosen_actions.append(random.randint(1, num_actions[s]))
            chosen_actions_cost_prob = [
                return_cost_probability(action, slot + 1, context)
                for slot, action in enumerate(chosen_actions)
            ]
            total_cost = sum([cost for cost, _ in chosen_actions_cost_prob])

            f.write(f"slates shared {total_cost} |User {context_name[context-1]}\n")
            # write actions
            action_space["params"]["iteration"] = i
            action_spaces = action_space_obj(**action_space["params"])
            for ind, slot in enumerate(action_spaces):
                for a in slot:
                    f.write(
                        f"slates action {ind} |Action {a}\n",
                    )

            for s in range(num_slots):
                f.write(
                    f"slates slot {chosen_actions[s]}:{chosen_actions_cost_prob[s][1]} |Slot {slot_name[s]}\n"
                )
            f.write("\n")
    return os.path.join(script_directory, dataFile)
