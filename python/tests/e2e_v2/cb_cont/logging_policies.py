def constant_probability(chosen_action):
    return 1


def even_probability(chosen_action, num_actions):
    return round(1 / num_actions, 2)
