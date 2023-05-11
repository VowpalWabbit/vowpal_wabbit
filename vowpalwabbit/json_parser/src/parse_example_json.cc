// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/json_parser/parse_example_json.h"

#include "json_utils.h"
#include "vw/common/string_view.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/cb_continuous_label.h"
#include "vw/core/learner.h"

// seems to help with skipping spaces
// #define RAPIDJSON_SIMD
// #define RAPIDJSON_SSE42

// Let MSVC know that it should not even try to compile RapidJSON as managed
// - pragma documentation: https://docs.microsoft.com/en-us/cpp/preprocessor/managed-unmanaged?view=vs-2017
// - /clr compilation detection: https://docs.microsoft.com/en-us/cpp/dotnet/how-to-detect-clr-compilation?view=vs-2017
#if (_MANAGED == 1) || (_M_CEE == 1)
#  pragma managed(push, off)
#endif

// RapidJson triggers this warning by memcpying non-trivially copyable type. Ignore it so that our warnings are not
// polluted by it.
// https://github.com/Tencent/rapidjson/issues/1700
VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_CLASS_MEMACCESS
#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>
VW_WARNING_STATE_POP

#if (_MANAGED == 1) || (_M_CEE == 1)
#  pragma managed(pop)
#endif

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <limits>
#include <sstream>
#include <vector>

// portability fun
#ifndef _WIN32
#  define _stricmp strcasecmp
#endif

using namespace rapidjson;

namespace VW
{
class workspace;
}

namespace
{
template <bool audit>
class BaseState;

template <bool audit>
class Context;

template <bool audit>
class BaseState
{
public:
  const char* name;

  BaseState(const char* pname) : name(pname) {}

  virtual BaseState<audit>* Null(Context<audit>& ctx)
  {
    // ignore Null by default and stay in the current state
    return ctx.previous_state == nullptr ? this : ctx.previous_state;
  }

  virtual BaseState<audit>* Bool(Context<audit>& ctx, bool b)
  {
    ctx.error() << "Unexpected token: bool (" << (b ? "true" : "false") << ")";
    return nullptr;
  }

  virtual BaseState<audit>* Float(Context<audit>& ctx, float v)
  {
    ctx.error() << "Unexpected token: float (" << v << ")";
    return nullptr;
  }

  virtual BaseState<audit>* Uint(Context<audit>& ctx, unsigned v)
  {
    ctx.error() << "Unexpected token: uint (" << v << ")";
    return nullptr;
  }

  virtual BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool)
  {
    ctx.error() << "Unexpected token: std::string('" << str << "' len: " << len << ")";
    return nullptr;
  }

  virtual BaseState<audit>* StartObject(Context<audit>& ctx)
  {
    ctx.error() << "Unexpected token: {";
    return nullptr;
  }

  virtual BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool /* copy */)
  {
    ctx.error() << "Unexpected token: key('" << str << "' len: " << len << ")";
    return nullptr;
  }

  virtual BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType)
  {
    ctx.error() << "Unexpected token: }";
    return nullptr;
  }

  virtual BaseState<audit>* StartArray(Context<audit>& ctx)
  {
    ctx.error() << "Unexpected token: [";
    return nullptr;
  }

  virtual BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType)
  {
    ctx.error() << "Unexpected token: ]";
    return nullptr;
  }
};

template <bool audit>
class ArrayToPdfState : public BaseState<audit>
{
public:
  VW::continuous_actions::pdf_segment segment;

  BaseState<audit>* return_state;

  ArrayToPdfState() : BaseState<audit>("ArrayToPdfObject") {}

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    obj_return_state = ctx.previous_state;
    return this;
  }

  BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool /* copy */) override
  {
    ctx.key = str;
    ctx.key_length = len;
    return this;
  }

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType /* len */, bool) override
  {
    if (_stricmp(str, "NaN") != 0)
    {
      ctx.error() << "The only supported string in the array is 'NaN'";
      return nullptr;
    }

    return this;
  }

  BaseState<audit>* StartArray(Context<audit>&) override
  {
    segment = {0., 0., 0.};
    return this;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType) override
  {
    // check valid pdf else remove
    auto& red_fts = ctx.ex->ex_reduction_features.template get<VW::continuous_actions::reduction_features>();
    if (!VW::continuous_actions::is_valid_pdf(red_fts.pdf)) { red_fts.pdf.clear(); }
    return return_state;
  }

  BaseState<audit>* Float(Context<audit>& ctx, float v) override
  {
    if (!_stricmp(ctx.key, "left")) { segment.left = v; }
    else if (!_stricmp(ctx.key, "right")) { segment.right = v; }
    else if (!_stricmp(ctx.key, "pdf_value")) { segment.pdf_value = v; }
    else if (!_stricmp(ctx.key, "chosen_action"))
    {
      ctx.ex->ex_reduction_features.template get<VW::continuous_actions::reduction_features>().chosen_action = v;
    }
    else
    {
      ctx.error() << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length;
      return nullptr;
    }

    return this;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned v) override { return Float(ctx, static_cast<float>(v)); }

  BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType) override
  {
    ctx.ex->ex_reduction_features.template get<VW::continuous_actions::reduction_features>().pdf.push_back(segment);
    segment = {0., 0., 0.};
    return obj_return_state;
  }

private:
  BaseState<audit>* obj_return_state;
};

template <bool audit>
class ArrayToGraphState : public BaseState<audit>
{
public:
  BaseState<audit>* return_state;
  VW::cb_graph_feedback::triplet graph_triplet;

  ArrayToGraphState() : BaseState<audit>("ArrayToGraphObject"), graph_triplet(0, 0, -1.f) {}

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    obj_return_state = ctx.previous_state;
    return this;
  }

  BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool /* copy */) override
  {
    ctx.key = str;
    ctx.key_length = len;
    return this;
  }

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool) override
  {
    ctx.error() << "Unexpected token: std::string('" << str << "' len: " << len << ")";
    return nullptr;
  }

  BaseState<audit>* StartArray(Context<audit>& ctx) override
  {
    return_state = ctx.previous_state;
    graph_triplet = VW::cb_graph_feedback::triplet(0, 0, -1.f);
    return this;
  }

  BaseState<audit>* EndArray(Context<audit>&, rapidjson::SizeType) override { return return_state; }

  BaseState<audit>* Float(Context<audit>& ctx, float v) override
  {
    if (!_stricmp(ctx.key, "val")) { graph_triplet.val = v; }
    else
    {
      ctx.error() << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length;
      return nullptr;
    }

    return this;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned v) override
  {
    if (!_stricmp(ctx.key, "val")) { graph_triplet.val = v; }
    else if (!_stricmp(ctx.key, "row")) { graph_triplet.row = v; }
    else if (!_stricmp(ctx.key, "col")) { graph_triplet.col = v; }
    else
    {
      ctx.error() << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length;
      return nullptr;
    }
    return this;
  }

  BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType) override
  {
    auto& graph_reduction_features =
        ctx.ex->ex_reduction_features.template get<VW::cb_graph_feedback::reduction_features>();
    if (graph_triplet.val != -1.f)
    {
      graph_reduction_features.push_triplet(graph_triplet.row, graph_triplet.col, graph_triplet.val);
    }
    graph_triplet = VW::cb_graph_feedback::triplet(0, 0, -1.f);
    return obj_return_state;
  }

private:
  BaseState<audit>* obj_return_state;
};

template <bool audit>
class LabelObjectState : public BaseState<audit>
{
public:
  VW::cb_class cb_label;
  VW::cb_continuous::continuous_label_elm cont_label_element = {0., 0., 0.};
  bool found = false;
  bool found_cb = false;
  bool found_cb_continuous = false;
  std::vector<unsigned int> actions;
  std::vector<float> probs;
  std::vector<unsigned int> inc;

  LabelObjectState() : BaseState<audit>("LabelObject") {}

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    ctx._label_parser.default_label(ctx.ex->l);

    // don't allow { { { } } }
    if (ctx.previous_state == this)
    {
      ctx.error() << "invalid label object. nested objected.";
      return nullptr;
    }

    // keep previous state
    return_state = ctx.previous_state;

