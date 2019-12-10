// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <float.h>
#include <errno.h>

#include "reductions.h"
#include "vw.h"
#include "search.h"

using namespace VW::config;

namespace DebugMT
{
void run(Search::search& sch, multi_ex& ec);
Search::search_metatask metatask = {"debug", run, nullptr, nullptr, nullptr, nullptr};

void run(Search::search& sch, multi_ex& ec)
{
  sch.base_task(ec)
      .foreach_action(
          [](Search::search& /*sch*/, size_t t, float min_cost, action a, bool taken, float a_cost) -> void {
            std::cerr << "==DebugMT== foreach_action(t=" << t << ", min_cost=" << min_cost << ", a=" << a
                      << ", taken=" << taken << ", a_cost=" << a_cost << ")" << std::endl;
          })

      .post_prediction([](Search::search& /*sch*/, size_t t, action a, float a_cost) -> void {
        std::cerr << "==DebugMT== post_prediction(t=" << t << ", a=" << a << ", a_cost=" << a_cost << ")" << std::endl;
      })

      .maybe_override_prediction([](Search::search& /*sch*/, size_t t, action& a, float& a_cost) -> bool {
        std::cerr << "==DebugMT== maybe_override_prediction(t=" << t << ", a=" << a << ", a_cost=" << a_cost << ")"
                  << std::endl;
        return false;
      })

      .final_run()

      .Run();
}
}  // namespace DebugMT

