#!/usr/bin/env python3

import os
import time

# Parameters
k = 2456  # number of labels
kary_tree = 16
l = 1.0
passes = 3
b = 28
output_model = "rcv1x_model"

# Dataset filenames
train_data = "rcv1x_train.vw"
test_data = "rcv1x_test.vw"

print("Vowpal Wabbit PLT example on RCV1X-2K dataset with {} labels".format(k))

# Download dataset (source: http://manikvarma.org/downloads/XC/XMLRepository.html)
if not os.path.exists(train_data):
    os.system("wget http://www.cs.put.poznan.pl/mwydmuch/data/{}".format(train_data))
if not os.path.exists(test_data):
    os.system("wget http://www.cs.put.poznan.pl/mwydmuch/data/{}".format(test_data))

print("\nTraining\n")

print("\nTraining\n")
start = time.time()
train_cmd = "vw {} -c --plt {} --loss_function logistic --kary_tree {} -l {} --passes {} -b {} -f {} --holdout_off".format(
    train_data, k, kary_tree, l, passes, b, output_model
)
print(train_cmd)
os.system(train_cmd)
train_time = time.time() - start

print("\nTesting with probability threshold = 0.5 (default prediction mode)\n")
start = time.time()
test_threshold_cmd = "vw {} -i {} --loss_function logistic --threshold 0.5 -t".format(
    test_data, output_model
)
print(test_threshold_cmd)
os.system(test_threshold_cmd)
thr_test_time = time.time() - start

print("\nTesting with top-5 prediction\n")
start = time.time()
test_topk_cmd = "vw {} -i {} --loss_function logistic --top_k 5 -t".format(
    test_data, output_model
)
print(test_topk_cmd)
os.system(test_topk_cmd)
topk_test_time = time.time() - start

# print("\ntrain time (s) = {:.3f}".format(train_time))
print("threshold test time (s) = {:.3f}".format(thr_test_time))
print("top-5 test time (s) = {:.3f}".format(topk_test_time))
