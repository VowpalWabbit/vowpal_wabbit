def new_action_after_threshold(**kwargs):
    iteration = kwargs.get("iteration", 0)
    threshold = kwargs.get("threshold", 0)
    # before iteration 500, it is sunny and after it is raining
    if iteration > threshold:
        return kwargs["after"]
    return kwargs["before"]
