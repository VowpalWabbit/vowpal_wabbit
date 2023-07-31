def threeSlot_clothes_sunny_raining(**kwargs):
    iteration = kwargs.get("iteration", 0)
    # before iteration 500, it is sunny and after it is raining
    if iteration > 500:
        return [
            ["buttonupshirt", "highvis", "rainshirt"],
            ["formalpants", "rainpants", "shorts"],
            ["rainshoe", "formalshoes", "flipflops"],
        ]

    return [
        ["tshirt", "longshirt", "turtleneck"],
        ["workpants", "shorts", "formalpants"],
        ["formalshoes", "runners", "flipflops"],
    ]
