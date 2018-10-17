import argparse
import random
import sys
import time

import rl_client

class my_error_callback(rl_client.error_callback):
  def on_error(self, error_code, error_message):
    print("Background error:")
    print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rl_client.create_config_from_json(config_file.read())

class person:
    def __init__(self, id, major, hobby, fav_char, prob):
        self._id = id
        self._major = major
        self._hobby = hobby
        self._favorite_character = fav_char
        self._topic_click_probability = prob

    def get_features(self):
        return '"User":{{"id":"{}","major":"{}","hobby":"{}","favorite_character":"{}"}}'.format(self._id, self._major, self._hobby, self._favorite_character)

    def get_outcome(self, chosen_action):
        norm_draw_val = random.random()
        click_prob = self._topic_click_probability[chosen_action]
        if (norm_draw_val <= click_prob):
            return 1.0
        else:
            return 0.0

class rl_sim:
    def __init__(self, args):
        self._options = args

        self.config = load_config_from_json(self._options.json_config)
        self._rl_client = rl_client.live_model(self.config, my_error_callback())
        self._rl_client.init()

        tp1 = {'HerbGarden': 0.3, "MachineLearning": 0.2 }
        tp2 = {'HerbGarden': 0.1, "MachineLearning": 0.4 }

        self._actions = ['HerbGarden', 'MachineLearning']
        self._people = [
            person('rnc', 'engineering', 'hiking', 'spock', tp1),
            person('mk', 'psychology', 'kids', '7of9', tp2)]

    def loop(self):
        round = 0
        while (True):
            try:
                # random.sample() returns a list
                person = random.sample(self._people, 1)[0]
                
                # create context
                shared_features = person.get_features()
                action_features = '"_multi": [ {"topic":"HerbGarden"}, {"topic":"MachineLearning"} ]'
                context = '{{ {}, {} }}'.format(shared_features, action_features)

                model_id, chosen_action_id, actions_probabilities, event_id = self._rl_client.choose_rank(context)
                
                outcome = person.get_outcome(self._actions[chosen_action_id])
                self._rl_client.report_outcome(event_id, outcome)

                print('Round: {}, Person: {}, Action: {}, Outcome: {}'
                    .format(round, person._id, chosen_action_id, outcome))

                round += 1
                time.sleep(0.1)
            except Exception as e:
                print(e)
                time.sleep(2)
                continue

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
