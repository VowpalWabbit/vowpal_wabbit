// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
/*
  Initial implementation by Hal Daume and John Langford.  Reimplementation
  by John Langford.
*/

#include <iostream>
#include <fstream>
#include <ctime>
#include <numeric>

#include "reductions.h"

using namespace LEARNER;
using namespace VW::config;

struct direction
{
  size_t id;          // unique id for node
  size_t tournament;  // unique id for node
  uint32_t winner;    // up traversal, winner
  uint32_t loser;     // up traversal, loser
  uint32_t left;      // down traversal, left
  uint32_t right;     // down traversal, right
  bool last;
};

struct ect
{
  uint64_t k;
  uint64_t errors;
  float class_boundary;

  v_array<direction> directions;  // The nodes of the tournament datastructure

  v_array<v_array<v_array<uint32_t>>> all_levels;

  v_array<uint32_t> final_nodes;  // The final nodes of each tournament.

  v_array<size_t> up_directions;    // On edge e, which node n is in the up direction?
  v_array<size_t> down_directions;  // On edge e, which node n is in the down direction?

  size_t tree_height;  // The height of the final tournament.

  uint32_t last_pair;

  v_array<bool> tournaments_won;

  ~ect()
  {
    for (auto& all_level : all_levels)
    {
      for (auto& t : all_level) t.delete_v();
      all_level.delete_v();
    }
    all_levels.delete_v();
    final_nodes.delete_v();
    up_directions.delete_v();
    directions.delete_v();
    down_directions.delete_v();
    tournaments_won.delete_v();
  }
};

bool exists(v_array<size_t> db)
{
  for (unsigned long i : db)
    if (i != 0)
      return true;
  return false;
}

size_t final_depth(size_t eliminations)
{
  eliminations--;
  for (size_t i = 0; i < 32; i++)
    if (eliminations >> i == 0)
      return i;
  std::cerr << "too many eliminations" << std::endl;
  return 31;
}

bool not_empty(v_array<v_array<uint32_t>> const& tournaments)
{
  auto const first_non_empty_tournament = std::find_if(
      tournaments.cbegin(), tournaments.cend(), [](v_array<uint32_t>& tournament) { return !tournament.empty(); });
  return first_non_empty_tournament != tournaments.cend();
}

void print_level(v_array<v_array<uint32_t>> const& level)
{
  for (auto const& t : level)
  {
    for (auto i : t) std::cout << " " << i;
    std::cout << " | ";
  }
  std::cout << std::endl;
}

size_t create_circuit(ect& e, uint64_t max_label, uint64_t eliminations)
{
  if (max_label == 1)
    return 0;

  v_array<v_array<uint32_t>> tournaments = v_init<v_array<uint32_t>>();
  v_array<uint32_t> t = v_init<uint32_t>();

  for (uint32_t i = 0; i < max_label; i++)
  {
    t.push_back(i);
    direction d = {i, 0, 0, 0, 0, 0, false};
    e.directions.push_back(d);
  }

  tournaments.push_back(t);

  for (size_t i = 0; i < eliminations - 1; i++) tournaments.push_back(v_array<uint32_t>());

  e.all_levels.push_back(tournaments);

  size_t level = 0;

  uint32_t node = (uint32_t)e.directions.size();

  while (not_empty(e.all_levels[level]))
  {
    v_array<v_array<uint32_t>> new_tournaments = v_init<v_array<uint32_t>>();
    tournaments = e.all_levels[level];

    for (size_t t = 0; t < tournaments.size(); t++)
    {
      v_array<uint32_t> empty = v_init<uint32_t>();
      new_tournaments.push_back(empty);
    }

    for (size_t t = 0; t < tournaments.size(); t++)
    {
      for (size_t j = 0; j < tournaments[t].size() / 2; j++)
      {
        uint32_t id = node++;
        uint32_t left = tournaments[t][2 * j];
        uint32_t right = tournaments[t][2 * j + 1];

        direction d = {id, t, 0, 0, left, right, false};
        e.directions.push_back(d);
        uint32_t direction_index = (uint32_t)e.directions.size() - 1;
        if (e.directions[left].tournament == t)
          e.directions[left].winner = direction_index;
        else
          e.directions[left].loser = direction_index;
        if (e.directions[right].tournament == t)
          e.directions[right].winner = direction_index;
        else
          e.directions[right].loser = direction_index;
        if (e.directions[left].last)
          e.directions[left].winner = direction_index;

        if (tournaments[t].size() == 2 && (t == 0 || tournaments[t - 1].empty()))
        {
          e.directions[direction_index].last = true;
          if (t + 1 < tournaments.size())
            new_tournaments[t + 1].push_back(id);
          else  // winner eliminated.
            e.directions[direction_index].winner = 0;
          e.final_nodes.push_back((uint32_t)(e.directions.size() - 1));
        }
        else
          new_tournaments[t].push_back(id);
        if (t + 1 < tournaments.size())
          new_tournaments[t + 1].push_back(id);
        else  // loser eliminated.
          e.directions[direction_index].loser = 0;
      }
      if (tournaments[t].size() % 2 == 1)
        new_tournaments[t].push_back(tournaments[t].last());
    }
    e.all_levels.push_back(new_tournaments);
    level++;
  }

  e.last_pair = (uint32_t)((max_label - 1) * eliminations);

  if (max_label > 1)
    e.tree_height = final_depth(eliminations);

  return e.last_pair + (eliminations - 1);
}