    return this;
  }

  BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool /* copy */) override
  {
    ctx.key = str;
    ctx.key_length = len;
    return this;
  }

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType /* len */, bool) override
  {
    if (_stricmp(str, "NaN") != 0)
    {
      ctx.error() << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length
                  << ". The only string value supported in this context is NaN.";
      return nullptr;
    }

    // simple
    if (!_stricmp(ctx.key, "Label"))
    {
      ctx.ex->l.simple.label = std::numeric_limits<float>::quiet_NaN();
      found = true;
    }
    else if (!_stricmp(ctx.key, "Initial"))
    {
      auto& simple_red_features = ctx.ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
      simple_red_features.initial = std::numeric_limits<float>::quiet_NaN();
      found = true;
    }
    else if (!_stricmp(ctx.key, "Weight"))
    {
      auto& simple_red_features = ctx.ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
      simple_red_features.weight = std::numeric_limits<float>::quiet_NaN();
      found = true;
    }
    // CB/CA
    else if (!_stricmp(ctx.key, "Cost"))
    {
      if (found_cb_continuous) { cont_label_element.cost = std::numeric_limits<float>::quiet_NaN(); }
      else
      {
        cb_label.cost = std::numeric_limits<float>::quiet_NaN();
        found_cb = true;
      }
    }
    else if (!_stricmp(ctx.key, "Probability"))
    {
      cb_label.probability = std::numeric_limits<float>::quiet_NaN();
      found_cb = true;
    }
    // CA
    else if (!_stricmp(ctx.key, "Pdf_value") && found_cb_continuous)
    {
      cont_label_element.pdf_value = std::numeric_limits<float>::quiet_NaN();
    }
    else
    {
      ctx.error() << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length;
      return nullptr;
    }

    return this;
  }

  BaseState<audit>* Float(Context<audit>& ctx, float v) override
  {
    // simple
    if (!_stricmp(ctx.key, "Label"))
    {
      ctx.ex->l.simple.label = v;
      found = true;
    }
    else if (!_stricmp(ctx.key, "Initial"))
    {
      auto& simple_red_features = ctx.ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
      simple_red_features.initial = v;
      found = true;
    }
    else if (!_stricmp(ctx.key, "Weight"))
    {
      auto& simple_red_features = ctx.ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
      simple_red_features.weight = v;
      found = true;
    }
    // CB/CA
    else if (!_stricmp(ctx.key, "Action"))
    {
      if (found_cb_continuous) { cont_label_element.action = v; }
      else
      {
        cb_label.action = static_cast<uint32_t>(v);
        found_cb = true;
      }
    }
    else if (!_stricmp(ctx.key, "Cost"))
    {
      if (found_cb_continuous) { cont_label_element.cost = v; }
      else
      {
        cb_label.cost = v;
        found_cb = true;
      }
    }
    else if (!_stricmp(ctx.key, "Probability"))
    {
      cb_label.probability = v;
      found_cb = true;
    }
    // CA
    else if (!_stricmp(ctx.key, "Pdf_value") && found_cb_continuous) { cont_label_element.pdf_value = v; }
    else
    {
      ctx.error() << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length;
      return nullptr;
    }

    return this;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned v) override { return Float(ctx, static_cast<float>(v)); }

  BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType) override
  {
    if (ctx._label_parser.label_type == VW::label_type_t::CCB)
    {
      auto& ld = ctx.ex->l.conditional_contextual_bandit;

      for (auto id : inc) { ld.explicit_included_actions.push_back(id); }
      inc.clear();

      if ((actions.size() != 0) && (probs.size() != 0))
      {
        auto* outcome = new VW::ccb_outcome();
        outcome->cost = cb_label.cost;
        if (actions.size() != probs.size()) { THROW("Actions and probabilities must be the same length."); }

        for (size_t i = 0; i < this->actions.size(); i++) { outcome->probabilities.push_back({actions[i], probs[i]}); }
        actions.clear();
        probs.clear();

        ld.outcome = outcome;
        cb_label = VW::cb_class{};
      }
    }
    else if (ctx._label_parser.label_type == VW::label_type_t::SLATES)
    {
      auto& ld = ctx.ex->l.slates;
      if ((actions.size() != 0) && (probs.size() != 0))
      {
        if (actions.size() != probs.size()) { THROW("Actions and probabilities must be the same length."); }
        ld.labeled = true;

        for (size_t i = 0; i < this->actions.size(); i++) { ld.probabilities.push_back({actions[i], probs[i]}); }
        actions.clear();
        probs.clear();
        cb_label = VW::cb_class{};
      }
    }
    else if (found_cb)
    {
      if (ctx._label_parser.label_type == VW::label_type_t::CB_WITH_OBSERVATIONS)
      {
        auto& ld = ctx.ex->l.cb_with_observations;
        ld.event.costs.push_back(cb_label);
        cb_label = VW::cb_class{};
      }
      else
      {
        auto& ld = ctx.ex->l.cb;
        ld.costs.push_back(cb_label);

        found_cb = false;
        cb_label = VW::cb_class{};
      }
    }
    else if (found_cb_continuous)
    {
      auto* ld = &ctx.ex->l.cb_cont;
      ld->costs.push_back(cont_label_element);

      found_cb_continuous = false;
      cont_label_element = {0., 0., 0.};
    }
    else if (found) { found = false; }

    return return_state;
  }

private:
  BaseState<audit>* return_state;
};

// "_label_*":
template <bool audit>
class LabelSinglePropertyState : public BaseState<audit>
{
public:
  LabelSinglePropertyState() : BaseState<audit>("LabelSingleProperty") {}

  BaseState<audit>* StartObject(Context<audit>& ctx) override { return ctx.label_object_state.StartObject(ctx); }

  // forward _label
  BaseState<audit>* Float(Context<audit>& ctx, float v) override
  {
    // skip "_label_"
    ctx.key += 7;
    ctx.key_length -= 7;

    if (ctx.label_object_state.Float(ctx, v) == nullptr) { return nullptr; }

    return ctx.previous_state;
  }

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool copy) override
  {
    // skip "_label_"
    ctx.key += 7;
    ctx.key_length -= 7;

    if (ctx.label_object_state.String(ctx, str, len, copy) == nullptr) { return nullptr; }

    return ctx.previous_state;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned v) override
  {
    // skip "_label_"
    ctx.key += 7;
    ctx.key_length -= 7;

    if (ctx.label_object_state.Uint(ctx, v) == nullptr) { return nullptr; }

    return ctx.previous_state;
  }
};

template <bool audit>
class LabelIndexState : public BaseState<audit>
{
public:
  int index;

  LabelIndexState() : BaseState<audit>("LabelIndex"), index(-1) {}

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned int v) override
  {
    index = v;
    return ctx.previous_state;
  }
};

// "_label":"1"
// Note: doesn't support labelIndex
template <bool audit>
class LabelState : public BaseState<audit>
{
public:
  LabelState() : BaseState<audit>("Label") {}

  BaseState<audit>* StartObject(Context<audit>& ctx) override { return ctx.label_object_state.StartObject(ctx); }

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType /* len */, bool) override
  {
    auto null_logger = VW::io::create_null_logger();
    VW::parse_example_label(str, ctx._label_parser, ctx._ldict, *ctx._reuse_mem, *ctx.ex, null_logger);
    return ctx.previous_state;
  }

  BaseState<audit>* Float(Context<audit>& ctx, float v) override
  {
    // TODO: once we introduce label types, check here
    ctx.ex->l.simple.label = v;
    return ctx.previous_state;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned v) override
  {
    // TODO: once we introduce label types, check here
    ctx.ex->l.simple.label = static_cast<float>(v);
    return ctx.previous_state;
  }
};

// "_text":"a b c"
template <bool audit>
class TextState : public BaseState<audit>
{
public:
  TextState() : BaseState<audit>("text") {}

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool)
  {
    auto& ns = ctx.CurrentNamespace();

    // split into individual features
    const char* start = str;
    const char* end = str + length;
    for (char* p = const_cast<char*>(str); p != end; p++)
    {
      switch (*p)
      {
          // split on space and tab
        case ' ':
        case '\t':
          *p = '\0';
          if (p - start > 0) { ns.add_feature(start, ctx._hash_func, ctx._parse_mask); }

          start = p + 1;
          break;
          // escape chars
        case ':':
        case '|':
          *p = '_';
          break;
      }
    }

    if (start < end) { ns.add_feature(start, ctx._hash_func, ctx._parse_mask); }

    return ctx.previous_state;
  }
};

