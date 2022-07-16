#!/usr/bin/env python3

import os
import time

# Parameters
k = 30938  # number of labels
kary_tree = 16
l = 1.0
passes = 3
b = 30
output_model = "wiki10_model"

# Dataset filenames
train_data = "wiki10_train.vw"
test_data = "wiki10_test.vw"

print("Vowpal Wabbit PLT example on Wiki10-31K dataset with {} labels".format(k))

# Download dataset (source: http://manikvarma.org/downloads/XC/XMLRepository.html)
if not os.path.exists(train_data):
    os.system("wget http://www.cs.put.poznan.pl/mwydmuch/data/{}".format(train_data))
if not os.path.exists(test_data):
    os.system("wget http://www.cs.put.poznan.pl/mwydmuch/data/{}".format(test_data))

print("\nTraining\n")

start = time.time()
os.system(
    "vw {} -c --plt {} --kary_tree {} -l {} --passes {} -b {} -f {} --holdout_off".format(
        train_data, k, kary_tree, l, passes, b, output_model
    )
)
train_time = time.time() - start

print("\nTesting with probability threshold = 0.5 (default prediction mode)\n")
start = time.time()
os.system("vw {} -i {} --threshold 0.5 -t".format(test_data, output_model))
thr_test_time = time.time() - start

print("\nTesting with top-5 prediction\n")
start = time.time()
os.system("vw {} -i {} --top_k 5 -t".format(test_data, output_model))
topk_test_time = time.time() - start

print("\ntrain time (s) = {:.3f}".format(train_time))
print("threshold test time (s) = {:.3f}".format(thr_test_time))
print("top-5 test time (s) = {:.3f}".format(topk_test_time))
