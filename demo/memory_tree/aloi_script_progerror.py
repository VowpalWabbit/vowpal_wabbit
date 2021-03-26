import os
import time
import numpy as np
#from IPython import embed


#for shot in available_shots.iterkeys():
print("## perform experiments on aloi ##")
num_of_classes = 1000
leaf_example_multiplier = 10
shots = 100
lr = 0.001
bits = 29
alpha = 0.1 #0.3
passes = 1  #3 #5
use_oas = False
dream_at_update = 0
learn_at_leaf = True #turn on leaf at leaf actually works better
loss = "squared"
dream_repeats = 20 #3
online = True
#random_seed = 4000

tree_node = int(2*passes*(num_of_classes*shots/(np.log(num_of_classes*shots)/np.log(2)*leaf_example_multiplier)));

train_data = "aloi_train.vw"
test_data = "aloi_test.vw"
if os.path.exists(train_data) is not True:
        os.system("wget http://kalman.ml.cmu.edu/wen_datasets/{}".format(train_data))
if os.path.exists(test_data) is not True:
        os.system("wget http://kalman.ml.cmu.edu/wen_datasets/{}".format(test_data))


saved_model = "{}.vw".format(train_data)

print("## Training...")
start = time.time()
command_line = f"../../build/vowpalwabbit/vw -d {train_data} --memory_tree {tree_node} {'--learn_at_leaf' if learn_at_leaf else ''} --max_number_of_labels {num_of_classes} --dream_at_update {dream_at_update} \
                   --dream_repeats {dream_repeats} {'--oas' if use_oas else ''} {'--online' if online else ''} \
                --leaf_example_multiplier {leaf_example_multiplier} --alpha {alpha} -l {lr} -b {bits} -c --passes {passes} --loss_function {loss} --holdout_off -f {saved_model}"
os.system(command_line)
train_time = time.time() - start

    #test:
#print "## Testing..."
#start = time.time();
#os.system(".././vw {} -i {}".format(test_data, saved_model))

#test_time = time.time() - start

print("## train time {}, and test time {}".format(train_time, test_time))





