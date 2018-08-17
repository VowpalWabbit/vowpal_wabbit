import argparse
import random
import sys
import time
import uuid

import rl_client

class my_error_callback(rl_client.error_callback):
  def on_error(self, error_code, error_message):
    print("Background error:")
    print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rl_client.create_config_from_json(config_file.read())

class person:
    def __init__(self, id, major, hobby, fav_char, p):
        self._id = id
        self._major = major
        self._hobby = hobby
        self._favorite_character = fav_char
        self._topic_click_probability = p

    def get_features(self):
        return '"User":{{"id":"{}","major":"{}","hobby":"{}","favorite_character":"{}"}}'.format(self._id, self._major, self._hobby, self._favorite_character)

    def get_outcome(self, chosen_action):
        draw_uniform = random.uniform(0, 10000)
        norm_draw_val = draw_uniform / 10000.0
        click_prob = self._topic_click_probability[chosen_action]
        if (norm_draw_val <= click_prob):
            return 1.0
        else:
            return 0.0

class rl_sim:
    def __init__(self, args):
        self._options = args

        self.config = load_config_from_json(self._options.json_config)
        self._rl = rl_client.live_model(self.config, my_error_callback())
        self._rl.init()

        tp1 = {'HerbGarden': 0.6, "MachineLearning": 0.4 }
        tp2 = {'HerbGarden': 0.2, "MachineLearning": 0.8 }

        self._actions = ['HerbGarden', 'MachineLearning']
        self._people = [
            person('rnc', 'engineering', 'hiking', 'spock', tp1),
            person('mk', 'psychology', 'kids', '7of9', tp2)]

    def loop(self):
        round = 0
        while (True):
            try:
                p = self.pick_a_random_person()
                context_features = p.get_features()
                action_features = '"_multi": [ {"topic":"HerbGarden"}, {"topic":"MachineLearning"} ]'
                context_json = self.create_context_json(context_features, action_features)
                req_id = str(uuid.uuid4())

                model_id, chosen_action_id, action_probabilities = self._rl.choose_rank(req_id, context_json)
                outcome = p.get_outcome(self._actions[chosen_action_id])
                self._rl.report_outcome(req_id, outcome)

                print('Round: {}, Person: {}, Action: {}, Outcome: {}'
                    .format(round, p._id, chosen_action_id, outcome))

                round = round + 1
                time.sleep(0.1)
            except Exception as e:
                print(e)
                time.sleep(2)
                continue

    def pick_a_random_person(self):
        return self._people[random.randint(0, len(self._people) - 1)]

    def create_context_json(self, context, action):
        return '{{ {}, {} }}'.format(context, action)

def process_cmd_line(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--json_config', help='client json config', required=True)
    return parser.parse_args(args)

def main(args):
    vm = process_cmd_line(args)
    sim = rl_sim(vm)
    sim.loop()

if __name__ == "__main__":
   main(sys.argv[1:])
