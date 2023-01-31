// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
/*
  Initial implementation by Hal Daume and John Langford.  Reimplementation
  by John Langford.
*/

#include "vw/core/reductions/ect.h"

#include "vw/config/options.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/parser.h"
#include "vw/core/setup_base.h"
#include "vw/io/logger.h"

#include <fmt/core.h>

#include <cfloat>
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class direction
{
public:
  size_t id;          // unique id for node
  size_t tournament;  // unique id for node
  uint32_t winner;    // up traversal, winner
  uint32_t loser;     // up traversal, loser
  uint32_t left;      // down traversal, left
  uint32_t right;     // down traversal, right
  bool last;
};

class ect
{
public:
  uint64_t k = 0;
  uint64_t errors = 0;
  float class_boundary = 0.f;

  VW::v_array<direction> directions;  // The nodes of the tournament datastructure

  std::vector<std::vector<VW::v_array<uint32_t>>> all_levels;

  VW::v_array<uint32_t> final_nodes;  // The final nodes of each tournament.

  VW::v_array<size_t> up_directions;    // On edge e, which node n is in the up direction?
  VW::v_array<size_t> down_directions;  // On edge e, which node n is in the down direction?

  size_t tree_height = 0;  // The height of the final tournament.

  uint32_t last_pair = 0;

  VW::v_array<bool> tournaments_won;
  VW::io::logger logger;

  explicit ect(VW::io::logger logger) : logger(std::move(logger)) {}
};

size_t final_depth(size_t eliminations, VW::io::logger& logger)
{
  eliminations--;
  for (size_t i = 0; i < 32; i++)
  {
    if (eliminations >> i == 0) { return i; }
  }
  logger.err_error("too many eliminations");
  return 31;
}

bool not_empty(std::vector<VW::v_array<uint32_t>> const& tournaments)
{
  auto const first_non_empty_tournament = std::find_if(tournaments.cbegin(), tournaments.cend(),
      [](const VW::v_array<uint32_t>& tournament) { return !tournament.empty(); });
  return first_non_empty_tournament != tournaments.cend();
}

size_t create_circuit(ect& e, uint64_t max_label, uint64_t eliminations)
{
  if (max_label == 1) { return 0; }

  std::vector<VW::v_array<uint32_t>> tournaments;
  VW::v_array<uint32_t> t;

  for (uint32_t i = 0; i < max_label; i++)
  {
    t.push_back(i);
    direction d = {i, 0, 0, 0, 0, 0, false};
    e.directions.push_back(d);
  }

  tournaments.push_back(t);

  for (size_t i = 0; i < eliminations - 1; i++) { tournaments.push_back(VW::v_array<uint32_t>()); }

  e.all_levels.push_back(tournaments);

  size_t level = 0;

  uint32_t node = static_cast<uint32_t>(e.directions.size());

  while (not_empty(e.all_levels[level]))
  {
    std::vector<VW::v_array<uint32_t>> new_tournaments;
    tournaments = e.all_levels[level];

    for (size_t i = 0; i < tournaments.size(); i++)
    {
      VW::v_array<uint32_t> empty;
      new_tournaments.push_back(empty);
    }

    for (size_t i = 0; i < tournaments.size(); i++)
    {
      for (size_t j = 0; j < tournaments[i].size() / 2; j++)
      {
        uint32_t id = node++;
        uint32_t left = tournaments[i][2 * j];
        uint32_t right = tournaments[i][2 * j + 1];

        direction d = {id, i, 0, 0, left, right, false};
        e.directions.push_back(d);
        uint32_t direction_index = static_cast<uint32_t>(e.directions.size()) - 1;
        if (e.directions[left].tournament == i) { e.directions[left].winner = direction_index; }
        else { e.directions[left].loser = direction_index; }
        if (e.directions[right].tournament == i) { e.directions[right].winner = direction_index; }
        else { e.directions[right].loser = direction_index; }
        if (e.directions[left].last) { e.directions[left].winner = direction_index; }

        if (tournaments[i].size() == 2 && (i == 0 || tournaments[i - 1].empty()))
        {
          e.directions[direction_index].last = true;
          if (i + 1 < tournaments.size()) { new_tournaments[i + 1].push_back(id); }
          else
          {  // winner eliminated.
            e.directions[direction_index].winner = 0;
          }
          e.final_nodes.push_back(static_cast<uint32_t>(e.directions.size() - 1));
        }
        else { new_tournaments[i].push_back(id); }
        if (i + 1 < tournaments.size()) { new_tournaments[i + 1].push_back(id); }
        else
        {  // loser eliminated.
          e.directions[direction_index].loser = 0;
        }
      }
      if (tournaments[i].size() % 2 == 1) { new_tournaments[i].push_back(tournaments[i].back()); }
    }
    e.all_levels.push_back(new_tournaments);
    level++;
  }

  e.last_pair = static_cast<uint32_t>((max_label - 1) * eliminations);

  if (max_label > 1) { e.tree_height = final_depth(eliminations, e.logger); }

  return e.last_pair + (eliminations - 1);
}

