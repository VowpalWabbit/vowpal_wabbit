#include "offset_tree_cont.h"
#include "parse_args.h"  // setup_base()
#include "learner.h"     // init_learner()
#include "action_score.h"

using namespace VW::config;
using namespace LEARNER;

namespace VW { namespace offset_tree_cont {

  tree_node::tree_node(uint32_t node_id, uint32_t left_node_id, uint32_t right_node_id, uint32_t p_id, bool is_leaf)
    : id(node_id), left_id(left_node_id), right_id(right_node_id), parent_id(p_id), is_leaf(is_leaf)
  {
  }

  bool tree_node::operator==(const tree_node& rhs) const
  {
    if (this == &rhs)
      return true;
    return (id == rhs.id && left_id == rhs.left_id && right_id == rhs.right_id && is_leaf == rhs.is_leaf && parent_id == rhs.parent_id);
  }

  bool tree_node::operator!=(const tree_node& rhs) const { return !(*this == rhs); }

  void min_depth_binary_tree::build_tree(uint32_t num_nodes)
  {
    // Sanity checks
    if (_initialized)
    {
      if (num_nodes != _num_leaf_nodes)
      {
        THROW("Tree already initialized.  New leaf node count (" << num_nodes << ") does not equal current value. ("
                                                                 << _num_leaf_nodes << ")");
      }
      return;
    }

    _num_leaf_nodes = num_nodes;
    // deal with degenerate cases of 0 and 1 actions
    if (_num_leaf_nodes == 0)
    {
      _initialized = true;
      return;
    }

    try
    {
      nodes.reserve(2 * _num_leaf_nodes - 1);
      std::vector<uint32_t> tournaments;
      // Add all leaf nodes to node collection
      tournaments.reserve(_num_leaf_nodes);
      for (uint32_t i = 0; i < _num_leaf_nodes; i++)
      {
        nodes.emplace_back(i, 0, 0, 0, true);
        tournaments.emplace_back(i);
      }
      while (tournaments.size() > 1)
      {
        // ids of next set of nodes start with the current
        const auto num_tournaments = tournaments.size();
        // node count (since ids are zero based)
        auto id = nodes.size();
        std::vector<uint32_t> new_tournaments;
        new_tournaments.reserve(num_tournaments / 2 + 1);
        for (size_t j = 0; j < num_tournaments / 2; ++j)
        {
          auto left = tournaments[2 * j];
          auto right = tournaments[2 * j + 1];
          new_tournaments.emplace_back(id);
          nodes[left].parent_id = id;
          nodes[right].parent_id = id;
          nodes.emplace_back(id++, left, right, 0, false);
        }
        if (num_tournaments % 2 == 1)
          new_tournaments.emplace_back(tournaments.back());
        tournaments = std::move(new_tournaments);
      }
      root_idx = tournaments[0];
      nodes[nodes.size()-1].parent_id = root_idx;
    }
    catch (std::bad_alloc& e)
    {
      THROW("Unable to allocate memory for offset tree.  Label count:" << _num_leaf_nodes << " bad_alloc:" << e.what());
    }
    _initialized = true;
  }

  uint32_t min_depth_binary_tree::internal_node_count() const { return nodes.size() - _num_leaf_nodes; }

  uint32_t min_depth_binary_tree::leaf_node_count() const { return _num_leaf_nodes; }

  void offset_tree::init(uint32_t num_actions) { binary_tree.build_tree(num_actions); }

  int32_t offset_tree::learner_count() const { return binary_tree.internal_node_count(); }

  // Helper to deal with collections that don't start with an index of 0
  template<typename T>
  struct offset_helper
  {
    // typedef verbose prediction buffer type
    offset_helper(T& b, uint32_t index_offset)
    : start_index_offset{index_offset}, collection(b) {}

    // intercept index operator to adjust the offset before
    // passing to underlying collection
    typename T::const_reference operator[](size_t idx) const
    {
      return collection[idx - start_index_offset];
    }

    typename T::reference operator[](size_t idx) {
      return collection[idx - start_index_offset];
    }

  private:
    uint32_t start_index_offset = 0;
    T& collection;
  };

