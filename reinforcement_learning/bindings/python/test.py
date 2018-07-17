import unittest
import rlinference

test_config_json = '''
{
    "ApplicationID": "<appid>",
    "EventHubInteractionConnectionString": "Endpoint=sb://<ingest>.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=<SAKey>;EntityPath=interaction",
    "EventHubObservationConnectionString": "Endpoint=sb://<ingest>.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=<SAKey>;EntityPath=observation",
    "IsExplorationEnabled": true,
    "ModelBlobUri": "https://<storage>.blob.core.windows.net/mwt-models/current?sv=2017-07-29&sr=b&sig=<sig>&st=2018-06-26T09%3A00%3A55Z&se=2028-06-26T09%3A01%3A55Z&sp=r",
    "AppInsightsKey": "<AppInsightsKey>",
    "InitialExplorationEpsilon": 1.0,
    "SendBatchIntervalMs": 5,
    "SendHighMaterMark": true
}
'''

class ConfigTests(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.config = rlinference.create_config_from_json(test_config_json)

    def test_set(self):
        self.config.set("CustomKey", "CustomValue")
        self.assertEqual(self.config.get("CustomKey", None), "CustomValue")

    def test_get(self):
        self.assertEqual(self.config.get(rlinference.APP_ID, None), "<appid>")
        self.assertEqual(self.config.get(rlinference.INTERACTION_EH_HOST, None), "<ingest>.servicebus.windows.net")
        self.assertEqual(self.config.get(rlinference.INTERACTION_EH_NAME, None), "interaction")
        self.assertEqual(self.config.get(rlinference.INTERACTION_EH_KEY_NAME, None), "RootManageSharedAccessKey")
        self.assertEqual(self.config.get(rlinference.INTERACTION_EH_KEY, None), "<SAKey>")

    def test_get_default(self):
        self.assertEqual(self.config.get("UnsetKey", "DefaultValue"), "DefaultValue")

    def test_get_int(self):
        self.assertEqual(self.config.get_int(rlinference.SEND_BATCH_INTERVAL,-1), 5)

    def test_get_bool(self):
        self.assertEqual(self.config.get_bool(rlinference.SEND_HIGH_WATER_MARK, False), True)

    def test_get_float(self):
        self.assertAlmostEqual(self.config.get_float(rlinference.INITIAL_EPSILON, -1.0), 1.0)

def load_config_from_json(file_name):
    with open(file_name, 'r') as config_file:
        return rlinference.create_config_from_json(config_file.read())

# The following tests will fail without a suitable 'client.json' file present, so they cannot be
# run in an automated test harness. Dependency injection for network components is required to make
# these automated.
class LiveModelTests(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.config = load_config_from_json("client.json")

    def test_choose_rank(self):
        model = rlinference.live_model(self.config)
        model.init()

        uuid = "uuid"
        context = '{"_multi":[{},{}]}'
        model.choose_rank(uuid, context)

    def test_choose_rank_invalid_context(self):
        model = rlinference.live_model(self.config)
        model.init()

        uuid = "uuid"
        invalid_context = ""
        self.assertRaises(Exception, model.choose_rank, uuid, invalid_context)

    def test_choose_rank_invalid_uuid(self):
        model = rlinference.live_model(self.config)
        model.init()

        invalid_uuid = ""
        context = '{"_multi":[{},{}]}'
        self.assertRaises(Exception, model.choose_rank, invalid_uuid, context)

    def test_report_outcome(self):
        model = rlinference.live_model(self.config)
        model.init()

        uuid = "uuid"
        context = '{"_multi":[{},{}]}'
        model.choose_rank(uuid, context)
        model.report_outcome(uuid, 1.0)

    def test_report_outcome_no_connection(self):
        # Requires dependency injection for network.
        return

    def test_report_outcome_server_failure(self):
        # Requires dependency injection for network.
        return

    def test_async_error_callback(self):
        # Requires dependency injection to fake a background failure.
        return

if __name__ == '__main__':
    unittest.main()