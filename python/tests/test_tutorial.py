from vowpalwabbit import pyvw
import DistributionallyRobustUnitTestData as dro

import collections
import os
import random

import os

# this test was adapted from this tutorial: https://vowpalwabbit.org/tutorials/cb_simulation.html

class Simulator:
    # VW tries to minimize loss/cost, therefore we will pass cost as -reward
    USER_LIKED_ARTICLE = -1.0
    USER_DISLIKED_ARTICLE = 0.0

    users = ['Tom', 'Anna']
    times_of_day = ['morning', 'afternoon']
    # actions = ["politics", "sports", "music", "food", "finance", "health", "camping"]
    actions = ["politics", "sports", "music"]

    def __init__(self, debug_logfile=None, seed=10, has_automl=False):
        random.seed(seed)

        self.has_aml = has_automl

        if debug_logfile:
            self.debug_log = open(debug_logfile, 'w')
        else:
            self.debug_log = open(os.devnull,"w") 

        self.cost_sum = 0.
        self.ctr = []

        self.other_index_count = collections.Counter()
        self.index_count = collections.Counter()

        self.count_1 = collections.Counter()
        self.count_2 = collections.Counter()

        self.ocrl = None
        self.ocrl2 = None

        if has_automl:
            self.ocrl = dro.OnlineDRO.OnlineCressieReadLB(alpha=0.05, tau=0.999)
            self.ocrl2 = dro.OnlineDRO.OnlineCressieReadLB(alpha=0.05, tau=0.999)


    def get_cost(self, context, action):
        if context['user'] == "Tom":
            if context['time_of_day'] == "morning" and action == 'politics':
                return self.USER_LIKED_ARTICLE
            elif context['time_of_day'] == "afternoon" and action == 'music':
                return self.USER_LIKED_ARTICLE
            else:
                return self.USER_DISLIKED_ARTICLE
        elif context['user'] == "Anna":
            if context['time_of_day'] == "morning" and action == 'sports':
                return self.USER_LIKED_ARTICLE
            elif context['time_of_day'] == "afternoon" and action == 'politics':
                return self.USER_LIKED_ARTICLE
            else:
                return self.USER_DISLIKED_ARTICLE

    # This function modifies (context, action, cost, probability) to VW friendly format
    def to_vw_example_format(self, context, actions, cb_label = None, ns1='U', ns2='A'):
        if ns1 == ns2:
            raise("not allowed to have same namespace")

        if cb_label is not None:
            chosen_action, cost, prob = cb_label
            self.other_index_count.update(((chosen_action,prob),))
        example_string = ""
        example_string += f'shared |{ns1}ser user={context["user"]} time_of_day={context["time_of_day"]}\n'
        for action in actions:
            if cb_label is not None and action == chosen_action:
                example_string += "0:{}:{} ".format(cost, prob)
            example_string += f'|{ns2}ction article={action} \n'
        #Strip the last newline
        print("example:::"+example_string[:-1], file=self.debug_log)
        return example_string[:-1]

    def sample_custom_pmf(self, pmf):
        total = sum(pmf)
        scale = 1 / total
        pmf = [x * scale for x in pmf]
        draw = random.random()
        print("draw="+str(draw), file=self.debug_log)
        sum_prob = 0.0
        for index, prob in enumerate(pmf):
            sum_prob += prob
            if(sum_prob > draw):
                print(f"index:{index} prob:{prob}, sum_prob="+str(sum_prob), file=self.debug_log)
                return index, prob

    def get_action(self, vw, context, actions):
        vw_text_example = self.to_vw_example_format(context,actions)
        pmf = vw.predict(vw_text_example)
        print("pmf:" +str(pmf), file=self.debug_log)
        chosen_action_index, prob = self.sample_custom_pmf(pmf)
        self.index_count.update(((chosen_action_index,prob),))
        print(str(chosen_action_index)+" "+str(prob), file=self.debug_log)
        return actions[chosen_action_index], prob

    def choose_user(self, users):
        return random.choice(users)

    def choose_time_of_day(self, times_of_day):
        return random.choice(times_of_day)

    def process_metrics(self, i, metrics):
        vw_b = metrics["bound_1"]
        vw_b_2 = metrics["bound_2"]
        w = metrics["w_1"]
        r = metrics["r_1"]
        w2 = metrics["w_2"]
        r2 = metrics["r_2"]
        assert(r == r2)
        self.ocrl.update(1, w, r)
        self.count_1.update(((w,r),))
        self.ocrl.recomputeduals()
        self.ocrl2.update(1, w2, r2)
        self.count_2.update(((w2,r2),))
        self.ocrl2.recomputeduals()

        if False:
            print(str(i)+":"+str(vw_b_2)+", "+str(vw_b))
            print("ips"+str(i)+":"+str(metrics["ips_2"])+", "+str(metrics["ips_1"]))
            # only works for no interactions - hardcoded
            # compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
            assert float(vw_b_2) <= float(metrics["ips_1"]) , f"{vw_b_2} is higher than {metrics['ips_1']} at {i}"
            # assert (i < 30)

        if metrics["test_county"] % 500 == 0: # or metrics["test_county"] == 117:
            print("no interactions: lb: " + str(vw_b), file=self.debug_log)
            print("python: lb:" + str(self.ocrl.duals[0][0]), file=self.debug_log)
            print("interactions: lb: " + str(vw_b_2), file=self.debug_log)
            print("python: lb: " + str(self.ocrl2.duals[0][0]), file=self.debug_log)
            print("no interactions: ips: " + str(metrics["ips_1"]), file=self.debug_log)
            print("interactions: ips: " + str(metrics["ips_2"]), file=self.debug_log)
            print("num examples: " + str(metrics["test_county"]), file=self.debug_log)
            print(f"w{w}", file=self.debug_log)
            print(f"r{r}", file=self.debug_log)
            print(file=self.debug_log)
        assert(vw_b >= 0)
        assert(vw_b_2 >= 0)

    def run_simulation(self, vw, num_iterations, users, times_of_day, actions, cost_function, do_learn = True, shift=1):
        for i in range(shift, shift+num_iterations):
            print("", file=self.debug_log)
            print("id:"+str(i),file=self.debug_log)
            # 1. In each simulation choose a user
            user = self.choose_user(users)
            # 2. Choose time of day for a given user
            time_of_day = self.choose_time_of_day(times_of_day)

            # 3. Pass context to vw to get an action
            context = {'user': user, 'time_of_day': time_of_day}
            action, prob = self.get_action(vw, context, actions)

            # 4. Get cost of the action we chose
            cost = cost_function(context, action)
            print("cost="+str(cost), file=self.debug_log)
            self.cost_sum += cost

            if do_learn:
                # 5. Inform VW of what happened so we can learn from it
                # if (cost == 0):
                #     print(actions.index(action))
                vw_format = vw.parse(self.to_vw_example_format(context, actions, (action, cost, prob)),pyvw.vw.lContextualBandit)
                # 6. Learn
                vw.learn(vw_format)
                # 7. Let VW know you're done with these objects
                vw.finish_example(vw_format)

            # We negate this so that on the plot instead of minimizing cost, we are maximizing reward
            self.ctr.append(-1*self.cost_sum/i)

            if self.has_aml and i % 1 == 0:
                metrics = vw.get_learner_metrics()
                self.process_metrics(i, metrics)

        if self.has_aml and num_iterations + shift >= 2000: # fix: hardcoded 2000 bad
            print("counter1:"+str(self.count_1), file=self.debug_log)
            print("counter2:"+str(self.count_2), file=self.debug_log)
            print("indexcount:"+str(self.index_count), file=self.debug_log)
            print("ohterindexcount:"+str(self.other_index_count), file=self.debug_log)

            self.debug_log.close() # or also fix this

        return self.ctr

