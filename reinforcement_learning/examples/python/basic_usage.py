import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", "bindings", "python"))

import rlinference

class my_error_callback(rlinference.error_callback):
    def on_error(self, error_code, error_message):
        print("Background error:")
        print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rlinference.create_config_from_json(config_file.read())

def main():
    config = load_config_from_json("client.json")

    test_cb = my_error_callback()
    model = rlinference.live_model(config, test_cb)
    model.init()

    uuid = "uuid"
    context = '{"User":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"a1":"f1"},{"a2":"f2"}]}'

    uuid, model_id, chosen_action_id, all_action_probabilities = model.choose_rank(context)

    print("uuid: " + uuid)
    print("model_id: " + model_id)
    print("chosen action id: " + str(chosen_action_id))
    print("all action probabilities " + str(all_action_probabilities))

    reward = 1.0
    model.report_outcome(uuid, reward)

if __name__ == "__main__":
   main()
