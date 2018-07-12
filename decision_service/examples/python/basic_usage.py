import rlclientlib as rlcl


def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rlcl.utility_config.create_from_json(config_file.read())

def main():
    try:
        config = rlcl.config_collection()

        rl = rlcl.live_model(config)

        rl.init()

        response = rlcl.ranking_response()

        uuid = "uuid"
        context = r"""({ 
                       "User":{"id":"a","major":"eng","hobby":"hiking"},
                       "_multi":[{"a1":"f1"},{"a2":"f2"}]})"""

        response = rl.choose_rank(uuid, context)

        choosen_action = response.get_choosen_action_id()

        print('Chosen action id is {choosen_action}'.format(choosen_action = choosen_action))

        reward = 1.0
        rl.report_outcome(uuid, reward)
    
    except rlcl.exception as ex:
        print('Something bad happened: {description}'.format(description = ex.get_error_msg()))
        return -1

if __name__ == "__main__":
   main()
