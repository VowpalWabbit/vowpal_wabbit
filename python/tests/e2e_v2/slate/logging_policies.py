def even_probability(chosen_action, **kwargs):
    num_actions = kwargs["num_action"]
    return round(1 / num_actions, 2)
