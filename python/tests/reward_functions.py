def fixed_reward(**kwargs):
    return 1


def constant_reward(**kwargs):
    reward = kwargs["reward"]
    chosen_action = kwargs["chosen_action"]
    return reward[chosen_action - 1]


def fixed_reward_for_diff_context(**kwargs):
    chosen_action = kwargs["chosen_action"]
    chosen_context = kwargs["chosen_context"]
    if chosen_context == 1 and chosen_action == 2:
        return 1
    elif chosen_context == 2 and chosen_action == 2:
        return 0
    elif chosen_context == 1 and chosen_action == 1:
        return 0
    elif chosen_context == 2 and chosen_action == 1:
        return 1
    return 1
