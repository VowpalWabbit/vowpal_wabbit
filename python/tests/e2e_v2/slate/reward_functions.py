def fixed_reward(chosen_action, context, slot, reward):
    return reward[slot - 1][chosen_action - 1]


def reverse_reward_after_threshold(
    chosen_action, context, slot, reward, iteration, threshold
):
    if iteration > threshold:
        reward = [i[::-1] for i in reward]
    return reward[slot - 1][chosen_action - 1]
