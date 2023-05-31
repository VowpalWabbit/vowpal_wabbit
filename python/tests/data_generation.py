import random

def constant_function(no_sample, constant, lower_bound, upper_bound):
    dataFile = f"constant_func_{no_sample}_{constant}_{upper_bound}_{lower_bound}.txt"
    with open(dataFile, "w") as f:
        random.seed(10)
        for _ in range(no_sample):
            x = random.uniform(lower_bound, upper_bound)
            f.write(f"{constant} |f x:{x}\n")
    return dataFile
