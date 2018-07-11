import rlclientlib as rlcl


def load_config_from_json(file_name, cfgcoll):
    with open(file_name, 'r') as config_file:
        status = rlcl.api_status()
        return rlcl.utility_config.create_from_json(config_file.read(), cfgcoll, status)

def main():
    config = rlcl.config_collection()
    if (load_config_from_json("client.json", config) != rlcl.error_code.success):
        print('Unable to Load file: client.json')
        return -1

    status = rlcl.api_status()

    rl = rlcl.live_model(config)

    if (rl.init(status) != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1

    response = rlcl.ranking_response()

    uuid = "uuid"
    context = r"""({ 
                   "User":{"id":"a","major":"eng","hobby":"hiking"},
                   "_multi":[{"a1":"f1"},{"a2":"f2"}]})"""

    if (rl.choose_rank(uuid, context, response, status) != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1

    choosen_action = 0
    if (response.get_choosen_action_id(choosen_action, status) != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1

    print('Chosen action id is {choosen_action}'.format(choosen_action = choosen_action))

    reward = 1.0
    if (rl.report_outcome(uuid, reward, status) != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1


if __name__ == "__main__":
   main()