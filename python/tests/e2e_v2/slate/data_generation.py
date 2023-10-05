import random
import os
from test_helper import get_function_object

script_directory = os.path.dirname(os.path.realpath(__file__))


def generate_slate_data(
    f,
    num_examples,
    reward_function,
    logging_policy,
    action_space,
    context_name=["1"],
    seed=random.randint(0, 100),
):
    random.seed(seed)
    action_space_obj = get_function_object("slate.action_space", action_space["name"])

    reward_function_obj = get_function_object(
        "slate.reward_functions", reward_function["name"]
    )
    logging_policy_obj = get_function_object(
        "slate.logging_policies", logging_policy["name"]
    )

    def return_cost_probability(chosen_action, chosen_slot, context):
        cost = -reward_function_obj(
            chosen_action, context, chosen_slot, **reward_function["params"]
        )
        logging_policy["params"]["num_action"] = num_actions[chosen_slot - 1]
        logging_policy["params"]["chosen_action"] = chosen_action
        probability = logging_policy_obj(**logging_policy["params"])
        return cost, probability

    for i in range(num_examples):
        action_space["params"]["iteration"] = i
        action_spaces = action_space_obj(**action_space["params"])
        reward_function["params"]["iteration"] = i
        num_slots = len(action_spaces)
        num_actions = [len(slot) for slot in action_spaces]
        slot_name = [f"slot_{index}" for index in range(1, num_slots + 1)]
        chosen_actions = []
        num_context = len(context_name)
        if num_context > 1:
            context = random.randint(1, num_context)
        else:
            context = 1
        for s in range(num_slots):
            chosen_actions.append(random.randint(1, num_actions[s]))
        chosen_actions_cost_prob = [
            return_cost_probability(action, slot + 1, context)
            for slot, action in enumerate(chosen_actions)
        ]
        total_cost = sum([cost for cost, _ in chosen_actions_cost_prob])

        f.write(f"slates shared {total_cost} |User {context_name[context-1]}\n")
        # write actions
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
