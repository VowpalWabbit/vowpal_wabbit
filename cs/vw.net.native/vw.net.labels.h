#pragma once

#include "vw.net.native.h"
#include "vw.net.workspace.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/multiclass.h"
#include "vw/core/vw.h"

extern "C"
{
  API float SimpleLabelReadFromExample(VW::example* ex, float& weight, float& initial);
  API void SimpleLabelUpdateExample(vw_net_native::workspace_context* workspace, VW::example* ex, float label,
      float* maybe_weight, float* maybe_initial);

  API VW::cb_class* CbLabelReadFromExampleDangerous(VW::example* ex);
  API void CbLabelUpdateExample(VW::example* ex, const VW::cb_class* f);

  API vw_net_native::ERROR_CODE StringLabelParseAndUpdateExample(vw_net_native::workspace_context* workspace,
      VW::example* ex, const char* label, size_t label_len, VW::experimental::api_status* status = nullptr);

  API float SharedLabelGetCostConstant();

  // Note that this is not const char* because the receiver owns free()ing the string
  // using FreeDupString(); This is an unfortunate side-effect of how this API works:
  // Constructing dynamic strings as a function of the input parameters on the native
  // side.
  API char* ComputeDiffDescriptionSimpleLabels(VW::example* ex1, VW::example* ex2);
  API char* ComputeDiffDescriptionCbLabels(VW::example* ex1, VW::example* ex2);
}