namespace SelectiveBranchingMT
{
void run(Search::search& sch, multi_ex& ec);
void initialize(Search::search& sch, size_t& num_actions, options_i& options);
void finish(Search::search& sch);
Search::search_metatask metatask = {"selective_branching", run, initialize, finish, nullptr, nullptr};

typedef std::pair<action, float> act_score;
typedef v_array<act_score> path;
typedef std::pair<float, path> branch;

std::ostream& operator<<(std::ostream& os, const std::pair<unsigned int, float>& v)
{
  os << v.first << '_' << v.second;
  return os;
}

struct task_data
{
  size_t max_branches, kbest;
  v_array<branch> branches;
  v_array<std::pair<branch, std::string*> > final;
  path trajectory;
  float total_cost;
  size_t cur_branch;
  std::string* output_string;
  std::stringstream* kbest_out;
  task_data(size_t mb, size_t kb) : max_branches(mb), kbest(kb)
  {
    branches = v_init<branch>();
    final = v_init<std::pair<branch, std::string*> >();
    trajectory = v_init<act_score>();
    output_string = nullptr;
    kbest_out = nullptr;
  }
  ~task_data()
  {
    branches.delete_v();
    final.delete_v();
    trajectory.delete_v();
    delete output_string;
    delete kbest_out;
  }
};

void initialize(Search::search& sch, size_t& /*num_actions*/, options_i& options)
{
  size_t max_branches = 2;
  size_t kbest = 0;
  option_group_definition new_options("selective branching options");
  new_options
      .add(make_option("search_max_branch", max_branches)
               .default_value(2)
               .help("maximum number of branches to consider"))
      .add(make_option("search_kbest", kbest)
               .default_value(0)
               .help("number of best items to output (0=just like non-selectional-branching, default)"));
  options.add_and_parse(new_options);

  task_data* d = new task_data(max_branches, kbest);
  sch.set_metatask_data(d);
}

void finish(Search::search& sch) { delete sch.get_metatask_data<task_data>(); }

void run(Search::search& sch, multi_ex& ec)
{
  task_data& d = *sch.get_metatask_data<task_data>();

  // generate an initial trajectory, but record possible branches
  d.branches.clear();
  d.final.clear();
  d.trajectory.clear();
  d.total_cost = 0.;
  d.output_string = nullptr;

  cdbg << "*** INITIAL PASS ***" << std::endl;
  sch.base_task(ec)
      .foreach_action([](Search::search& sch, size_t t, float min_cost, action a, bool taken, float a_cost) -> void {
        cdbg << "==DebugMT== foreach_action(t=" << t << ", min_cost=" << min_cost << ", a=" << a << ", taken=" << taken
             << ", a_cost=" << a_cost << ")" << std::endl;
        if (taken)
          return;  // ignore the taken action
        task_data& d = *sch.get_metatask_data<task_data>();
        float delta = a_cost - min_cost;
        path branch = v_init<act_score>();
        push_many<act_score>(branch, d.trajectory.begin(), d.trajectory.size());
        branch.push_back(std::make_pair(a, a_cost));
        d.branches.push_back(std::make_pair(delta, branch));
        cdbg << "adding branch: " << delta << " -> " << branch << std::endl;
      })
      .post_prediction([](Search::search& sch, size_t /*t*/, action a, float a_cost) -> void {
        task_data& d = *sch.get_metatask_data<task_data>();
        d.trajectory.push_back(std::make_pair(a, a_cost));
        d.total_cost += a_cost;
      })
      .with_output_string([](Search::search& sch, std::stringstream& output) -> void {
        sch.get_metatask_data<task_data>()->output_string = new std::string(output.str());
      })
      .Run();

  // the last item the trajectory stack is complete and therefore not a branch
  // if (! d.branches.empty())
  //  d.branches.pop().second.delete_v();

  {
    // construct the final trajectory
    path original_final = v_init<act_score>();
    copy_array(original_final, d.trajectory);
    d.final.push_back(std::make_pair(std::make_pair(d.total_cost, original_final), d.output_string));
  }

  // sort the branches by cost
  stable_sort(
      d.branches.begin(), d.branches.end(), [](const branch& a, const branch& b) -> bool { return a.first < b.first; });

  // make new predictions
  for (size_t i = 0; i < std::min(d.max_branches, d.branches.size()); i++)
  {
    d.cur_branch = i;
    d.trajectory.clear();
    d.total_cost = 0.;
    d.output_string = nullptr;

    cdbg << "*** BRANCH " << i << " *** " << d.branches[i].first << " : " << d.branches[i].second << std::endl;
    sch.base_task(ec)
        .foreach_action([](Search::search& /*sch*/, size_t /*t*/, float /*min_cost*/, action /*a*/, bool /*taken*/,
                            float /*a_cost*/) -> void {})
        .maybe_override_prediction([](Search::search& sch, size_t t, action& a, float& a_cost) -> bool {
          task_data& d = *sch.get_metatask_data<task_data>();
          path& path = d.branches[d.cur_branch].second;
          if (t >= path.size())
            return false;
          a = path[t].first;
          a_cost = path[t].second;
          return true;
        })
        .post_prediction([](Search::search& sch, size_t /*t*/, action a, float a_cost) -> void {
          task_data& d = *sch.get_metatask_data<task_data>();
          d.trajectory.push_back(std::make_pair(a, a_cost));
          d.total_cost += a_cost;
        })
        .with_output_string([](Search::search& sch, std::stringstream& output) -> void {
          sch.get_metatask_data<task_data>()->output_string = new std::string(output.str());
        })
        .Run();

    {
      // construct the final trajectory
      path this_final = v_init<act_score>();
      copy_array(this_final, d.trajectory);
      d.final.push_back(std::make_pair(std::make_pair(d.total_cost, this_final), d.output_string));
    }
  }

  // sort the finals by cost
  stable_sort(d.final.begin(), d.final.end(),
      [](const std::pair<branch, std::string*>& a, const std::pair<branch, std::string*>& b) -> bool {
        return a.first.first < b.first.first;
      });

  d.kbest_out = nullptr;
  if (d.output_string && (d.kbest > 0))
  {
    d.kbest_out = new std::stringstream();
    for (size_t i = 0; i < std::min(d.final.size(), d.kbest); i++)
      (*d.kbest_out) << *d.final[i].second << "\t" << d.final[i].first.first << std::endl;
  }

  // run the final selected trajectory
  cdbg << "*** FINAL ***" << std::endl;
  d.cur_branch = 0;
  d.output_string = nullptr;
  sch.base_task(ec)
      .foreach_action([](Search::search& /*sch*/, size_t /*t*/, float /*min_cost*/, action /*a*/, bool /*taken*/,
                          float /*a_cost*/) -> void {})
      .maybe_override_prediction([](Search::search& sch, size_t t, action& a, float& a_cost) -> bool {
        task_data& d = *sch.get_metatask_data<task_data>();
        path& path = d.final[d.cur_branch].first.second;
        if ((t >= path.size()) || (path[t].first == (action)-1))
          return false;
        a = path[t].first;
        a_cost = path[t].second;
        return true;
      })
      .with_output_string([](Search::search& sch, std::stringstream& output) -> void {
        task_data& d = *sch.get_metatask_data<task_data>();
        if (d.kbest_out)
        {
          output.str("");
          output << d.kbest_out->str();
        }
      })
      .final_run()
      .Run();

  // clean up memory
  for (size_t i = 0; i < d.branches.size(); i++) d.branches[i].second.delete_v();
  d.branches.clear();
  for (size_t i = 0; i < d.final.size(); i++)
  {
    d.final[i].first.second.delete_v();
    delete d.final[i].second;
  }
  d.final.clear();
  delete d.kbest_out;
  d.kbest_out = nullptr;
}
}  // namespace SelectiveBranchingMT