def _test_helper(vw_arg: str, num_iterations=2000, seed=10, has_automl=False, log_filename=None):
    vw = pyvw.vw(arg_str=vw_arg)
    has_aml = "test_red" in vw.get_enabled_reductions()
    sim = Simulator(seed=seed, has_automl=has_aml, debug_logfile=log_filename)
    ctr = sim.run_simulation(vw, num_iterations, sim.users, sim.times_of_day, sim.actions, sim.get_cost)
    vw.save("readable.vw")
    vw.finish()
    return ctr

def _test_helper_save_load(vw_arg: str, num_iterations=2000, seed=10, has_automl=False, log_filename=None):
    split = 1500
    before_save = num_iterations-split

    first_vw = pyvw.vw(arg_str=vw_arg)
    has_aml = "test_red" in first_vw.get_enabled_reductions()
    sim = Simulator(seed=seed, has_automl=has_aml, debug_logfile=log_filename)
    # first chunk
    ctr = sim.run_simulation(first_vw, before_save, sim.users, sim.times_of_day, sim.actions, sim.get_cost)
    # save
    model_file = "test_save_load.vw"
    first_vw.save(model_file)
    first_vw.finish()
    # reload in another instance
    other_vw = pyvw.vw(f"-i {model_file} {vw_arg}") # todo remove vw_arg from here
    # continue
    ctr = sim.run_simulation(other_vw, split, sim.users, sim.times_of_day, sim.actions, sim.get_cost, shift=before_save+1)

    return ctr

def test_with_interaction():
    import math

    ctr = _test_helper(vw_arg="--invert_hash readable.vw --cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5")
    without_save = ctr[-1]

    assert(without_save >= 0.70)

    ctr = _test_helper_save_load(vw_arg="--save_resume --cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5")
    with_save = ctr[-1]

    assert(with_save >= 0.70)

    # both ctr's should be fairly equal except for the effect of vw seed
    assert(math.isclose(without_save, with_save, rel_tol=1e-2))

def test_without_interaction():
    ctr = _test_helper(vw_arg="--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5")

    print(ctr[-1])
    assert(ctr[-1] <= 0.49)
    assert(ctr[-1] >= 0.38)

# set test_red to 1 to return pred of with interaction
# set test_red to 0 to return pred of no interaction
def test_custom_reduction(config=0, sim_saveload=False):
    # 10281881982--audit --invert_hash
    args = f"--invert_hash readable.vw --test_red {str(config)} --cb_explore_adf -q AA --quiet --epsilon 0.2 --random_seed 5 --extra_metrics metrics.json"
    if sim_saveload:
        ctr = _test_helper_save_load(vw_arg=f"--save_resume {args}", log_filename=f"custom_reduc_{str(config)}.txt")
    else:
        ctr = _test_helper(vw_arg=args, log_filename=f"custom_reduc_{str(config)}.txt")

    print("custom reduction - "+str(config))
    print(ctr[-1])

    # if config == 0: # starting champ is no interactions
    #     # assert(ctr[-1] > 0.35) # without rotation
    #     assert(ctr[-1] > 0.70) # with champ rotation
    # elif config == 1: # starting champ is with interactions
    #     assert(ctr[-1] > 0.75)
    # else:
    #     assert(false)

# good for attaching debugger
print(f"pid: {os.getpid()}\n")

def print_stars():
    print()
    [print("****************************") for _ in range(5)]
    print()

# test_with_interaction()
# test_without_interaction()

# print_stars()

with_interaction = 1
without_interaction = 0
# print("pred WITH interaction ******")
test_custom_reduction(config=with_interaction)

# print_stars()

# print("pred WITHOUT interaction ******")
test_custom_reduction(config=without_interaction)