  const uint32_t& offset_tree::predict(LEARNER::single_learner& base, example& ec)
  {
    // - pair<float,float> stores the scores for left and right nodes
    // - prediction_buffer stores predictions for all the nodes in the tree for the duration
    //   of the predict() call
    // - static thread_local ensures only one copy per calling thread.  This is to reduce
    //   memory allocations in steady state.

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //TODO: instead of below
    auto& nodes = binary_tree.nodes;

    static thread_local CB::label saved_label;
    saved_label = ec.l.cb;
    ec.l.cb.costs.clear();

    size_t t_size = nodes.size();
    auto cur_node = nodes[t_size - 1];

    while (!(cur_node.is_leaf))
    {
      base.predict(ec, cur_node.id - binary_tree.leaf_node_count());
      if (ec.pred.multiclass == 1)
      {
        cur_node = nodes[cur_node.left_id];
      }
      else
      {
        cur_node = nodes[cur_node.right_id];
      }
    }

    ec.l.cb = saved_label;

    return (cur_node.id + 1); // 1 to k

   

    ///////////////////////////////////////////////////////////////////////////////////////////////

    //using predict_buffer_t = std::vector<std::pair<float, float>>;
    //static thread_local predict_buffer_t prediction_buffer(learner_count());
    //static thread_local std::vector<float> scores(binary_tree.leaf_node_count());

    //auto& t = binary_tree;

    //prediction_buffer.clear();
    //scores.resize(t.leaf_node_count());

    //// Handle degenerate cases of zero and one node trees
    //if (t.leaf_node_count() == 0)
    //  return scores;
    //if (t.leaf_node_count() == 1)
    //{
    //  scores[0] = 1.0f;
    //  return scores;
    //}

    //// Save and clear example label.  
    //// (CB algs predict_or_learn checks for valid label during learn.)
    //static thread_local CB::label saved_label;
    //saved_label = ec.l.cb;
    //ec.l.cb.costs.clear();

    //// Get predictions for all internal nodes
    //for (auto idx = 0; idx < t.internal_node_count(); ++idx)
    //{
    //  base.predict(ec, idx);
    //  prediction_buffer.emplace_back(ec.pred.a_s[0].score, ec.pred.a_s[1].score);
    //}

    //// Restore example label.
    //ec.l.cb = saved_label;

    //// use a offset helper to deal with start index offset
    //offset_helper<predict_buffer_t> buffer_helper(prediction_buffer, t.leaf_node_count());

    //// Compute action scores
    //for (auto rit = t.nodes.rbegin(); rit != t.nodes.rend(); ++rit)
    //{
    //  // done processing all internal nodes
    //  if (rit->is_leaf)
    //    break;

    //  // update probabilities for left node
    //  const float left_p = buffer_helper[rit->id].first;
    //  if (t.nodes[rit->left_id].is_leaf)
    //  {
    //    scores[rit->left_id] = left_p;
    //  }
    //  else
    //  {
    //    const auto left_left_p = buffer_helper[rit->left_id].first;
    //    buffer_helper[rit->left_id].first = left_left_p * left_p;
    //    const auto left_right_p = buffer_helper[rit->left_id].second;
    //    buffer_helper[rit->left_id].second = left_right_p * left_p;
    //  }

    //  // update probabilities for right node
    //  const float right_p = buffer_helper[rit->id].second;
    //  if (t.nodes[rit->right_id].is_leaf)
    //  {
    //    scores[rit->right_id] = right_p;
    //  }
    //  else
    //  {
    //    const auto right_left_p = buffer_helper[rit->right_id].first;
    //    buffer_helper[rit->right_id].first = right_left_p * right_p;
    //    const auto right_right_p = buffer_helper[rit->right_id].second;
    //    buffer_helper[rit->right_id].second = right_right_p * right_p;
    //  }
    //}

    //return scores;
  }

  

void offset_tree::learn(LEARNER::single_learner& base, example& ec) {

  ////////////////////////////////////////////////////////////////////////////////////////// //TODO: instead of below

  const auto global_action = ec.l.cb.costs[0].action; // TODO save ec.l.cb and restore later shallow copy
  const auto global_weight = ec.weight;

  std::map<uint32_t, float> node_cost;
  auto& nodes = binary_tree.nodes;
  auto& ac = ec.l.cb.costs;
  for (uint32_t i = 0; i < ac.size(); i++)
  {
    node_cost.insert(std::pair<uint32_t, float>(ac[i].action, ac[i].cost / ac[i].probability));
  }
  while (!node_cost.empty())
  {
    std::map<uint32_t, float> node_cost_new;
    for (auto& n : node_cost)
    {
      node_cost.erase(n.first);//TODO
      auto& v = nodes[n.first];
      if (v.id == v.parent_id) // if v is the root
        break;
      auto& v_parent = nodes[v.parent_id];
      auto& w = nodes[(v.id == v_parent.left_id) ? v_parent.right_id : v_parent.left_id]; // w is sibling of v
      auto& cost_v = node_cost[v.id];
      if (node_cost.find(w.id) != node_cost.end())
      {
        node_cost.erase(w.id);//TODO
        auto& cost_w = node_cost[w.id];
        if (cost_v != cost_w)
        {
          auto& min_vw = (cost_v < cost_w) ? v : w; //TODO:simplify
          uint32_t local_action = 2;
          if (min_vw.id == v_parent.left_id)
            local_action = 1;
          ec.l.cb.costs[0].action = local_action; // TODO:scalar label type
          ec.weight = abs(cost_v - cost_w);
          base.learn(ec, v.parent_id - binary_tree.leaf_node_count());
          base.predict(ec, v.parent_id - binary_tree.leaf_node_count());
          if (ec.pred.multiclass == local_action)
            node_cost_new.insert(std::pair<uint32_t, float>(v.parent_id, min(cost_v, cost_w)));
          else
            node_cost_new.insert(std::pair<uint32_t, float>(v.parent_id, max(cost_v, cost_w)));
        }
        else
        {
          node_cost_new.insert(std::pair<uint32_t, float>(v.parent_id, cost_v));
        }
      }
      else //(node_cost.find(w.id) == node_cost.end())
      {
        uint32_t local_action = 2;
        if (v.id == v_parent.right_id)
          local_action = 1;
        ec.l.cb.costs[0].action = local_action;
        ec.weight = cost_v;
        base.learn(ec, v.parent_id - binary_tree.leaf_node_count());
        base.predict(ec, v.parent_id - binary_tree.leaf_node_count());
        if (ec.pred.multiclass != local_action)
          node_cost_new.insert(std::pair<uint32_t, float>(v.parent_id, cost_v));
      }
    }
    node_cost = node_cost_new;
  }

  ec.l.cb.costs[0].action = global_action;
  ec.weight = global_weight;
   

   //////////////////////////////////////////////////////////////////////////////////////////


  //const auto global_action = ec.l.cb.costs[0].action;
  //const auto global_weight = ec.weight;

  //// for brevity
  //auto& nodes = binary_tree.nodes; 

  //tree_node node = nodes[global_action - 1];
  //do
  //{  // ascend
  //  const auto previous_id = node.id;
  //  node = nodes[node.parent_id];

  //  // learn
  //  uint32_t local_action = 2;
  //  if (node.left_id == previous_id)
  //    local_action = 1;
  //  ec.l.cb.costs[0].action = local_action;
  //  base.learn(ec, node.id - binary_tree.leaf_node_count());

  //  // re-weight
  //  base.predict(ec, node.id - binary_tree.leaf_node_count());
  //  ec.weight *= ec.pred.a_s[local_action - 1].score;
  //} while (node.parent_id != node.id);

  //ec.l.cb.costs[0].action = global_action;
  //ec.weight = global_weight;
}

inline void copy_to_multiclass(const uint32_t& label, uint32_t& action) {
 /* a_s.clear();
  for (uint32_t idx = 0; idx < scores.size(); ++idx)
  {
    a_s.push_back({idx, scores[idx]});
  }*/
  action = label; // TODO: not using but instead of above
}

void predict(offset_tree& ot, single_learner& base, example& ec)
{
  // get predictions for all internal nodes in binary tree.
  //ec.pred.a_s.clear();
  /*const auto& label = ot.predict(base, ec);
  copy_to_multiclass(label, ec.pred.multiclass);*/
  ec.pred.multiclass = ot.predict(base, ec); //TODO: instead of above
}

