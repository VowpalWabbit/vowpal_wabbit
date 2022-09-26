// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/global_data.h"
#include "vw/io/logger.h"

#include <memory>

namespace VW
{

struct model_delta
{
    // model_delta takes ownership of the workspace
    explicit model_delta(VW::workspace* ws) : _ws(ws) { }
    explicit model_delta(std::unique_ptr<VW::workspace>&& ws) : _ws(std::move(ws)) { }

    explicit operator VW::workspace&() const { return *_ws; }

private:
    std::unique_ptr<VW::workspace> _ws;
};

/**
 * Merge the differences of several workspaces into the given base workspace. This merges weights
 * and all training state. All given workspaces must be compatible with the base workspace, meaning they
 * should have the same reduction stack and same training based options.
 *
 * If the base workspace was not given, it is assumed that the given workspaces were trained from fresh.
 *
 * Note: This is an experimental API.
 *
 * @param base_workspace Optional common base model that all other models continued training from. If not supplied, then
 * all models are assumed to be trained from scratch.
 * @param workspaces_to_merge List of workspaces to merge.
 * @param logger Optional logger to be used for logging during function and is given to the resulting workspace
 * @return std::unique_ptr<VW::workspace> Pointer to the resulting workspace.
 */
std::unique_ptr<VW::workspace> merge_models(const VW::workspace* base_workspace,
    const std::vector<const VW::workspace*>& workspaces_to_merge, VW::io::logger* logger = nullptr,
    bool is_delta = false);

VW::model_delta merge_deltas(const std::vector<const VW::model_delta*>& deltas_to_merge, VW::io::logger* logger = nullptr);
}  // namespace VW

std::unique_ptr<VW::workspace> operator+(const VW::workspace& ws, const VW::model_delta& md);

VW::model_delta operator-(const VW::workspace& ws1, const VW::workspace& ws2);
