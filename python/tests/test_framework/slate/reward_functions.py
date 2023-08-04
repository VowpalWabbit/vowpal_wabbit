def fixed_reward(chosen_action, context, slot, **kwargs):
    reward = kwargs["reward"]
    return reward[slot - 1][chosen_action - 1]


def reverse_reward_after_threshold(chosen_action, context, slot, **kwargs):
    reward = kwargs["reward"]
    iteration = kwargs.get("iteration", 0)
    threshold = kwargs.get("threshold", 0)
    if iteration > threshold:
        reward = [i[::-1] for i in reward]
    return reward[slot - 1][chosen_action - 1]
