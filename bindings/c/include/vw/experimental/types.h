// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

// uint32_t, uint64_t, etc
#include <stdint.h>
// size_t
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef uint32_t VWStatus;
  static const VWStatus VW_success = 0;
// Generate all error codes based on the definitions provided in "error_data.h"
// This macro gets expanded for each individual error definition
#define ERROR_CODE_DEFINITION(code, name, message) static const VWStatus VW_##name = code;
#include "vw/experimental/error_data.h"
#undef ERROR_CODE_DEFINITION

  typedef uint32_t VWLabelType;
  static const VWLabelType VW_LABEL_SIMPLE = 0;
  static const VWLabelType VW_LABEL_CB = 1;
  static const VWLabelType VW_LABEL_CB_EVAL = 2;
  static const VWLabelType VW_LABEL_CS = 3;
  static const VWLabelType VW_LABEL_MULTILABELS = 4;
  static const VWLabelType VW_LABEL_MULTICLASS = 5;
  static const VWLabelType VW_LABEL_CCB = 6;
  static const VWLabelType VW_LABEL_SLATES = 7;

  typedef uint32_t VWPredictionType;
  static const VWPredictionType VW_PRED_SCALAR = 0;
  static const VWPredictionType VW_PRED_SCALARS = 1;
  static const VWPredictionType VW_PRED_ACTION_SCORES = 2;
  static const VWPredictionType VW_PRED_ACTION_PROBS = 3;
  static const VWPredictionType VW_PRED_ACTION_MULTICLASS = 4;
  static const VWPredictionType VW_PRED_ACTION_MULTILABELS = 5;
  static const VWPredictionType VW_PRED_PROB = 6;
  static const VWPredictionType VW_PRED_MUTLICLASS_PROBS = 7;
  static const VWPredictionType VW_PRED_DECISION_PROBS = 8;

  struct VWWorkspace_tag;
  typedef struct VWWorkspace_tag VWWorkspace;

  struct VWErrorInfo_tag;
  typedef struct VWErrorInfo_tag VWErrorInfo;

  struct VWString_tag;
  typedef struct VWString_tag VWString;

  struct VWLabel_tag;
  typedef struct VWLabel_tag VWLabel;

  struct VWPrediction_tag;
  typedef struct VWPrediction_tag VWPrediction;

  struct VWExample_tag;
  typedef struct VWExample_tag VWExample;

  struct VWOptions_tag;
  typedef struct VWOptions_tag VWOptions;

  typedef VWStatus(VWReadFunc)(void*, char*, size_t, size_t*);
  typedef VWStatus(VWWriteFunc)(void*, const char*, size_t, size_t*);
  typedef VWStatus(VWTraceMessageFunc)(void*, int trace_level, const char*, size_t);

  struct VWSearch_tag;
  typedef struct VWSearch_tag VWSearch;

#ifdef __cplusplus
}
#endif
