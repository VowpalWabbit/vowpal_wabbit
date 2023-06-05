#!/usr/bin/env python3

import os
import time

# This is demo example that demonstrates usage of PLT reduction on few popular multilabel datasets.

# Select dataset
dataset = "eurlex"  # should be in ["mediamill_exp1", "eurlex", "rcv1x", "wiki10", "amazonCat"]

# Select reduction
reduction = "plt"  # should be in ["plt", "multilabel_oaa"]


# Dataset filenames and model output file
train_data = f"{dataset}_train.vw"
test_data = f"{dataset}_test.vw"
output_model = f"{dataset}_{reduction}_model"

# Parameters
kary_tree = 16  # arity of the tree,
# higher values usually give better results, but increase training time

passes = 5  # number of passes over the dataset,
# for some datasets you might want to change number of passes

l = 0.5  # learning rate

other_training_params = "--holdout_off"  # because these datasets have many rare labels,
# disabling the holdout set improves the final performance

# dict with params for different datasets (k and b)
params_dict = {
    "mediamill_exp1": (101, 25),
    "rcv1x": (2456, 28),
    "eurlex": (3993, 28),
    "wiki10": (30938, 30),
    "amazonCat": (13330, 30),
}
if dataset in params_dict:
    k, b = params_dict[dataset]
else:
    print(f"Dataset {dataset} is not supported by this demo.")

# Download dataset (source: http://manikvarma.org/downloads/XC/XMLRepository.html)
# Datasets were transformed to VW's format,
# and features values were normalized (this helps with performance).
if not os.path.exists(train_data):
    print("Downloading train dataset:")
    os.system("wget http://www.cs.put.poznan.pl/mwydmuch/data/{}".format(train_data))

if not os.path.exists(test_data):
    print("Downloading test dataset:")
    os.system("wget http://www.cs.put.poznan.pl/mwydmuch/data/{}".format(test_data))


print(f"\nTraining Vowpal Wabbit {reduction} on {dataset} dataset:\n")
start = time.time()
train_cmd = f"vw {train_data} -c --{reduction} {k} --loss_function logistic -l {l} --passes {passes} -b {b} -f {output_model} {other_training_params}"
if reduction == "plt":
    train_cmd += f" --kary_tree {kary_tree}"
print(train_cmd)
os.system(train_cmd)
train_time = time.time() - start
print(f"train time (s) = {train_time:.3f}")


print("\nTesting with probability threshold = 0.5 (default prediction mode)\n")
start = time.time()
test_threshold_cmd = f"vw {test_data} -i {output_model} --loss_function logistic -t"
if reduction == "plt":
    test_threshold_cmd += " --threshold 0.5"
print(test_threshold_cmd)
os.system(test_threshold_cmd)
thr_test_time = time.time() - start
print(f"threshold test time (s) = {thr_test_time:.3f}")


if reduction == "plt":
    print("\nTesting with top-5 prediction\n")
    start = time.time()
    test_topk_cmd = (
        f"vw {test_data} -i {output_model} --loss_function logistic --top_k 5 -t"
    )
    print(test_topk_cmd)
    os.system(test_topk_cmd)
    topk_test_time = time.time() - start
    print(f"top-5 test time (s) = {topk_test_time:.3f}")