template <bool audit>
class TagState : public BaseState<audit>
{
public:
  // "_tag":"abc"
  TagState() : BaseState<audit>("tag") {}

  BaseState<audit>* String(Context<audit>& ctx, const char* str, SizeType length, bool)
  {
    ctx.ex->tag.insert(ctx.ex->tag.end(), str, str + length);
    return ctx.previous_state;
  }
};

template <bool audit>
class MultiState : public BaseState<audit>
{
public:
  MultiState() : BaseState<audit>("Multi") {}

  BaseState<audit>* StartArray(Context<audit>& ctx) override
  {
    // mark shared example
    if (ctx._label_parser.label_type == VW::label_type_t::CB)
    {
      VW::cb_label* ld = &ctx.ex->l.cb;
      VW::cb_class f;

      f.partial_prediction = 0.;
      f.action = static_cast<uint32_t>(VW::uniform_hash("shared", 6, 0));
      f.cost = FLT_MAX;
      f.probability = -1.f;

      ld->costs.push_back(f);
    }
    else if (ctx._label_parser.label_type == VW::label_type_t::CB_WITH_OBSERVATIONS)
    {
      VW::cb_with_observations_label* ld = &(*ctx.examples)[0]->l.cb_with_observations;
      VW::cb_class f;
      ld->event.costs.push_back(f);
    }
    else if (ctx._label_parser.label_type == VW::label_type_t::CCB)
    {
      auto* ld = &ctx.ex->l.conditional_contextual_bandit;
      ld->type = VW::ccb_example_type::SHARED;
    }
    else if (ctx._label_parser.label_type == VW::label_type_t::SLATES)
    {
      auto& ld = ctx.ex->l.slates;
      ld.type = VW::slates::example_type::SHARED;
    }
    else
      THROW("label type is not CB, CB_WITH_OBSERVATIONS, CCB or slates")

    return this;
  }

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    // allocate new example
    ctx.ex = &ctx.example_factory();
    ctx._label_parser.default_label(ctx.ex->l);
    if (ctx._label_parser.label_type == VW::label_type_t::CCB)
    {
      ctx.ex->l.conditional_contextual_bandit.type = VW::ccb_example_type::ACTION;
    }
    else if (ctx._label_parser.label_type == VW::label_type_t::SLATES)
    {
      ctx.ex->l.slates.type = VW::slates::example_type::ACTION;
    }

    ctx.examples->push_back(ctx.ex);

    // setup default namespace
    ctx.PushNamespace(" ", this);

    return &ctx.default_state;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType) override
  {
    // return to shared example
    ctx.ex = (*ctx.examples)[0];

    return &ctx.default_state;
  }
};

template <bool audit>
class ObservationState : public BaseState<audit>
{
public:
  ObservationState() : BaseState<audit>("Observation") {}

  BaseState<audit>* StartArray(Context<audit>& /*ctx*/) override { return this; }

  // NO_SANITIZE_UNDEFINED needed because ctx.example_factory function pointer may be typecasted
  BaseState<audit>* NO_SANITIZE_UNDEFINED StartObject(Context<audit>& ctx) override
  {
    // setup default namespace
    ctx.PushNamespace(" ", this);

    return &ctx.default_state;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType) override
  {
    // return to shared example
    ctx.ex = (*ctx.examples)[0];
    return &ctx.decision_service_state;
  }
};

template <bool audit>
class DefinitelyBadState : public BaseState<audit>
{
public:
  DefinitelyBadState() : BaseState<audit>("DefinitelyBad") {}

  BaseState<audit>* Bool(Context<audit>& ctx, bool b) override
  {
    if (b) { ctx.observation_example->l.cb_with_observations.is_definitely_bad = true; }

    return ctx.previous_state;
  }
};

// This state makes the assumption we are in CCB
template <bool audit>
class SlotsState : public BaseState<audit>
{
public:
  SlotsState() : BaseState<audit>("Slots") {}
  BaseState<audit>* saved;
  BaseState<audit>* saved_root_state;

  BaseState<audit>* StartArray(Context<audit>& ctx) override
  {
    // drain existing added namespace
    // todo check bounds
    saved = ctx.PopNamespace();
    saved_root_state = ctx.root_state;
    ctx.root_state = this;
    return this;
  }

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    // allocate new example
    ctx.ex = &ctx.example_factory();
    ctx._label_parser.default_label(ctx.ex->l);
    if (ctx._label_parser.label_type == VW::label_type_t::CCB)
    {
      ctx.ex->l.conditional_contextual_bandit.type = VW::ccb_example_type::SLOT;
    }
    else if (ctx._label_parser.label_type == VW::label_type_t::SLATES)
    {
      ctx.ex->l.slates.type = VW::slates::example_type::SLOT;
    }

    ctx.examples->push_back(ctx.ex);

    // The end object logic assumes shared example so we need to take an extra one here.
    ctx.label_index_state.index = static_cast<int>(ctx.examples->size()) - 2;

    // setup default namespace
    ctx.PushNamespace(" ", this);

    return &ctx.default_state;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType) override
  {
    // return to shared example
    ctx.ex = (*ctx.examples)[0];

    ctx.PushNamespace(" ", saved);
    ctx.root_state = saved_root_state;

    return &ctx.default_state;
  }
};

// "...":[Numbers only]
template <bool audit>
class ArrayState : public BaseState<audit>
{
public:
  ArrayState() : BaseState<audit>("Array") {}

  BaseState<audit>* StartArray(Context<audit>& ctx) override
  {
    if (ctx.previous_state == this)
    {
      ctx.error() << "Nested arrays are not supported";
      return nullptr;
    }

    ctx.PushNamespace(ctx.key, ctx.previous_state);

    array_hash = ctx.CurrentNamespace().namespace_hash;

    return this;
  }

  BaseState<audit>* Float(Context<audit>& ctx, float f) override
  {
    if (audit)
    {
      std::stringstream str;
      str << '[' << (array_hash - ctx.CurrentNamespace().namespace_hash) << ']';

      ctx.CurrentNamespace().add_feature(f, array_hash, str.str().c_str());
    }
    else { ctx.CurrentNamespace().add_feature(f, array_hash, nullptr); }
    array_hash++;

    return this;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned f) override { return Float(ctx, static_cast<float>(f)); }

  BaseState<audit>* Null(Context<audit>& /* ctx */) override
  {
    // ignore null values and stay in current state
    return this;
  }

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    // parse properties
    ctx.PushNamespace(ctx.namespace_path.size() > 0 ? ctx.CurrentNamespace().name : " ", this);

    return &ctx.default_state;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType /* elementCount */) override
  {
    return ctx.PopNamespace();
  }

private:
  VW::feature_index array_hash;
};

// only 0 is valid as DefaultState::Ignore injected that into the source stream
template <bool audit>
class IgnoreState : public BaseState<audit>
{
public:
  IgnoreState() : BaseState<audit>("Ignore") {}

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned) override { return ctx.previous_state; }
};

template <bool audit>
class DefaultState : public BaseState<audit>
{
public:
  DefaultState() : BaseState<audit>("Default") {}

  BaseState<audit>* Ignore(Context<audit>& ctx, rapidjson::SizeType length)
  {
    // fast ignore
    // skip key + \0 + "
    char* head = ctx.stream->src_ + length + 2;
    if (head >= ctx.stream_end || *head != ':')
    {
      ctx.error() << "Expected ':' found '" << *head << "'";
      return nullptr;
    }
    head++;

    // scan for ,}
    // support { { ... } }
    int depth = 0, sq_depth = 0;
    bool stop = false;
    while (!stop)
    {
      switch (*head)
      {
        case '\0':
          ctx.error() << "Found EOF";
          return nullptr;
        case '"':
        {
          // skip strings
          bool stopInner = false;
          while (!stopInner)
          {
            head++;
            switch (*head)
            {
              case '\0':
                ctx.error() << "Found EOF";
                return nullptr;
              case '\\':
                head++;
                break;
              case '"':
                stopInner = true;
                break;
            }
          }
          break;
        }
        case '{':
          depth++;
          break;
        case '}':
          if (depth == 0 && sq_depth == 0) { stop = true; }
          else { depth--; }
          break;
        case '[':
          sq_depth++;
          break;
        case ']':
          if (depth == 0 && sq_depth == 0) { stop = true; }
          else { sq_depth--; }
          break;
        case ',':
          if (depth == 0 && sq_depth == 0) { stop = true; }
          break;
      }
      head++;
    }

    // skip key + \0 + ":
    char* value = ctx.stream->src_ + length + 3;
    if (value >= ctx.stream_end)
    {
      ctx.error() << "Found EOF";
      return nullptr;
    }

    *value = '0';
    value++;
    memset(value, ' ', head - value - 1);

    return &ctx.ignore_state;
  }

  BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool) override
  {
    ctx.key = str;
    ctx.key_length = length;

    if (length > 0 && str[0] == '_')
    {
      // match _label*
      if (ctx.key_length >= 6 && !strncmp(ctx.key, "_label", 6))
      {
        if (ctx.key_length >= 7 && ctx.key[6] == '_')
        {
          if (length >= 9 && !strncmp(&ctx.key[7], "ca", 2)) { ctx.label_object_state.found_cb_continuous = true; }
          return &ctx.label_single_property_state;
        }
        else if (ctx.key_length == 6) { return &ctx.label_state; }
        else if (ctx.key_length == 11 && !_stricmp(ctx.key, "_labelIndex")) { return &ctx.label_index_state; }
        else
        {
          ctx.error() << "Unsupported key '" << ctx.key << "' len: " << length;
          return nullptr;
        }
      }

      if (ctx.key_length == 5 && !strcmp(ctx.key, "_text")) { return &ctx.text_state; }

      // TODO: _multi in _multi...
      if (ctx.key_length == 6 && !strcmp(ctx.key, "_multi")) { return &ctx.multi_state; }
      if (ctx.key_length == 6 && !strcmp(ctx.key, "_graph")) { return &ctx.array_to_graph_state; }

      if (ctx.key_length == 6 && !strcmp(ctx.key, "_slots")) { return &ctx.slots_state; }

      if (ctx.key_length == 4 && !_stricmp(ctx.key, "_tag")) { return &ctx.tag_state; }

      if (ctx.key_length == 4 && !_stricmp(ctx.key, "_inc"))
      {
        ctx.array_uint_state.output_array = &ctx.label_object_state.inc;
        ctx.array_uint_state.return_state = this;
        return &ctx.array_uint_state;
      }

      if (ctx.key_length == 2 && ctx.key[1] == 'a')
      {
        ctx.array_uint_state.output_array = &ctx.label_object_state.actions;
        ctx.array_uint_state.return_state = this;
        return &ctx.array_uint_state;
      }

      if (ctx.key_length == 2 && ctx.key[1] == 'p')
      {
        // Ignore "_p" when it is inside the "c" key in decision service state.
        if (ctx.root_state == &ctx.decision_service_state) { Ignore(ctx, length); }

        ctx.array_float_state.output_array = &ctx.label_object_state.probs;
        ctx.array_float_state.return_state = this;
        return &ctx.array_float_state;
      }

      else if (length == 8 && !strncmp(str, "_slot_id", 8))
      {
        if (ctx._label_parser.label_type != VW::label_type_t::SLATES)
        {
          THROW("Can only use _slot_id with slates examples");
        }
        ctx.uint_state.output_uint = &ctx.ex->l.slates.slot_id;
        ctx.array_float_state.return_state = this;
        return &ctx.array_float_state;
      }

      else if (ctx.key_length == 20 && !strncmp(str, "_original_label_cost", 20))
      {
        if (!ctx.decision_service_data) { THROW("_original_label_cost is only valid in DSJson"); }
        ctx.original_label_cost_state.aggr_float = &ctx.decision_service_data->original_label_cost;
        ctx.original_label_cost_state.first_slot_float = &ctx.decision_service_data->original_label_cost_first_slot;
        ctx.original_label_cost_state.return_state = this;
        return &ctx.original_label_cost_state;
      }

      else if (ctx.key_length == 5 && !_stricmp(ctx.key, "__aid"))
      {
        ctx.uint_dedup_state.return_state = this;
        return &ctx.uint_dedup_state;
      }

      else if (ctx.key_length == 15 && !strncmp(str, "_definitely_bad", 15))
      {
        if ((ctx.return_path.back())->name == ctx.o_state.name) { return &ctx.definitely_bad_state; }
      }

      return Ignore(ctx, length);
    }

    return this;
  }

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool) override
  {
    // string escape
    const char* end = str + length;
    for (char* p = const_cast<char*>(str); p != end; p++)
    {
      switch (*p)
      {
        case ' ':
        case '\t':
        case '|':
        case ':':
          *p = '_';
      }
    }
    const char* ns = ctx.CurrentNamespace().name;
    if (ctx.ignore_features == nullptr ||
        (ctx.ignore_features->find(ns) == ctx.ignore_features->end() ||
            ctx.ignore_features->at(ns).find(ctx.key) == ctx.ignore_features->at(ns).end()))
    {
      if (ctx._chain_hash) { ctx.CurrentNamespace().add_feature(ctx.key, str, ctx._hash_func, ctx._parse_mask); }
      else
      {
        char* prepend = const_cast<char*>(str) - ctx.key_length;
        memmove(prepend, ctx.key, ctx.key_length);

        ctx.CurrentNamespace().add_feature(prepend, ctx._hash_func, ctx._parse_mask);
      }
    }

    return this;
  }

  BaseState<audit>* Bool(Context<audit>& ctx, bool b) override
  {
    if (b) { ctx.CurrentNamespace().add_feature(ctx.key, ctx._hash_func, ctx._parse_mask); }

    return this;
  }

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    ctx.PushNamespace(ctx.key, this);
    return this;
  }

  BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType memberCount) override
  {
    BaseState<audit>* return_state = ctx.PopNamespace();

    if (!strcmp(return_state->name, ctx.o_state.name))
    {
      return return_state;  // return to observation state
    }

    if (ctx.namespace_path.empty())
    {
      int label_index = ctx.label_index_state.index;
      // we're at the end of the example
      if (label_index >= 0)
      {
        // skip shared example
        label_index++;
        if (label_index >= (int)ctx.examples->size())
        {
          ctx.error() << "Out of bounds error: _labelIndex must be smaller than number of actions! _labelIndex="
                      << (label_index - 1) << " Number of actions=" << ctx.examples->size() - 1 << " ";
          return nullptr;
        }

        // apply labelIndex
        ctx.ex = (*ctx.examples)[label_index];

        // reset for next example
        ctx.label_index_state.index = -1;
      }

      // inject label
      ctx.label_object_state.EndObject(ctx, memberCount);

      // inject observation examples
      if (ctx.observation_example != nullptr) { ctx.examples->push_back(ctx.observation_example); }
      // If we are in CCB mode and there have been no slots. Check label cost, prob and action were passed. In that
      // case this is CB, so generate a single slot with this info.
      if (ctx._label_parser.label_type == VW::label_type_t::CCB)
      {
        auto num_slots = std::count_if(ctx.examples->begin(), ctx.examples->end(),
            [](VW::example* ex) { return ex->l.conditional_contextual_bandit.type == VW::ccb_example_type::SLOT; });
        if (num_slots == 0 && ctx.label_object_state.found_cb)
        {
          ctx.ex = &ctx.example_factory();
          ctx._label_parser.default_label(ctx.ex->l);
          ctx.ex->l.conditional_contextual_bandit.type = VW::ccb_example_type::SLOT;
          ctx.examples->push_back(ctx.ex);

          auto outcome = new VW::ccb_outcome();
          outcome->cost = ctx.label_object_state.cb_label.cost;
          outcome->probabilities.push_back(
              {ctx.label_object_state.cb_label.action - 1, ctx.label_object_state.cb_label.probability});
          ctx.ex->l.conditional_contextual_bandit.outcome = outcome;
        }
      }
    }

    // if we're at the top-level go back to ds_state
    return ctx.namespace_path.empty() ? ctx.root_state : return_state;
  }

  BaseState<audit>* Float(Context<audit>& ctx, float f) override
  {
    auto& ns = ctx.CurrentNamespace();
    auto hash_index = ctx._hash_func(ctx.key, strlen(ctx.key), ns.namespace_hash) & ctx._parse_mask;
    ns.add_feature(f, hash_index, ctx.key);
    return this;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned f) override { return Float(ctx, static_cast<float>(f)); }

  BaseState<audit>* StartArray(Context<audit>& ctx) override { return ctx.array_state.StartArray(ctx); }
};

