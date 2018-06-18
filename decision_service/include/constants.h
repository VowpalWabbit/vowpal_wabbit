#pragma once

namespace reinforcement_learning {
  using str_const = const char * const;
}

namespace reinforcement_learning {  namespace name {
      str_const APP_ID                  = "appid";
      str_const MODEL_SRC               = "model.source";
      str_const MODEL_BLOB_URI          = "model.blob.uri";
      str_const MODEL_REFRESH_INTERVAL  = "model.refreshintervalms";
      str_const MODEL_IMPLEMENTATION    = "model.implementation";       // VW vs other ML
      str_const VW_CMDLINE              = "vw.commandline";
      str_const INITIAL_EPSILON         = "initial_exploration.epsilon";

      str_const INTERACTION_EH_HOST     = "interaction.eventhub.host";
      str_const INTERACTION_EH_NAME     = "interaction.eventhub.name";
      str_const INTERACTION_EH_KEY_NAME = "interaction.eventhub.keyname";
      str_const INTERACTION_EH_KEY      = "interaction.eventhub.key";

      str_const OBSERVATION_EH_HOST     = "observation.eventhub.host";
      str_const OBSERVATION_EH_NAME     = "observation.eventhub.name";
      str_const OBSERVATION_EH_KEY_NAME = "observation.eventhub.keyname";
      str_const OBSERVATION_EH_KEY      = "observation.eventhub.key";

      str_const SEND_HIGH_WATER_MARK    = "eh.send.highwatermark";
      str_const SEND_QUEUE_MAXSIZE      = "eh.send.queue.maxsize";
      str_const SEND_BATCH_INTERVAL     = "eh.send.batchintervalms";

      str_const EH_TEST                 = "eventhub.mock";
}}

namespace reinforcement_learning {  namespace value {
      str_const AZURE_STORAGE_BLOB = "AZURE_STORAGE_BLOB";
      str_const VW                 = "VW";
}}

