import argparse
import random
import sys
import time
import uuid

import rlclientlib as rlcl


class rl_sim:
    def __init__(self, args):
        self._options = args

    def loop(self):
        if (self.init() != True):
            return -1

        response = rlcl.ranking_response()

        round = 0
        while (True):
            try:
                p = self.pick_a_random_person()
                context_features = p.get_features()
                action_features = r"""("_multi": [ {"topic":"HerbGarden"}, {"topic":"MachineLearning"} ])"""
                context_json = self.create_context_json(context_features, action_features)
                req_id = str(uuid.uuid4())

                response = self._rl.choose_rank(req_id, context_json)
                choosen_action = response.get_choosen_action_id()
                reward = p.get_reward(self._actions[choosen_action])
                self._rl.report_outcome(req_id, reward)

                print('Round: {round}, Person: {person}, Action: {action}, Reward: {reward}'.format(round = round, person = p.id(), action = choosen_action, reward = reward))

                round = round + 1
                time.sleep(0.1)
            except rlcl.exception as ex:
                print('Something bad happened: {description}'.format(description = ex.get_error_msg()))
                continue

    def init(self):
        if (self.init_rl() != rlcl.error_code.success):
            return False

        self.init_people()
        return True

    def init_rl(self):
        try:
            config = self.load_config_from_json(self._options.json_config)
            self._rl = rlcl.live_model(config)
            self._rl.init()
            return rlcl.error_code.success

        except rlcl.exception as ex:
            print('Something bad happened: {description}'.format(description = ex.get_error_msg()))
            return -1

    def init_people(self):
        tp1 = {'HerbGarden': 0.002, "MachineLearning": 0.03 }
        tp2 = {'HerbGarden': 0.015, "MachineLearning": 0.05 }

        self._actions = ['HerbGarden', 'MachineLearning']
        self._people = [person('rnc', 'engineering', 'hiking', 'spock', tp1), person('mk', 'psychology', 'kids', '7of9', tp2)]

    def load_config_from_json(self, file_name):
        with open(file_name, 'r') as config_file:
            return rlcl.utility_config.create_from_json(config_file.read())

    def pick_a_random_person(self):
        return self._people[random.randint(0, len(self._people) - 1)]

    def create_context_json(self, context, action):
        return '{ ' + context + ', ' + action + ' }'

class person:
    def __init__(self, id, major, hobby, fav_char, p):
        self._id = id
        self._major = major
        self._hobby = hobby
        self._favorite_character = fav_char
        self._topic_click_probability = p

    def get_features(self):
        return r"""("User":{
            ("id":")""" + self._id + r"""(",)
            ("major":")""" + self._major + r"""(",)
            ("hobby":")""" + self._hobby + r"""(",)
            ("favorite_character":")""" + self._favorite_character + r"""
            ("})""";

    def get_reward(self, choosen_action):
        draw_uniform = random.uniform(0, 10000)
        norm_draw_val = draw_uniform / 10000.0
        click_prob = self._topic_click_probability[choosen_action]
        if (norm_draw_val <= click_prob):
            return 1.0
        else:
            return 0.0

    def id(self):
        return "id"

def process_cmd_line(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--json_config', help='client json config')
    return parser.parse_args(args)

def main(args):
    vm = process_cmd_line(args)
    sim = rl_sim(vm)
    sim.loop()

if __name__ == "__main__":
   main(sys.argv[1:])