template <bool audit, typename T>
class ArrayToVectorState : public BaseState<audit>
{
public:
  ArrayToVectorState() : BaseState<audit>("ArrayToVectorState") {}

  std::vector<T>* output_array;
  BaseState<audit>* return_state;

  // Allows for single value handling.
  bool has_seen_array_start = false;

  BaseState<audit>* StartArray(Context<audit>& ctx) override
  {
    if (ctx.previous_state == this)
    {
      ctx.error() << "Nested arrays are not supported";
      return nullptr;
    }

    has_seen_array_start = true;

    return this;
  }

  BaseState<audit>* String(
      Context<audit>& ctx, const char* str, rapidjson::SizeType /*length*/, bool /* copy */) override
  {
    if (_stricmp(str, "NaN") != 0)
    {
      ctx.error() << "The only supported string in the array is 'NaN'";
      return nullptr;
    }

    output_array->push_back(std::numeric_limits<T>::quiet_NaN());

    if (!has_seen_array_start)
    {
      has_seen_array_start = false;
      return return_state;
    }

    return this;
  }

  BaseState<audit>* Uint(Context<audit>& /* ctx */, unsigned f) override
  {
    output_array->push_back(static_cast<T>(f));

    if (!has_seen_array_start)
    {
      has_seen_array_start = false;
      return return_state;
    }

    return this;
  }

  BaseState<audit>* Float(Context<audit>& /* ctx */, float f) override
  {
    output_array->push_back(static_cast<T>(f));

    if (!has_seen_array_start)
    {
      has_seen_array_start = false;
      return return_state;
    }

    return this;
  }

  BaseState<audit>* Null(Context<audit>& /* ctx */) override
  {
    if (!has_seen_array_start)
    {
      has_seen_array_start = false;
      return return_state;
    }

    // ignore null values and stay in current state
    return this;
  }

  BaseState<audit>* EndArray(Context<audit>& /*ctx*/, rapidjson::SizeType /*length*/) override
  {
    has_seen_array_start = false;
    return return_state;
  }
};

template <bool audit>
class StringToStringState : public BaseState<audit>
{
public:
  StringToStringState() : BaseState<audit>("StringToStringState") {}

  std::string* output_string;
  BaseState<audit>* return_state;

  BaseState<audit>* String(
      Context<audit>& /*ctx*/, const char* str, rapidjson::SizeType length, bool /* copy */) override
  {
    output_string->assign(str, str + length);
    return return_state;
  }

  BaseState<audit>* Null(Context<audit>& /*ctx*/) override { return return_state; }
};

// AggrFunc prototype is void (*)(float *input_output, float f);
// Basic Aggregation Types
namespace float_aggregation
{
inline void set(float* output, float f) { *output = f; }
}  // namespace float_aggregation

template <bool audit, void (*func)(float*, float)>
class FloatToFloatState : public BaseState<audit>
{
public:
  FloatToFloatState() : BaseState<audit>("FloatToFloatState") {}

  float* output_float;
  BaseState<audit>* return_state;

  BaseState<audit>* Float(Context<audit>& /*ctx*/, float f) override
  {
    func(output_float, f);
    return return_state;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned i) override { return Float(ctx, static_cast<float>(i)); }

  BaseState<audit>* Null(Context<audit>& /*ctx*/) override
  {
    func(output_float, 0.f);
    return return_state;
  }
};

// HACK: This state object is a complete hack. This object needs to address some very specific business
//       logic which cannot be handled in the state machine in a better way without impacting performance.
//       This level of specificity should NOT be in the parser, do not use this as an example of what to do.
template <bool audit>
class FloatToFloatState_OriginalLabelCostHack : public BaseState<audit>
{
public:
  FloatToFloatState_OriginalLabelCostHack() : BaseState<audit>("FloatToFloatState_OriginalLabelCostHack") {}

  float* aggr_float;
  float* first_slot_float;
  bool seen_first = false;
  BaseState<audit>* return_state;

  BaseState<audit>* Float(Context<audit>& /*ctx*/, float f) override
  {
    *aggr_float += f;
    if (!seen_first)
    {
      seen_first = true;
      *first_slot_float = f;
    }
    return return_state;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned i) override { return Float(ctx, static_cast<float>(i)); }

  BaseState<audit>* Null(Context<audit>& /*ctx*/) override
  {
    // do nothing
    return return_state;
  }
};

template <bool audit>
class UIntDedupState : public BaseState<audit>
{
public:
  UIntDedupState() : BaseState<audit>("UIntDedupState") {}

  uint32_t* output_uint;
  BaseState<audit>* return_state;

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned i) override
  {
    auto* new_ex = ctx.examples->back();

    if (ctx.dedup_examples->find(i) == ctx.dedup_examples->end()) { THROW("dedup id not found: " << i); }

    auto* stored_ex = ctx.dedup_examples->at(i);

    new_ex->indices = stored_ex->indices;
    for (auto& ns : new_ex->indices) { new_ex->feature_space[ns] = stored_ex->feature_space[ns]; }
    new_ex->ft_offset = stored_ex->ft_offset;
    return return_state;
  }
};

template <bool audit>
class UIntToUIntState : public BaseState<audit>
{
public:
  UIntToUIntState() : BaseState<audit>("UIntToUIntState") {}

  uint32_t* output_uint;
  BaseState<audit>* return_state;

  BaseState<audit>* Uint(Context<audit>& /*ctx*/, unsigned i) override
  {
    *output_uint = i;
    return return_state;
  }
};

template <bool audit>
class BoolToBoolState : public BaseState<audit>
{
public:
  BoolToBoolState() : BaseState<audit>("BoolToBoolState") {}

  bool* output_bool;
  BaseState<audit>* return_state;

  BaseState<audit>* Bool(Context<audit>& /*ctx*/, bool b) override
  {
    *output_bool = b;
    return return_state;
  }
};

template <bool audit>
class SlotOutcomeList : public BaseState<audit>
{
public:
  VW::parsers::json::decision_service_interaction* interactions;

  SlotOutcomeList() : BaseState<audit>("SlotOutcomeList") {}

  BaseState<audit>* StartArray(Context<audit>& ctx) override
  {
    slot_object_index = 0;

    // Find start index of slot objects by iterating until we find the first slot example.
    for (auto ex : *ctx.examples)
    {
      if ((ctx._label_parser.label_type == VW::label_type_t::CCB &&
              ex->l.conditional_contextual_bandit.type != VW::ccb_example_type::SLOT) ||
          (ctx._label_parser.label_type == VW::label_type_t::SLATES &&
              ex->l.slates.type != VW::slates::example_type::SLOT))
      {
        slot_object_index++;
      }
    }
    old_root = ctx.root_state;
    ctx.root_state = this;

    if (slot_object_index == 0) { THROW("Badly formed ccb example. Shared example is required.") }

    return this;
  }

  BaseState<audit>* StartObject(Context<audit>& ctx) override
  {
    // Set current example so that default state correctly sets the label.
    ctx.ex = (*ctx.examples)[slot_object_index];
    // The end object logic assumes shared example so we need to take one here.
    ctx.label_index_state.index = slot_object_index - 1;

    slot_object_index++;

    // Push a namespace so that default state can get back here when it reaches the end of the object.
    ctx.PushNamespace(" ", this);

    return &ctx.default_state;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType) override
  {
    // DSJson requires the interaction object to be filled. After reading all slot outcomes fill out the top actions.
    for (auto ex : *ctx.examples)
    {
      if (ctx._label_parser.label_type == VW::label_type_t::CCB &&
          ex->l.conditional_contextual_bandit.type == VW::ccb_example_type::SLOT)
      {
        if (ex->l.conditional_contextual_bandit.outcome)
        {
          interactions->actions.push_back(ex->l.conditional_contextual_bandit.outcome->probabilities[0].action);
          interactions->probabilities.push_back(ex->l.conditional_contextual_bandit.outcome->probabilities[0].score);
        }
      }
      else if (ctx._label_parser.label_type == VW::label_type_t::SLATES &&
          ex->l.slates.type == VW::slates::example_type::SLOT)
      {
        if (ex->l.slates.labeled)
        {
          interactions->actions.push_back(ex->l.slates.probabilities[0].action);
          interactions->probabilities.push_back(ex->l.slates.probabilities[0].score);
        }
      }
    }

    ctx.root_state = old_root;
    return &ctx.decision_service_state;
  }

private:
  int slot_object_index = 0;