uint32_t ect_predict(ect& e, single_learner& base, example& ec)
{
  if (e.k == (size_t)1)
    return 1;

  uint32_t finals_winner = 0;

  // Binary final elimination tournament first
  ec.l.simple = {FLT_MAX, 0., 0.};

  for (size_t i = e.tree_height - 1; i != (size_t)0 - 1; i--)
  {
    if ((finals_winner | (((size_t)1) << i)) <= e.errors)
    {
      // a real choice exists
      uint32_t problem_number = e.last_pair + (finals_winner | (((uint32_t)1) << i)) - 1;  // This is unique.

      base.learn(ec, problem_number);

      if (ec.pred.scalar > e.class_boundary)
        finals_winner = finals_winner | (((size_t)1) << i);
    }
  }

  uint32_t id = e.final_nodes[finals_winner];
  while (id >= e.k)
  {
    base.learn(ec, id - e.k);

    if (ec.pred.scalar > e.class_boundary)
      id = e.directions[id].right;
    else
      id = e.directions[id].left;
  }
  return id + 1;
}

void ect_train(ect& e, single_learner& base, example& ec)
{
  if (e.k == 1)  // nothing to do
    return;
  MULTICLASS::label_t mc = ec.l.multi;

  label_data simple_temp;

  simple_temp.initial = 0.;

  e.tournaments_won.clear();

  uint32_t id = e.directions[mc.label - 1].winner;
  bool left = e.directions[id].left == mc.label - 1;
  do
  {
    if (left)
      simple_temp.label = -1;
    else
      simple_temp.label = 1;

    ec.l.simple = simple_temp;
    base.learn(ec, id - e.k);
    float old_weight = ec.weight;
    ec.weight = 0.;
    base.learn(ec, id - e.k);  // inefficient, we should extract final prediction exactly.
    ec.weight = old_weight;

    bool won = (ec.pred.scalar - e.class_boundary) * simple_temp.label > 0;

    if (won)
    {
      if (!e.directions[id].last)
        left = e.directions[e.directions[id].winner].left == id;
      else
        e.tournaments_won.push_back(true);
      id = e.directions[id].winner;
    }
    else
    {
      if (!e.directions[id].last)
      {
        left = e.directions[e.directions[id].loser].left == id;
        if (e.directions[id].loser == 0)
          e.tournaments_won.push_back(false);
      }
      else
        e.tournaments_won.push_back(false);
      id = e.directions[id].loser;
    }
  } while (id != 0);

  if (e.tournaments_won.empty())
    std::cout << "badness!" << std::endl;

  // tournaments_won is a bit vector determining which tournaments the label won.
  for (size_t i = 0; i < e.tree_height; i++)
  {
    for (uint32_t j = 0; j < e.tournaments_won.size() / 2; j++)
    {
      bool left = e.tournaments_won[j * 2];
      bool right = e.tournaments_won[j * 2 + 1];
      if (left == right)  // no query to do
        e.tournaments_won[j] = left;
      else  // query to do
      {
        if (left)
          simple_temp.label = -1;
        else
          simple_temp.label = 1;
        simple_temp.weight = (float)(1 << (e.tree_height - i - 1));
        ec.l.simple = simple_temp;

        uint32_t problem_number = e.last_pair + j * (1 << (i + 1)) + (1 << i) - 1;

        base.learn(ec, problem_number);

        if (ec.pred.scalar > e.class_boundary)
          e.tournaments_won[j] = right;
        else
          e.tournaments_won[j] = left;
      }
      if (e.tournaments_won.size() % 2 == 1)
        e.tournaments_won[e.tournaments_won.size() / 2] = e.tournaments_won[e.tournaments_won.size() - 1];
      e.tournaments_won.end() = e.tournaments_won.begin() + (1 + e.tournaments_won.size()) / 2;
    }
  }
}

void predict(ect& e, single_learner& base, example& ec)
{
  MULTICLASS::label_t mc = ec.l.multi;
  if (mc.label == 0 || (mc.label > e.k && mc.label != (uint32_t)-1))
    std::cout << "label " << mc.label << " is not in {1," << e.k << "} This won't work right." << std::endl;
  ec.pred.multiclass = ect_predict(e, base, ec);
  ec.l.multi = mc;
}

void learn(ect& e, single_learner& base, example& ec)
{
  MULTICLASS::label_t mc = ec.l.multi;
  predict(e, base, ec);
  uint32_t pred = ec.pred.multiclass;

  if (mc.label != (uint32_t)-1)
    ect_train(e, base, ec);
  ec.l.multi = mc;
  ec.pred.multiclass = pred;
}

base_learner* ect_setup(options_i& options, vw& all)
{
  auto data = scoped_calloc_or_throw<ect>();
  std::string link;
  option_group_definition new_options("Error Correcting Tournament Options");
  new_options.add(make_option("ect", data->k).keep().help("Error correcting tournament with <k> labels"))
      .add(make_option("error", data->errors).keep().default_value(0).help("errors allowed by ECT"))
      // Used to check value. TODO replace
      .add(make_option("link", link)
               .default_value("identity")
               .keep()
               .help("Specify the link function: identity, logistic, glf1 or poisson"));
  options.add_and_parse(new_options);

  if (!options.was_supplied("ect"))
    return nullptr;

  size_t wpp = create_circuit(*data.get(), data->k, data->errors + 1);

  base_learner* base = setup_base(options, all);
  if (link == "logistic")
    data->class_boundary = 0.5;  // as --link=logistic maps predictions in [0;1]

  learner<ect, example>& l = init_multiclass_learner(data, as_singleline(base), learn, predict, all.p, wpp);

  return make_base(l);
}
