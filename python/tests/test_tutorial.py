from vowpalwabbit import pyvw

import pytest
import random

import os

# this test was adapted from this tutorial: https://vowpalwabbit.org/tutorials/cb_simulation.html

class Simulator:
    # VW tries to minimize loss/cost, therefore we will pass cost as -reward
    USER_LIKED_ARTICLE = -1.0
    USER_DISLIKED_ARTICLE = 0.0

    users = ['Tom', 'Anna']
    times_of_day = ['morning', 'afternoon']
    actions = ["politics", "sports", "music", "food", "finance", "health", "camping"]

    def __init__(self, debug_logfile=None, seed=10):
        random.seed(seed)

        if debug_logfile:
            self.debug_log = open(debug_logfile, 'w')
        else:
            self.debug_log = open(os.devnull,"w") 

        self.cost_sum = 0.
        self.ctr = []

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
    def to_vw_example_format(self, context, actions, cb_label = None):
        if cb_label is not None:
            chosen_action, cost, prob = cb_label
        example_string = ""
        example_string += "shared |User user={} time_of_day={}\n".format(context["user"], context["time_of_day"])
        for action in actions:
            if cb_label is not None and action == chosen_action:
                example_string += "0:{}:{} ".format(cost, prob)
            example_string += "|Action article={} \n".format(action)
        #Strip the last newline
        return example_string[:-1]

    def sample_custom_pmf(self, pmf):
        total = sum(pmf)
        scale = 1 / total
        pmf = [x * scale for x in pmf]
        draw = random.random()
        sum_prob = 0.0
        for index, prob in enumerate(pmf):
            sum_prob += prob
            if(sum_prob > draw):
                return index, prob

    def get_action(self, vw, context, actions):
        vw_text_example = self.to_vw_example_format(context,actions)
        pmf = vw.predict(vw_text_example)
        chosen_action_index, prob = self.sample_custom_pmf(pmf)
        return actions[chosen_action_index], prob

    def choose_user(self, users):
        return random.choice(users)

    def choose_time_of_day(self, times_of_day):
        return random.choice(times_of_day)

    def run_simulation(self, vw, num_iterations, users, times_of_day, actions, cost_function, do_learn = True, shift=1):
        for i in range(shift, shift+num_iterations):
            # 1. In each simulation choose a user
            user = self.choose_user(users)
            # 2. Choose time of day for a given user
            time_of_day = self.choose_time_of_day(times_of_day)

            # 3. Pass context to vw to get an action
            context = {'user': user, 'time_of_day': time_of_day}
            action, prob = self.get_action(vw, context, actions)

            # 4. Get cost of the action we chose
            cost = cost_function(context, action)
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

        return self.ctr

def _test_helper(vw_arg, num_iterations=3000, seed=10):
    vw = pyvw.vw(vw_arg)
    sim = Simulator(seed=seed)
    ctr = sim.run_simulation(vw, num_iterations, sim.users, sim.times_of_day, sim.actions, sim.get_cost)
    return ctr

def _test_helper_save_load(vw_arg, num_iterations=3000, seed=10):
    split = 1500
    before_save = num_iterations-split

    sim = Simulator(seed=seed)
    first_vw = pyvw.vw(vw_arg)
    # first chunk
    ctr = sim.run_simulation(first_vw, before_save, sim.users, sim.times_of_day, sim.actions, sim.get_cost)
    # save
    model_file = "test_save_load.vw"
    first_vw.save(model_file)
    # reload in another instance
    other_vw = pyvw.vw(f"-i {model_file}")
    # continue
    ctr = sim.run_simulation(other_vw, split, sim.users, sim.times_of_day, sim.actions, sim.get_cost, shift=before_save+1)

    return ctr

def test_with_interaction():
    import math

    ctr = _test_helper(vw_arg="--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5")
    without_save = ctr[-1]

    assert(without_save >= 0.70)

    ctr = _test_helper_save_load(vw_arg="--cb_explore_adf -q UA --quiet --epsilon 0.2 --random_seed 5")
    with_save = ctr[-1]

    assert(with_save >= 0.70)

    # both ctr's should be fairly equal except for the effect of vw seed
    assert(math.isclose(without_save, with_save, rel_tol=1e-2))

def test_without_interaction():
    ctr = _test_helper(vw_arg="--cb_explore_adf --quiet --epsilon 0.2 --random_seed 5")

    assert(ctr[-1] <= 0.49)
    assert(ctr[-1] >= 0.38)