  std::vector<uint32_t> actions;
  std::vector<float> probs;
  float cost;

  BaseState<audit>* old_root;
};

template <bool audit>
class DecisionServiceState : public BaseState<audit>
{
public:
  DecisionServiceState() : BaseState<audit>("DecisionService") {}

  VW::parsers::json::decision_service_interaction* data;

  BaseState<audit>* StartObject(Context<audit>& /* ctx */) override
  {
    // TODO: improve validation
    return this;
  }

  BaseState<audit>* EndObject(Context<audit>& /*ctx*/, rapidjson::SizeType /* memberCount */) override
  {
    // TODO: improve validation
    return this;
  }

  BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool /* copy */) override
  {
    if (length == 1)
    {
      switch (str[0])
      {
        case 'a':
          ctx.array_uint_state.output_array = &data->actions;
          ctx.array_uint_state.return_state = this;
          return &ctx.array_uint_state;
        case 'p':
          data->probabilities.clear();
          ctx.array_float_state.output_array = &data->probabilities;
          ctx.array_float_state.return_state = this;
          return &ctx.array_float_state;
        case 'c':
          ctx.key = " ";
          ctx.key_length = 1;
          return &ctx.default_state;
        case 'o':
          if (ctx._label_parser.label_type == VW::label_type_t::CB_WITH_OBSERVATIONS)
          {
            ctx.key = " ";
            ctx.key_length = 1;

            // allocate new example
            ctx.ex = &ctx.example_factory();
            ctx._label_parser.default_label(ctx.ex->l);
            ctx.ex->l.cb_with_observations.is_observation = true;
            ctx.observation_example = ctx.ex;

            return &ctx.o_state;
          }
      }
    }
    else if (length == 3 && !strcmp(str, "pdf"))
    {
      ctx.array_pdf_state.return_state = this;
      return &ctx.array_pdf_state;
    }
    else if (length == 5 && !strcmp(str, "pdrop"))
    {
      ctx.float_state.output_float = &data->probability_of_drop;
      ctx.float_state.return_state = this;
      return &ctx.float_state;
    }
    else if (length == 7 && !strcmp(str, "EventId"))
    {
      ctx.string_state.output_string = &data->event_id;
      ctx.string_state.return_state = this;
      return &ctx.string_state;
    }
    else if (length == 9 && !strcmp(str, "Timestamp"))
    {
      ctx.string_state.output_string = &data->timestamp;
      ctx.string_state.return_state = this;
      return &ctx.string_state;
    }
    else if (length > 0 && str[0] == '_')
    {
      // match _label*
      if (length >= 6 && !strncmp(str, "_label", 6))
      {
        ctx.key = str;
        ctx.key_length = length;
        if (length >= 7 && ctx.key[6] == '_')
        {
          if (length >= 9 && !strncmp(&ctx.key[7], "ca", 2)) { ctx.label_object_state.found_cb_continuous = true; }
          return &ctx.label_single_property_state;
        }
        else if (length == 6) { return &ctx.label_state; }
        else if (length == 11 && !_stricmp(str, "_labelIndex")) { return &ctx.label_index_state; }
      }
      else if (length == 10 && !strncmp(str, "_skipLearn", 10))
      {
        ctx.bool_state.output_bool = &data->skip_learn;
        ctx.bool_state.return_state = this;
        return &ctx.bool_state;
      }
      else if (length == 9 && !strncmp(str, "_outcomes", 9))
      {
        ctx.slot_outcome_list_state.interactions = data;
        return &ctx.slot_outcome_list_state;
      }
      else if (length == 2 && !strncmp(str, "_p", 2))
      {
        data->probabilities.clear();
        ctx.array_float_state.output_array = &data->probabilities;
        ctx.array_float_state.return_state = this;
        return &ctx.array_float_state;
      }
      else if (length == 20 && !strncmp(str, "_original_label_cost", 20))
      {
        ctx.original_label_cost_state.aggr_float = &data->original_label_cost;
        ctx.original_label_cost_state.first_slot_float = &data->original_label_cost_first_slot;
        ctx.original_label_cost_state.return_state = this;
        return &ctx.original_label_cost_state;
      }
      else if (length == 3 && !strncmp(str, "_ba", 3))
      {
        ctx.array_uint_state.output_array = &data->baseline_actions;
        ctx.array_uint_state.return_state = this;
        return &ctx.array_uint_state;
      }
    }

    // ignore unknown properties
    return ctx.default_state.Ignore(ctx, length);
  }
};

template <bool audit>
class Context
{
public:
  VW::label_parser _label_parser;
  VW::hash_func_t _hash_func;
  uint64_t _hash_seed;
  uint64_t _parse_mask;
  bool _chain_hash;

  VW::label_parser_reuse_mem* _reuse_mem;
  const VW::named_labels* _ldict;
  VW::io::logger* _logger;

  // last "<key>": encountered
  const char* key;
  rapidjson::SizeType key_length;

  BaseState<audit>* current_state;
  BaseState<audit>* previous_state;

  // the path of namespaces
  std::vector<VW::parsers::json::details::namespace_builder<audit>> namespace_path;
  std::vector<BaseState<audit>*> return_path;

  const std::unordered_map<uint64_t, VW::example*>* dedup_examples;
  std::unordered_map<std::string, std::set<std::string>>* ignore_features = nullptr;

  VW::multi_ex* examples;
  VW::example* observation_example = nullptr;
  VW::example* ex;
  rapidjson::InsituStringStream* stream;
  const char* stream_end;

  VW::example_factory_t example_factory;

  // TODO: This shouldn't really exist in the Context. Once the JSON parser
  // gets refactored to separate the VWJson/DSJson concepts, this should
  // be moved into the DSJson version of the context
  VW::parsers::json::decision_service_interaction* decision_service_data = nullptr;

  // states
  DefaultState<audit> default_state;
  LabelState<audit> label_state;
  LabelObjectState<audit> label_object_state;
  LabelSinglePropertyState<audit> label_single_property_state;
  LabelIndexState<audit> label_index_state;
  TextState<audit> text_state;
  TagState<audit> tag_state;
  MultiState<audit> multi_state;
  ObservationState<audit> o_state;
  DefinitelyBadState<audit> definitely_bad_state;
  IgnoreState<audit> ignore_state;
  ArrayState<audit> array_state;
  SlotsState<audit> slots_state;
  ArrayToPdfState<audit> array_pdf_state;
  ArrayToGraphState<audit> array_to_graph_state;

  // DecisionServiceState
  DecisionServiceState<audit> decision_service_state;
  ArrayToVectorState<audit, float> array_float_state;
  ArrayToVectorState<audit, unsigned> array_uint_state;
  StringToStringState<audit> string_state;
  FloatToFloatState<audit, float_aggregation::set> float_state;
  FloatToFloatState_OriginalLabelCostHack<audit> original_label_cost_state;
  UIntToUIntState<audit> uint_state;
  UIntDedupState<audit> uint_dedup_state;
  BoolToBoolState<audit> bool_state;
  SlotOutcomeList<audit> slot_outcome_list_state;

  BaseState<audit>* root_state;

  Context()
  {
    current_state = &default_state;
    root_state = &default_state;
  }

  void init(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask,
      bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict, VW::io::logger* logger)
  {
    assert(reuse_mem != nullptr);
    assert(logger != nullptr);
    _label_parser = lbl_parser;
    _hash_func = hash_func;
    _hash_seed = hash_seed;
    _parse_mask = parse_mask;
    _chain_hash = chain_hash;
    _reuse_mem = reuse_mem;
    _ldict = ldict;
    _logger = logger;

    key = " ";
    key_length = 1;
    previous_state = nullptr;
  }

  std::stringstream& error()
  {
    if (!error_ptr) { error_ptr.reset(new std::stringstream{}); }

    return *error_ptr;
  }

  void SetStartStateToDecisionService(VW::parsers::json::decision_service_interaction* data)
  {
    decision_service_state.data = data;
    current_state = root_state = &decision_service_state;
  }

  void PushNamespace(const char* ns, BaseState<audit>* return_state)
  {
    push_ns(ex, ns, namespace_path, _hash_func, _hash_seed);
    return_path.push_back(return_state);
  }