  void learn(offset_tree& tree, single_learner& base, example& ec)
  {
    //ec.pred.a_s.clear();
    /*static thread_local offset_tree::scores_t saved_scores;*/
    static thread_local uint32_t saved_label;

    // store predictions before learning
    saved_label = tree.predict(base, ec);

    // learn
    tree.learn(base, ec);

    //restore predictions
    //copy_to_action_scores(saved_scores, ec.pred.a_s); //rm
    ec.pred.multiclass = saved_label; // TODO: instead of above
  }

  base_learner* offset_tree_cont_setup(VW::config::options_i& options, vw& all)
  {
    option_group_definition new_options("Offset tree continuous Options");
    uint32_t num_actions;
    new_options.add(make_option("otc", num_actions).keep().help("Offset tree continuous with <k> labels")); //TODO: oct
    options.add_and_parse(new_options);

    if (!options.was_supplied("otc"))
      return nullptr;

    // Ensure that cb_explore will be the base reduction
    /*if (!options.was_supplied("cb_explore")) //TODO
    {
      options.insert("cb_explore", "2");
    }*/

    if (!options.was_supplied("cb")) //TODO: instead of above
    {
      options.insert("cb", "2");
    }

    auto otree = scoped_calloc_or_throw<offset_tree>();
    otree->init(num_actions);

    base_learner* base = setup_base(options, all);

    // all.delete_prediction = ACTION_SCORE::delete_action_scores; //TODO: commented

    learner<offset_tree, example>& l = init_learner(
        otree, as_singleline(base), learn, predict, otree->learner_count(), prediction_type::multiclass);
    // TODO: changed to prediction_type::multiclass
    return make_base(l);
  }

}}