uint32_t ect_predict(ect& e, learner& base, VW::example& ec)
{
  if (e.k == static_cast<size_t>(1)) { return 1; }

  uint32_t finals_winner = 0;

  // Binary final elimination tournament first
  ec.l.simple = {FLT_MAX};
  ec.ex_reduction_features.template get<VW::simple_label_reduction_features>().reset_to_default();

  for (size_t i = e.tree_height - 1; i != static_cast<size_t>(0) - 1; i--)
  {
    if ((finals_winner | ((static_cast<size_t>(1)) << i)) <= e.errors)
    {
      // a real choice exists
      uint32_t problem_number =
          e.last_pair + (finals_winner | ((static_cast<uint32_t>(1)) << i)) - 1;  // This is unique.

      base.predict(ec, problem_number);

      if (ec.pred.scalar > e.class_boundary) { finals_winner = finals_winner | ((static_cast<size_t>(1)) << i); }
    }
  }

  uint32_t id = e.final_nodes[finals_winner];
  while (id >= e.k)
  {
    base.predict(ec, id - e.k);

    if (ec.pred.scalar > e.class_boundary) { id = e.directions[id].right; }
    else { id = e.directions[id].left; }
  }
  return id + 1;
}

void ect_train(ect& e, learner& base, VW::example& ec)
{
  if (e.k == 1)
  {  // nothing to do
    return;
  }
  VW::multiclass_label mc = ec.l.multi;

  VW::simple_label simple_temp;

  e.tournaments_won.clear();

  uint32_t id = e.directions[mc.label - 1].winner;
  bool left = e.directions[id].left == mc.label - 1;
  do {
    if (left) { simple_temp.label = -1; }
    else { simple_temp.label = 1; }

    ec.l.simple = simple_temp;
    base.learn(ec, id - e.k);
    float old_weight = ec.weight;
    ec.weight = 0.;
    base.learn(ec, id - e.k);  // inefficient, we should extract final prediction exactly.
    ec.weight = old_weight;

    bool won = (ec.pred.scalar - e.class_boundary) * simple_temp.label > 0;

    if (won)
    {
      if (!e.directions[id].last) { left = e.directions[e.directions[id].winner].left == id; }
      else { e.tournaments_won.push_back(true); }
      id = e.directions[id].winner;
    }
    else
    {
      if (!e.directions[id].last)
      {
        left = e.directions[e.directions[id].loser].left == id;
        if (e.directions[id].loser == 0) { e.tournaments_won.push_back(false); }
      }
      else { e.tournaments_won.push_back(false); }
      id = e.directions[id].loser;
    }
  } while (id != 0);

  // TODO: error? warn? info? what level is this supposed to be?
  if (e.tournaments_won.empty())
  {
    e.logger.out_error("Internal error occurred. tournaments_won was empty which should not be possible.");
  }

  // tournaments_won is a bit vector determining which tournaments the label won.
  for (size_t i = 0; i < e.tree_height; i++)
  {
    for (uint32_t j = 0; j < e.tournaments_won.size() / 2; j++)
    {
      left = e.tournaments_won[j * 2];
      bool right = e.tournaments_won[j * 2 + 1];
      if (left == right)
      {  // no query to do
        e.tournaments_won[j] = left;
      }
      else  // query to do
      {
        if (left) { simple_temp.label = -1; }
        else { simple_temp.label = 1; }
        ec.l.simple = simple_temp;
        ec.weight = static_cast<float>(1 << (e.tree_height - i - 1));

        uint32_t problem_number = e.last_pair + j * (1 << (i + 1)) + (1 << i) - 1;

        base.learn(ec, problem_number);

        if (ec.pred.scalar > e.class_boundary) { e.tournaments_won[j] = right; }
        else { e.tournaments_won[j] = left; }
      }

      if (e.tournaments_won.size() % 2 == 1)
      {
        e.tournaments_won[e.tournaments_won.size() / 2] = e.tournaments_won[e.tournaments_won.size() - 1];
      }
      e.tournaments_won.resize((1 + e.tournaments_won.size()) / 2);
    }
  }
}

void predict(ect& e, learner& base, VW::example& ec) { ec.pred.multiclass = ect_predict(e, base, ec); }

void learn(ect& e, learner& base, VW::example& ec)
{
  VW::multiclass_label mc = ec.l.multi;
  uint32_t pred = ec.pred.multiclass;

  if (mc.label != static_cast<uint32_t>(-1)) { ect_train(e, base, ec); }
  ec.l.multi = mc;
  ec.pred.multiclass = pred;
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::ect_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  auto data = VW::make_unique<ect>(all.logger);
  std::string link;
  option_group_definition new_options("[Reduction] Error Correcting Tournament");
  new_options.add(make_option("ect", data->k).keep().necessary().help("Error correcting tournament with <k> labels"))
      .add(make_option("error", data->errors).keep().default_value(0).help("Errors allowed by ECT"))
      // Used to check value. TODO replace
      .add(make_option("link", link)
               .default_value("identity")
               .keep()
               .one_of({"identity", "logistic", "glf1", "poisson"})
               .help("Specify the link function"));

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  size_t wpp = create_circuit(*data.get(), data->k, data->errors + 1);

  auto base = stack_builder.setup_base_learner();
  if (link == "logistic")
  {
    data->class_boundary = 0.5;  // as --link=logistic maps predictions in [0;1]
  }

  auto l = make_reduction_learner(
      std::move(data), require_singleline(base), learn, predict, stack_builder.get_setupfn_name(ect_setup))
               .set_params_per_weight(wpp)
               .set_input_label_type(VW::label_type_t::MULTICLASS)
               .set_output_label_type(VW::label_type_t::SIMPLE)
               .set_input_prediction_type(VW::prediction_type_t::SCALAR)
               .set_output_prediction_type(VW::prediction_type_t::MULTICLASS)
               .set_update_stats(VW::details::update_stats_multiclass_label<ect>)
               .set_output_example_prediction(VW::details::output_example_prediction_multiclass_label<ect>)
               .set_print_update(VW::details::print_update_multiclass_label<ect>)
               .build();

  all.example_parser->lbl_parser = VW::multiclass_label_parser_global;

  return l;
}
