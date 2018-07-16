import rlinference

class my_error_callback(rlinference.error_callback):
  def on_error(self, error_code, error_message):
    print(error_code)
    print(error_message)

json_contents = open('client.json', 'r').read()
config = rlinference.create_config_from_json(json_contents)
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