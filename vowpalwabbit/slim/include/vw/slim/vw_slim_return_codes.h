#pragma once

#define S_VW_PREDICT_OK 0
#define E_VW_PREDICT_ERR_INVALID_MODEL 1
#define E_VW_PREDICT_ERR_WEIGHT_INDEX_OUT_OF_RANGE 2
#define E_VW_PREDICT_ERR_GD_RESUME_NOT_SUPPORTED 3
#define E_VW_PREDICT_ERR_CB_EXPLORATION_MISSING 4
#define E_VW_PREDICT_ERR_NOT_A_CB_MODEL 5
#define E_VW_PREDICT_ERR_NO_MODEL_LOADED 6
#define E_VW_PREDICT_ERR_NO_A_CSOAA_MODEL 7
#define E_VW_PREDICT_ERR_EXPLORATION_FAILED 8
#define E_VW_PREDICT_ERR_INVALID_MODEL_CHECK_SUM 9
#define E_VW_PREDICT_ERR_HASH_SEED_NOT_SUPPORTED 10
#define RETURN_ON_FAIL(stmt)                                    \
  {                                                             \
    int ret##__LINE__ = stmt;                                   \
    if (ret##__LINE__ != S_VW_PREDICT_OK) return ret##__LINE__; \
  }
#define RETURN_EXPLORATION_ON_FAIL(stmt)                                                                       \
  {                                                                                                            \
    int ret##__LINE__ = stmt;                                                                                  \
    if (ret##__LINE__ != S_EXPLORATION_OK) return E_VW_PREDICT_ERR_EXPLORATION_FAILED | (ret##__LINE__ << 16); \
  }
