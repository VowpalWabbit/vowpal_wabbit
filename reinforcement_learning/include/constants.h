#pragma once

namespace reinforcement_learning {  namespace name {
      const char *const  APP_ID                  = "appid";
      const char *const  MODEL_SRC               = "model.source";
      const char *const  MODEL_BLOB_URI          = "model.blob.uri";
      const char *const  MODEL_REFRESH_INTERVAL  = "model.refreshintervalms";
      const char *const  MODEL_IMPLEMENTATION    = "model.implementation";       // VW vs other ML
      const char *const  VW_CMDLINE              = "vw.commandline";
      const char *const  INITIAL_EPSILON         = "initial_exploration.epsilon";
      const char *const  INTERACTION_EH_HOST     = "interaction.eventhub.host";
      const char *const  INTERACTION_EH_NAME     = "interaction.eventhub.name";
      const char *const  INTERACTION_EH_KEY_NAME = "interaction.eventhub.keyname";
      const char *const  INTERACTION_EH_KEY      = "interaction.eventhub.key";
      const char *const  OBSERVATION_EH_HOST     = "observation.eventhub.host";
      const char *const  OBSERVATION_EH_NAME     = "observation.eventhub.name";
      const char *const  OBSERVATION_EH_KEY_NAME = "observation.eventhub.keyname";
      const char *const  OBSERVATION_EH_KEY      = "observation.eventhub.key";
      const char *const  SEND_HIGH_WATER_MARK    = "eh.send.highwatermark";
      const char *const  SEND_QUEUE_MAXSIZE      = "eh.send.queue.maxsize";
      const char *const  SEND_BATCH_INTERVAL     = "eh.send.batchintervalms";
      const char *const  EH_TEST                 = "eventhub.mock";
}}

namespace reinforcement_learning {  namespace value {
      const char * const AZURE_STORAGE_BLOB = "AZURE_STORAGE_BLOB";
      const char * const VW                 = "VW";
}}

