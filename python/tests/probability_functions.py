def constant_probability(chosen_action, **kwargs):
    return 1


def even_probability(chosen_action, **kwargs):
    num_actions = kwargs["num_actions"]
    return 1 / num_actions
