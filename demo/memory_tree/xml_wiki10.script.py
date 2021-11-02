import os
import time
import numpy as np
#from IPython import embed

print("perform experiments on wiki10 (multilabel)")
leaf_example_multiplier = 2  #2
lr = 0.1
bits = 30
alpha = 0.1 #0.3
passes = 1 #4 #4
learn_at_leaf = True
#num_queries = 1  #does not really use
#hal_version = 1 #does not really use
use_oas = True
dream_at_update = 1 #0 #1
loss = "squared"
dream_repeats = 3
#Precision_at_K = 5

num_examples = 14146
max_num_labels = 30938

tree_node = int(num_examples/(np.log(num_examples)/np.log(2)*leaf_example_multiplier))
train_data = "wiki10_train.mat.mult_label.vw.txt"
test_data = "wiki10_test.mat.mult_label.vw.txt"
if os.path.exists(train_data) is not True:
        os.system("wget http://kalman.ml.cmu.edu/wen_datasets/{}".format(train_data))
if os.path.exists(test_data) is not True:
        os.system("wget http://kalman.ml.cmu.edu/wen_datasets/{}".format(test_data))

saved_model = "{}.vw".format(train_data)

print("## Training...")
start = time.time()
#train_data = 'tmp_rcv1x.vw.txt'
command_line = f"../../build/vowpalwabbit/vw {train_data} --memory_tree {tree_node} {'--learn_at_leaf' if learn_at_leaf else ''} --dream_at_update {dream_at_update}\
                --max_number_of_labels {max_num_labels} --dream_repeats {dream_repeats} {'--oas' if use_oas else ''} \
                --leaf_example_multiplier {leaf_example_multiplier} --alpha {alpha} -l {lr} -b {bits} -c --passes {passes} --loss_function {loss} --holdout_off -f {saved_model}"
os.system(command_line)
train_time = time.time() - start

print("## Testing...")
start = time.time()
os.system("../../build/vowpalwabbit/vw {} --oas {} -i {}".format(test_data, use_oas, saved_model))
test_time = time.time() - start
print("## train time {}, and test time {}".format(train_time, test_time))

