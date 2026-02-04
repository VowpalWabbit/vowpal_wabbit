import os
import time
import numpy as np

# available_shots = {'three':3, "one":1}
available_shots = {"three": 3}

for shot, shots in available_shots.items():
    print("## perform experiments on {}-shot wikipara-10K ##".format(shot))
    # shots = available_shots[shot]
    num_of_classes = 10000
    leaf_example_multiplier = 10  # 2
    lr = 0.1
    bits = 29  # 30
    passes = 1  # 2
    # hal_version = 1
    # num_queries = 1 #int(np.log(shots*num_of_classes)/np.log(2.))
    alpha = 0.1
    learn_at_leaf = False
    use_oas = False
    dream_at_update = 1
    dream_repeats = 15
    loss = "squared"
    online = True
    sort_feature = True

    tree_node = int(
        2
        * passes
        * (
            num_of_classes
            * shots
            / (np.log(num_of_classes * shots) / np.log(2) * leaf_example_multiplier)
        )
    )

    train_data = "paradata10000_{}_shot.vw.train".format(shot)
    test_data = "paradata10000_{}_shot.vw.test".format(shot)
    if os.path.exists(train_data) is not True:
        os.system("wget http://kalman.ml.cmu.edu/wen_datasets/{}".format(train_data))
    if os.path.exists(test_data) is not True:
        os.system("wget http://kalman.ml.cmu.edu/wen_datasets/{}".format(test_data))

    saved_model = "{}.vw".format(train_data)

    print("## Training...")
    start = time.time()
    command_line = f"../../build/vowpalwabbit/cli/vw -d {train_data} --memory_tree {tree_node} {'--learn_at_leaf' if learn_at_leaf else ''} --max_number_of_labels {num_of_classes} {'--oas' if use_oas else ''} {'--online' if online else ''} --dream_at_update {dream_at_update}\
              --leaf_example_multiplier {leaf_example_multiplier} --dream_repeats {dream_repeats} {'--sort_features' if sort_feature else ''}\
        --alpha {alpha} -l {lr} -b {bits} -c --passes {passes} --loss_function {loss} --holdout_off -f {saved_model}"
    os.system(command_line)
    train_time = time.time() - start

    # test:
    # print "## Testing..."
    # start = time.time();
    # os.system(".././vw {} -i {}".format(test_data, saved_model))

    # test_time = time.time() - start

    # print "## train time {}, and test time {}".format(train_time, test_time)
