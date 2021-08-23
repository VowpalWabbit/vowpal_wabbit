from vowpalwabbit import pyvw

import DistributionallyRobustUnitTestData as dro
import random
import collections

# this test was adapted from this tutorial: https://vowpalwabbit.org/tutorials/cb_simulation.html

# VW tries to minimize loss/cost, therefore we will pass cost as -reward
USER_LIKED_ARTICLE = -1.0
USER_DISLIKED_ARTICLE = 0.0

debug_log = None

def get_cost(context,action):
    if context['user'] == "Tom":
        if context['time_of_day'] == "morning" and action == 'politics':
            return USER_LIKED_ARTICLE
        elif context['time_of_day'] == "afternoon" and action == 'music':
            return USER_LIKED_ARTICLE
        else:
            return USER_DISLIKED_ARTICLE
    elif context['user'] == "Anna":
        if context['time_of_day'] == "morning" and action == 'sports':
            return USER_LIKED_ARTICLE
        elif context['time_of_day'] == "afternoon" and action == 'politics':
            return USER_LIKED_ARTICLE
        else:
            return USER_DISLIKED_ARTICLE

other_index_count = collections.Counter()

# This function modifies (context, action, cost, probability) to VW friendly format
def to_vw_example_format(context, actions, cb_label = None):
    global other_index_count
    if cb_label is not None:
        chosen_action, cost, prob = cb_label
        other_index_count.update(((chosen_action,prob),))
    example_string = ""
    example_string += "shared |Gser user={} time_of_day={}\n".format(context["user"], context["time_of_day"])
    for action in actions:
        if cb_label is not None and action == chosen_action:
            example_string += "0:{}:{} ".format(cost, prob)
        example_string += "|Tction article={} \n".format(action)
    #Strip the last newline
    print("example:::"+example_string[:-1], file=debug_log)
    return example_string[:-1]


def sample_custom_pmf(pmf):
    total = sum(pmf)
    scale = 1 / total
    pmf = [x * scale for x in pmf]
    draw = random.random()
    print("draw="+str(draw), file=debug_log)
    sum_prob = 0.0
    for index, prob in enumerate(pmf):
        sum_prob += prob
        if(sum_prob > draw):
            # raise("blabhlabh")
            print(f"index:{index} prob:{prob}, sum_prob="+str(sum_prob), file=debug_log)
            return index, prob
    raise("impossible to arrive here")

index_count = collections.Counter()

def get_action(vw, context, actions):
    global index_count
    vw_text_example = to_vw_example_format(context,actions)
    pmf = vw.predict(vw_text_example)
    print("pmf:" +str(pmf), file=debug_log)
    chosen_action_index, prob = sample_custom_pmf(pmf)
    index_count.update(((chosen_action_index,prob),))
    print(str(chosen_action_index)+" "+str(prob), file=debug_log)
    return actions[chosen_action_index], prob

users = ['Tom', 'Anna']
times_of_day = ['morning', 'afternoon']
actions = ["politics", "sports", "music"]

def choose_user(users):
    return random.choice(users)

def choose_time_of_day(times_of_day):
    return random.choice(times_of_day)

cost_sum = 0.
ctr = []
ocrl = None
ocrl2 = None
count_1 = None
count_2 = None

def clear_test_state(log_filename = None, has_automl = False):
    global cost_sum, ctr
    global ocrl, ocrl2
    global count_1, count_2
    global index_count, other_index_count
    global debug_log

    if has_automl:
        ocrl = dro.OnlineDRO.OnlineCressieReadLB(alpha=0.05, tau=0.999)
        ocrl2 = dro.OnlineDRO.OnlineCressieReadLB(alpha=0.05, tau=0.999)

    count_1 = collections.Counter()
    count_2 = collections.Counter()
    index_count.clear()
    other_index_count.clear()

    cost_sum = 0.
    ctr = []

    random.seed(10)

    debug_log = open(log_filename, 'w')


