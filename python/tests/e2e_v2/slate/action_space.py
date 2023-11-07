def new_action_after_threshold(iteration, threshold, before, after):
    # before iteration 500, it is sunny and after it is raining
    if iteration > threshold:
        return after
    return before
