import uuid
import rl_client

class my_error_callback(rl_client.error_callback):
    def on_error(self, error_code, error_message):
        print("Background error:")
        print(error_message)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rl_client.create_config_from_json(config_file.read())

def main():
    config = load_config_from_json("client.json")

    test_cb = my_error_callback()
    model = rl_client.live_model(config, test_cb)
    model.init()

    event_id = str(uuid.uuid4())
    context = '{"User":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"N1":{"F1":"V1"},"N2":{"F2":"V2"}},{"N3":{"F1":"V3"}}]}'

    model_id, chosen_action_id, actions_probabilities = model.choose_rank(context, event_id=event_id)

    print("event_id: " + event_id)
    print("model_id: " + model_id)
    print("chosen action id: " + str(chosen_action_id))
    print("all action probabilities " + str(actions_probabilities))

    model_id, chosen_action_id, actions_probabilities, event_id = model.choose_rank(context, deferred = False)
    model.report_action_taken(event_id)
    print("event_id: " + event_id)
    print("model_id: " + model_id)
    print("chosen action id: " + str(chosen_action_id))
    print("actions probabilities list: " + str(actions_probabilities))

    event_id = str(uuid.uuid4())
    model_id, chosen_action_id, actions_probabilities = model.choose_rank(context, deferred=True, event_id=event_id)
    model.report_action_taken(event_id)
    print("event_id: " + event_id)
    print("model_id: " + model_id)
    print("chosen action id: " + str(chosen_action_id))
    print("actions probabilities list: " + str(actions_probabilities))


    outcome = 1.0
    model.report_outcome(event_id, outcome)

if __name__ == "__main__":
   main()
