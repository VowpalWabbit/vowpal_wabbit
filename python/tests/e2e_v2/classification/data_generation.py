import os, random
from test_helper import get_function_object


script_directory = os.path.dirname(os.path.realpath(__file__))
random.seed(10)


def generate_classification_data(
    num_example,
    num_classes,
    num_features,
    classify_func,
    bounds=None,
    seed=random.randint(0, 100),
):
    random.seed(seed)
    dataFile = f"classification_{num_classes}_{num_features}_{num_example}.txt"
    classify_func_obj = get_function_object(
        "classification.classification_functions", classify_func["name"]
    )
    if not bounds:
        bounds = [[0, 1] for _ in range(num_features)]
    with open(os.path.join(script_directory, dataFile), "w") as f:
        for _ in range(num_example):
            x = [
                random.uniform(bounds[index][0], bounds[index][1])
                for index in range(num_features)
            ]
            y = classify_func_obj(x, **classify_func["params"])
            f.write(
                f"{y} |f {' '.join([f'x{i}:{x[i]}' for i in range(num_features)])}\n"
            )
    return os.path.join(script_directory, dataFile)
