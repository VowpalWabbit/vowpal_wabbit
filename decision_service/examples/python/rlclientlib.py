class api_status:
    def __init__(self):
        pass

    def get_error_code(self):
        return 0

    def get_error_msg(self):
        return ""

class live_model:
    def __init__(self, config):
        pass
    
    def init(self):
        return api_status()

    def choose_rank(self, uuid, context):
        return ranking_response(), api_status()

    def report_outcome(self, uuid, reward):
        return api_status()

class ranking_response:
    def __init(self):
        pass

    def get_choosen_action_id(self):
        return 0, api_status()

class config_collection:
    def __init__(self):
        pass


class utility_config:
    def __init__(self):
        pass

    def create_from_json(config_json):
        return "", api_status()

class error_code:
    success = 0
    invalid_argument = 1
    background_queue_overflow = 2
    eventhub_http_generic = 3
    http_bad_status_code = 4
    action_not_found = 5
    background_thread_start = 6
    not_initialized = 7
    eventhub_generate_SAS_hash = 8
    create_fn_exception = 9
    type_not_registered = 10
    http_uri_not_provided = 11
    last_modified_not_found = 12
    last_modified_invalid = 13
    bad_content_length = 14
    exception_during_http_req = 15
    model_export_frequency_not_provided = 16
    bad_time_interval = 17
    data_callback_exception = 18
    data_callback_not_set = 19
    json_no_actions_found = 20
    json_parse_error = 21
    exploration_error = 22
    action_out_of_bounds = 23
    model_update_error = 24
    model_rank_error = 25
    pdf_sampling_error = 26
    eh_connstr_parse_error = 27