  BaseState<audit>* PopNamespace()
  {
    pop_ns(ex, namespace_path);
    auto return_state = return_path.back();
    return_path.pop_back();
    return return_state;
  }

  VW::parsers::json::details::namespace_builder<audit>& CurrentNamespace() { return namespace_path.back(); }

  bool TransitionState(BaseState<audit>* next_state)
  {
    if (next_state == nullptr) { return false; }

    previous_state = current_state;
    current_state = next_state;

    return true;
  }

private:
  std::unique_ptr<std::stringstream> error_ptr;
};

template <bool audit>
class VWReaderHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, VWReaderHandler<audit>>
{
public:
  Context<audit> ctx;

  void init(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask,
      bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict, VW::io::logger* logger,
      VW::multi_ex* examples, rapidjson::InsituStringStream* stream, const char* stream_end,
      VW::example_factory_t example_factory, std::unordered_map<std::string, std::set<std::string>>* ignore_features,
      const std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr)
  {
    ctx.init(lbl_parser, hash_func, hash_seed, parse_mask, chain_hash, reuse_mem, ldict, logger);
    ctx.examples = examples;
    ctx.ex = (*examples)[0];
    lbl_parser.default_label(ctx.ex->l);

    ctx.stream = stream;
    ctx.stream_end = stream_end;
    ctx.example_factory = std::move(example_factory);
    ctx.dedup_examples = dedup_examples;
    ctx.ignore_features = ignore_features;
  }

  // virtual dispatch to current state
  bool Bool(bool v) { return ctx.TransitionState(ctx.current_state->Bool(ctx, v)); }
  bool Int(int v) { return ctx.TransitionState(ctx.current_state->Float(ctx, static_cast<float>(v))); }
  bool Uint(unsigned v) { return ctx.TransitionState(ctx.current_state->Uint(ctx, v)); }
  bool Int64(int64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, static_cast<float>(v))); }
  bool Uint64(uint64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, static_cast<float>(v))); }
  bool Double(double v) { return ctx.TransitionState(ctx.current_state->Float(ctx, static_cast<float>(v))); }
  bool String(const char* str, SizeType len, bool copy)
  {
    return ctx.TransitionState(ctx.current_state->String(ctx, str, len, copy));
  }
  bool StartObject() { return ctx.TransitionState(ctx.current_state->StartObject(ctx)); }
  bool Key(const char* str, SizeType len, bool copy)
  {
    return ctx.TransitionState(ctx.current_state->Key(ctx, str, len, copy));
  }
  bool EndObject(SizeType count) { return ctx.TransitionState(ctx.current_state->EndObject(ctx, count)); }
  bool StartArray() { return ctx.TransitionState(ctx.current_state->StartArray(ctx)); }
  bool EndArray(SizeType count) { return ctx.TransitionState(ctx.current_state->EndArray(ctx, count)); }
  bool Null() { return ctx.TransitionState(ctx.current_state->Null(ctx)); }

  bool VWReaderHandlerNull() { return true; }
  bool VWReaderHandlerDefault() { return false; }

  // alternative to above if we want to re-use the VW float parser...
  bool RawNumber(const char* /* str */, rapidjson::SizeType /* length */, bool /* copy */) { return false; }

  std::stringstream& error() { return ctx.error(); }

  BaseState<audit>* current_state() { return ctx.current_state; }
};

template <bool audit>
class json_parser
{
public:
  rapidjson::Reader reader;
  VWReaderHandler<audit> handler;
};
}  // namespace

template <bool audit>
void VW::parsers::json::read_line_json(const VW::label_parser& lbl_parser, hash_func_t hash_func, uint64_t hash_seed,
    uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict,
    VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory, VW::io::logger& logger,
    std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    const std::unordered_map<uint64_t, VW::example*>* dedup_examples)
{
  if (lbl_parser.label_type == VW::label_type_t::SLATES)
  {
    VW::parsers::json::details::parse_slates_example_json<audit>(lbl_parser, hash_func, hash_seed, parse_mask,
        chain_hash, examples, line, length, std::move(example_factory), dedup_examples);
    return;
  }

  // string line_copy(line);
  // destructive parsing
  InsituStringStream ss(line);
  json_parser<audit> parser;

  VWReaderHandler<audit>& handler = parser.handler;

  handler.init(lbl_parser, hash_func, hash_seed, parse_mask, chain_hash, reuse_mem, ldict, &logger, &examples, &ss,
      line + length, example_factory, ignore_features, dedup_examples);

  ParseResult result =
      parser.reader.template Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler<audit>>(ss, handler);
  if (!result.IsError()) { return; }

  BaseState<audit>* current_state = handler.current_state();

  // The stack of namespaces must be drained so there are no half extents left around.
  while (!handler.ctx.namespace_path.empty()) { handler.ctx.PopNamespace(); }

  THROW("JSON parser error at " << result.Offset() << ": " << GetParseError_En(result.Code())
                                << ". "
                                   "Handler: "
                                << handler.error().str()
                                << "State: " << (current_state ? current_state->name : "null"));  // <<
  // "Line: '"<< line_copy << "'");
}

template <bool audit>
void VW::parsers::json::read_line_json(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    example_factory_t example_factory, const std::unordered_map<uint64_t, VW::example*>* dedup_examples)
{
  return read_line_json<audit>(all.parser_runtime.example_parser->lbl_parser, all.parser_runtime.example_parser->hasher,
      all.runtime_config.hash_seed, all.runtime_state.parse_mask, all.parser_runtime.chain_hash_json,
      &all.parser_runtime.example_parser->parser_memory_to_reuse, all.sd->ldict.get(), examples, line, length,
      std::move(example_factory), all.logger, &all.feature_tweaks_config.ignore_features_dsjson, dedup_examples);
}

inline bool apply_pdrop(VW::label_type_t label_type, float pdrop, VW::multi_ex& examples, VW::io::logger& logger)
{
  if (pdrop == 1.)
  {
    logger.err_error("JSON parser error: examples with pdrop==1 are not supported");
    return false;
  }
  // Event with certain pdrop had (1-pdrop) as probability to survive,
  // so it is one of (1 / (1-pdrop)) events that we should learn on, and weight should be updated accordingly.
  if (label_type == VW::label_type_t::CB)
  {
    for (auto& e : examples) { e->l.cb.weight /= 1 - pdrop; }
  }
  else if (label_type == VW::label_type_t::CCB)
  {
    for (auto& e : examples) { e->l.conditional_contextual_bandit.weight /= 1 - pdrop; }
  }
  if (label_type == VW::label_type_t::SLATES)
  {
    // TODO
  }
  return true;
}

// returns true if succesfully parsed, returns false if not and logs warning
template <bool audit>
bool VW::parsers::json::read_line_decision_service_json(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data)
{
  if (all.parser_runtime.example_parser->lbl_parser.label_type == VW::label_type_t::SLATES)
  {
    VW::parsers::json::details::parse_slates_example_dsjson<audit>(
        all, examples, line, length, std::move(example_factory), data);
    return apply_pdrop(
        all.parser_runtime.example_parser->lbl_parser.label_type, data->probability_of_drop, examples, all.logger);
  }

  std::vector<char> line_vec;
  if (copy_line)
  {
    line_vec.insert(line_vec.end(), line, line + length);
    line = &line_vec.front();
  }

  InsituStringStream ss(line);
  json_parser<audit> parser;

  VWReaderHandler<audit>& handler = parser.handler;
  handler.init(all.parser_runtime.example_parser->lbl_parser, all.parser_runtime.example_parser->hasher,
      all.runtime_config.hash_seed, all.runtime_state.parse_mask, all.parser_runtime.chain_hash_json,
      &all.parser_runtime.example_parser->parser_memory_to_reuse, all.sd->ldict.get(), &all.logger, &examples, &ss,
      line + length, example_factory, &all.feature_tweaks_config.ignore_features_dsjson);

  handler.ctx.SetStartStateToDecisionService(data);
  handler.ctx.decision_service_data = data;

  ParseResult result =
      parser.reader.template Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler<audit>>(ss, handler);

  if (result.IsError())
  {
    BaseState<audit>* current_state = handler.current_state();

    // The stack of namespaces must be drained so there are no half extents left around.
    while (!handler.ctx.namespace_path.empty()) { handler.ctx.PopNamespace(); }

    if (all.parser_runtime.example_parser->strict_parse)
    {
      THROW("JSON parser error at " << result.Offset() << ": " << GetParseError_En(result.Code())
                                    << ". "
                                       "Handler: "
                                    << handler.error().str()
                                    << "State: " << (current_state ? current_state->name : "null"));
    }
    else
    {
      all.logger.err_error("JSON parser error at {0}: {1}. Handler: {2} State: {3}", result.Offset(),
          GetParseError_En(result.Code()), handler.error().str(), (current_state ? current_state->name : "null"));
      return false;
    }
  }

  return apply_pdrop(
      all.parser_runtime.example_parser->lbl_parser.label_type, data->probability_of_drop, examples, all.logger);
}

