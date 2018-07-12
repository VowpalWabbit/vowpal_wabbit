import rlclientlib as rlcl


def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        status = rlcl.api_status()
        return rlcl.utility_config.create_from_json(config_file.read())[0]

def main():
    config = rlcl.config_collection()

    rl = rlcl.live_model(config)

    status = rl.init()
    if (status.get_error_code() != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1

    response = rlcl.ranking_response()

    uuid = "uuid"
    context = r"""({ 
                   "User":{"id":"a","major":"eng","hobby":"hiking"},
                   "_multi":[{"a1":"f1"},{"a2":"f2"}]})"""

    response, status = rl.choose_rank(uuid, context)
    if (status.get_error_code() != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1

    choosen_action, status = response.get_choosen_action_id()
    if (status.get_error_code() != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1

    print('Chosen action id is {choosen_action}'.format(choosen_action = choosen_action))

    reward = 1.0
    status = rl.report_outcome(uuid, reward)
    if (status.get_error_code() != rlcl.error_code.success):
        print(status.get_error_msg())
        return -1


if __name__ == "__main__":
   main()