def run_simulation(vw, num_iterations, users, times_of_day, actions, cost_function, do_learn = True, shift=1):
    global cost_sum, ctr
    global ocrl, ocrl2
    global count_1, count_2
    global index_count, other_index_count
    # keep track if simulation is under new reduction
    # this is only used to obtain more specific metrics
    has_aml = "test_red" in vw.get_enabled_reductions()

    for i in range(shift, shift+num_iterations):
        print("", file=debug_log)
        print("id:"+str(i),file=debug_log)
        # 1. In each simulation choose a user
        user = choose_user(users)
        # 2. Choose time of day for a given user
        time_of_day = choose_time_of_day(times_of_day)

        # 3. Pass context to vw to get an action
        context = {'user': user, 'time_of_day': time_of_day}
        action, prob = get_action(vw, context, actions)

        # 4. Get cost of the action we chose
        cost = cost_function(context, action)
        print("cost="+str(cost), file=debug_log)
        cost_sum += cost

        if do_learn:
            # 5. Inform VW of what happened so we can learn from it
            # if (cost == 0):
            #     print(actions.index(action))
            vw_format = vw.parse(to_vw_example_format(context, actions, (action, cost, prob)),pyvw.vw.lContextualBandit)
            # 6. Learn
            vw.learn(vw_format)
            # 7. Let VW know you're done with these objects
            vw.finish_example(vw_format)

        # We negate this so that on the plot instead of minimizing cost, we are maximizing reward
        ctr.append(-1*cost_sum/i)

        if has_aml and i % 1 == 0:
            metrics = vw.get_learner_metrics()
            vw_b = metrics["bound_1"]
            vw_b_2 = metrics["bound_2"]
            w = metrics["w_1"]
            r = metrics["r_1"]
            w2 = metrics["w_2"]
            r2 = metrics["r_2"]
            assert(r == r2)
            ocrl.update(1, w, r)
            count_1.update(((w,r),))
            ocrl.recomputeduals()
            ocrl2.update(1, w2, r2)
            count_2.update(((w2,r2),))
            ocrl2.recomputeduals()
            if metrics["test_county"] % 500 == 0: # or metrics["test_county"] == 117:
                print("no interactions: lb: " + str(vw_b), file=debug_log)
                print("python: lb:" + str(ocrl.duals[0][0]), file=debug_log)
                print("interactions: lb: " + str(vw_b_2), file=debug_log)
                print("python: lb: " + str(ocrl2.duals[0][0]), file=debug_log)
                print("no interactions: ips: " + str(metrics["ips_1"]), file=debug_log)
                print("interactions: ips: " + str(metrics["ips_2"]), file=debug_log)
                print("num examples: " + str(metrics["test_county"]), file=debug_log)
                print(f"w{w}", file=debug_log)
                print(f"r{r}", file=debug_log)
                print(file=debug_log)
            assert(vw_b >= 0)
            assert(vw_b_2 >= 0)

    if num_iterations + shift >= 2000:
        print("counter1:"+str(count_1), file=debug_log)
        print("counter2:"+str(count_2), file=debug_log)
        print("indexcount:"+str(index_count), file=debug_log)
        print("ohterindexcount:"+str(other_index_count), file=debug_log)

        debug_log.close()

    return ctr

def test_with_interaction(num_iterations=2000):
    clear_test_state(log_filename="with_inter.txt")
    vw = pyvw.vw("--save_resume --random_seed 5 --cb_explore_adf -q GT --quiet --epsilon 0.2")
    ctr = run_simulation(vw, num_iterations, users, times_of_day, actions, get_cost)

    print("with interaction")
    print(ctr[-1])
    # assert(ctr[-1] == 0.765)

def test_without_interaction(num_iterations=2000):
    clear_test_state(log_filename="without_inter.txt")
    vw = pyvw.vw("--save_resume --random_seed 5 --cb_explore_adf --quiet --epsilon 0.2")
    num_iterations = 2000
    ctr = run_simulation(vw, num_iterations, users, times_of_day, actions, get_cost)

    print("without interaction")
    print(ctr[-1])
    # assert(ctr[-1] == 0.4035)

# set test_red to 1 to return pred of with interaction
# set test_red to 0 to return pred of no interaction
def test_custom_reduction(config=0, num_iterations=2000, sim_saveload=True):
    clear_test_state(has_automl=True, log_filename=f"custom_reduc_{str(config)}.txt")
    vw_args = f"--save_resume --random_seed 5 --test_red {str(config)} --cb_explore_adf --quiet --epsilon 0.2 --extra_metrics metrics.json"
    vw = pyvw.vw(vw_args)

    if sim_saveload:
        # first 500
        ctr = run_simulation(vw, num_iterations-1500, users, times_of_day, actions, get_cost)
        # save
        model_file = "prueba.vw"
        vw.save(model_file)
        # reload in another instance
        other_vw = pyvw.vw(f"-i {model_file} {vw_args}")
        # continue
        ctr = run_simulation(other_vw, 1500, users, times_of_day, actions, get_cost, shift=501)
    else:
        ctr = run_simulation(vw, num_iterations, users, times_of_day, actions, get_cost)

    print("custom reduction - "+str(config))
    print(ctr[-1])
    # if config == 0:
    #     assert(ctr[-1] == 0.371)
    # elif config == 1:
    #     assert(ctr[-1] == 0.766)
    # else:
    #     assert(false)

# good for attaching debugger
import os
print(f"pid: {os.getpid()}\n")

def print_stars():
    print()
    [print("****************************") for _ in range(5)]
    print()

test_with_interaction()
test_without_interaction()

print_stars()

with_interaction = 1
without_interaction = 0
print("pred WITH interaction ******")
test_custom_reduction(config=with_interaction)

print_stars()

print("pred WITHOUT interaction ******")
test_custom_reduction(config=without_interaction)