template <bool audit>
bool VW::parsers::json::details::parse_line_json(
    VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples)
{
  if (all->parser_runtime.example_parser->decision_service_json)
  {
    // Skip lines that do not start with "{"
    if (line[0] != '{') { return false; }

    VW::parsers::json::decision_service_interaction interaction;
    bool result = VW::parsers::json::template read_line_decision_service_json<audit>(
        *all, examples, line, num_chars, false, [all]() -> VW::example& { return VW::get_unused_example(all); },
        &interaction);

    if (!result)
    {
      VW::return_multiple_example(*all, examples);
      examples.push_back(&VW::get_unused_example(all));
      if (all->parser_runtime.example_parser->metrics)
      {
        all->parser_runtime.example_parser->metrics->line_parse_error++;
      }
      return false;
    }

    if (all->parser_runtime.example_parser->metrics)
    {
      if (!interaction.event_id.empty())
      {
        if (all->parser_runtime.example_parser->metrics->first_event_id.empty())
        {
          all->parser_runtime.example_parser->metrics->first_event_id = std::move(interaction.event_id);
        }
        else { all->parser_runtime.example_parser->metrics->last_event_id = std::move(interaction.event_id); }
      }

      if (!interaction.timestamp.empty())
      {
        if (all->parser_runtime.example_parser->metrics->first_event_time.empty())
        {
          all->parser_runtime.example_parser->metrics->first_event_time = std::move(interaction.timestamp);
        }
        else { all->parser_runtime.example_parser->metrics->last_event_time = std::move(interaction.timestamp); }
      }

      // Technically the aggregation operation here is supposed to be user-defined
      // but according to Casey, the only operation used is Sum
      // The _original_label_cost element is found either at the top level OR under
      // the _outcomes node (for CCB)
      all->parser_runtime.example_parser->metrics->dsjson_sum_cost_original += interaction.original_label_cost;
      all->parser_runtime.example_parser->metrics->dsjson_sum_cost_original_first_slot +=
          interaction.original_label_cost_first_slot;
      if (!interaction.actions.empty())
      {
        // APS requires this metric for CB (baseline action is 1)
        if (interaction.actions[0] == 1)
        {
          all->parser_runtime.example_parser->metrics->dsjson_sum_cost_original_baseline +=
              interaction.original_label_cost;
        }

        if (!interaction.baseline_actions.empty())
        {
          if (interaction.actions[0] == interaction.baseline_actions[0])
          {
            all->parser_runtime.example_parser->metrics->dsjson_number_of_label_equal_baseline_first_slot++;
            all->parser_runtime.example_parser->metrics->dsjson_sum_cost_original_label_equal_baseline_first_slot +=
                interaction.original_label_cost_first_slot;
          }
          else { all->parser_runtime.example_parser->metrics->dsjson_number_of_label_not_equal_baseline_first_slot++; }
        }
      }
    }

    // TODO: In refactoring the parser to be usable standalone, we need to ensure that we
    // stop suppressing "skip_learn" interactions. Also, not sure if this is the right logic
    // for counterfactual. (@marco)
    if (interaction.skip_learn)
    {
      if (all->parser_runtime.example_parser->metrics)
      {
        all->parser_runtime.example_parser->metrics->number_of_skipped_events++;
      }
      VW::return_multiple_example(*all, examples);
      examples.push_back(&VW::get_unused_example(all));
      return false;
    }

    // let's ask to continue reading data until we find a line with actions provided
    if (interaction.actions.size() == 0 && all->l->is_multiline())
    {
      if (all->parser_runtime.example_parser->metrics)
      {
        all->parser_runtime.example_parser->metrics->number_of_events_zero_actions++;
      }
      VW::return_multiple_example(*all, examples);
      examples.push_back(&VW::get_unused_example(all));
      return false;
    }
  }
  else
  {
    VW::parsers::json::template read_line_json<audit>(
        *all, examples, line, num_chars, [all]() -> VW::example& { return VW::get_unused_example(all); });
  }

  return true;
}

inline void append_empty_newline_example_for_driver(VW::workspace* all, VW::multi_ex& examples)
{
  // note: the json parser does single pass parsing and cannot determine if a shared example is needed.
  // since the communication between the parsing thread the main learner expects examples to be requested in order (as
  // they're layed out in memory) there is no way to determine upfront if a shared example exists thus even if there are
  // no features for the shared example, still an empty example is returned.

  // insert new line example at the end
  if (examples.size() > 1)
  {
    VW::example& ae = VW::get_unused_example(all);
    static const char empty[] = "";
    VW::string_view example(empty);
    VW::parsers::text::details::substring_to_example(all, &ae, example);
    ae.is_newline = true;

    examples.push_back(&ae);
  }
}

// This is used by the python parser
template <bool audit>
void VW::parsers::json::line_to_examples_json(VW::workspace* all, VW::string_view line_view, VW::multi_ex& examples)
{
  // The JSON reader does insitu parsing and therefore modifies the input
  // string, so we make a copy since this function cannot modify the input
  // string.
  std::vector<char> owned_str;
  size_t len = line_view.size() + 1;
  owned_str.resize(len);
  std::memcpy(owned_str.data(), line_view.data(), len);

  bool good_example =
      VW::parsers::json::details::parse_line_json<audit>(all, owned_str.data(), line_view.size(), examples);
  if (!good_example)
  {
    VW::return_multiple_example(*all, examples);
    examples.push_back(&VW::get_unused_example(all));
    return;
  }
}

template <bool audit>
int VW::parsers::json::read_features_json(VW::workspace* all, io_buf& buf, VW::multi_ex& examples)
{
  // Keep reading lines until a valid set of examples is produced.
  bool reread;
  do {
    reread = false;

    char* line;
    size_t num_chars;
    size_t num_chars_initial = VW::parsers::text::details::read_features(buf, line, num_chars);
    if (num_chars_initial < 1) { return static_cast<int>(num_chars_initial); }

    // Ensure there is a null terminator.
    line[num_chars] = '\0';

    reread = !VW::parsers::json::details::parse_line_json<audit>(all, line, num_chars, examples);
  } while (reread);

  append_empty_newline_example_for_driver(all, examples);

  return 1;
}

// Explicitly instantiate templates only in this source file
template void VW::parsers::json::read_line_json<true>(const VW::label_parser& lbl_parser, hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem,
    const VW::named_labels* ldict, VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory,
    VW::io::logger& logger, std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    const std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::parsers::json::read_line_json<false>(const VW::label_parser& lbl_parser, hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem,
    const VW::named_labels* ldict, VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory,
    VW::io::logger& logger, std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    const std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template void VW::parsers::json::read_line_json<true>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, example_factory_t example_factory, const std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::parsers::json::read_line_json<false>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, example_factory_t example_factory, const std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template bool VW::parsers::json::read_line_decision_service_json<true>(VW::workspace& all, VW::multi_ex& examples,
    char* line, size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data);
template bool VW::parsers::json::read_line_decision_service_json<false>(VW::workspace& all, VW::multi_ex& examples,
    char* line, size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data);

template bool VW::parsers::json::details::parse_line_json<true>(
    VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);
template bool VW::parsers::json::details::parse_line_json<false>(
    VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);

template void VW::parsers::json::line_to_examples_json<true>(
    VW::workspace* all, VW::string_view sv, VW::multi_ex& examples);
template void VW::parsers::json::line_to_examples_json<false>(
    VW::workspace* all, VW::string_view sv, VW::multi_ex& examples);

template int VW::parsers::json::read_features_json<true>(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);
template int VW::parsers::json::read_features_json<false>(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);
