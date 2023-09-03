def fixed_reward(chosen_action, context):
    return 1


def constant_reward(chosen_action, context, reward):
    return reward[chosen_action - 1]


def fixed_reward_two_action(chosen_action, context):
    if context == 1 and chosen_action == 2:
        return 1
    elif context == 2 and chosen_action == 2:
        return 0
    elif context == 1 and chosen_action == 1:
        return 0
    elif context == 2 and chosen_action == 1:
        return 1
    return 1
