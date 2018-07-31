import argparse
import json
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "..", "bindings", "python"))
import rlinference

class my_error_callback(rlinference.error_callback):
  def on_error(self, error_code, error_message):
    print("Background error:")
    print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rlinference.create_config_from_json(config_file.read())

def process_cmd_line(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('--json_config', help='client json config', required=True)
    parser.add_argument('--log_file', help='log file to replay', required=True)
    return parser.parse_args(args)

def main(args):
    options = process_cmd_line(args)

    config = load_config_from_json(options.json_config)
    model = rlinference.live_model(config, my_error_callback())
    model.init()

    with open(options.log_file) as fp:
        for count, line in enumerate(fp):
            current_example = json.loads(line)

            context_json = json.dumps(current_example["c"])
            uuid, model_id, chosen_action_id, action_probabilities = model.choose_rank(current_example["EventId"], context_json)

            if("o" in current_example):
                for observation in current_example["o"]:
                    model.report_outcome(observation["EventId"], observation["v"])


if __name__ == "__main__":
    main(sys.argv[1:])
