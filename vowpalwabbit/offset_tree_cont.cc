#include "offset_tree_cont.h"
#include "parse_args.h"  // setup_base()
#include "learner.h"     // init_learner()
#include <algorithm>
#include "reductions.h"
#include "debug_log.h"

using namespace VW::config;
using namespace LEARNER;

VW_DEBUG_ENABLE(false);

namespace VW { namespace offset_tree_cont {

tree_node::tree_node(
    uint32_t node_id, uint32_t left_node_id, uint32_t right_node_id, uint32_t p_id, uint32_t depth, bool is_leaf)
    : id(node_id), left_id(left_node_id), right_id(right_node_id), parent_id(p_id), depth(depth), is_leaf(is_leaf)
{
}

bool tree_node::operator==(const tree_node& rhs) const
{
  if (this == &rhs)
    return true;
  return (id == rhs.id && left_id == rhs.left_id && right_id == rhs.right_id && is_leaf == rhs.is_leaf &&
      parent_id == rhs.parent_id);
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

  uint32_t depth = 0;
  try
  {
    // TODO: check below
    nodes.reserve(2 * _num_leaf_nodes - 1);
    uint32_t i = 0;
    uint32_t depth_const = 1;
    nodes.emplace_back(i, 0, 0, i, depth, true);  // root is the parent of itself
    while (i < _num_leaf_nodes - 1)
    {
      nodes[i].left_id = 2 * i + 1;
      nodes[i].right_id = 2 * i + 2;
      nodes[i].is_leaf = false;
      if (2 * i + 1 >= depth_const)
        depth_const = (1 << (++depth + 1)) - 1;
      nodes.emplace_back(2 * i + 1, 0, 0, i, depth, true);
      nodes.emplace_back(2 * i + 2, 0, 0, i, depth, true);
      i++;
    }
  }
  catch (std::bad_alloc& e)
  {
    THROW("Unable to allocate memory for offset tree.  Label count:" << _num_leaf_nodes << " bad_alloc:" << e.what());
  }
  _initialized = true;
  _depth = depth;
}

uint32_t min_depth_binary_tree::internal_node_count() const { return (uint32_t)nodes.size() - _num_leaf_nodes; }

uint32_t min_depth_binary_tree::leaf_node_count() const { return _num_leaf_nodes; }

uint32_t min_depth_binary_tree::depth() const { return _depth; }

void offset_tree::init(uint32_t num_actions) { binary_tree.build_tree(num_actions); }

int32_t offset_tree::learner_count() const { return binary_tree.internal_node_count(); }

uint32_t offset_tree::predict(LEARNER::single_learner& base, example& ec)
{
  auto& nodes = binary_tree.nodes;

  // Handle degenerate cases of zero node trees
  if (binary_tree.leaf_node_count() == 0)  // todo: chnage this to throw error at some point
    return 0;
  const CB::label saved_label = ec.l.cb;
  ec.l.simple.label = FLT_MAX; // says it is a test example
  auto cur_node = nodes[0];

  while (!(cur_node.is_leaf))
  {
    ec.partial_prediction = 0.f;
    ec.pred.scalar = 0.f;
    ec.l.simple.initial = 0.f; // needed for gd.predict()
    base.predict(ec, cur_node.id);
    VW_DBG(ec) << "otree_c: predict() after base.predict() " << scalar_pred_to_string(ec) << ", nodeid = " << cur_node.id << std::endl;
    if (ec.pred.scalar == -1)  // TODO: check
    {
      cur_node = nodes[cur_node.left_id];
    }
    else
    {
      cur_node = nodes[cur_node.right_id];
    }
  }
  ec.l.cb = saved_label;
  return (cur_node.id - binary_tree.internal_node_count() + 1);  // 1 to k
}

bool compareByid(const node_cost& a, const node_cost& b) { return a.node_id < b.node_id; }

void offset_tree::learn(LEARNER::single_learner& base, example& ec)
{
  const auto saved_label = ec.l.cb;
  const auto saved_weight = ec.weight;
  const auto saved_pred = ec.pred.multiclass;

  std::vector<VW::offset_tree_cont::node_cost> node_costs;
  std::vector<VW::offset_tree_cont::node_cost> node_costs_buffer;

  auto& nodes = binary_tree.nodes;
  auto& ac = ec.l.cb.costs;

  VW_DBG(ec) << "otree_c: learn() -- tree_traversal -- " << std::endl;
  for (uint32_t i = 0; i < ac.size(); i++)
  {
    uint32_t node_id = ac[i].action + binary_tree.internal_node_count();
    VW_DBG(ec) << "otree_c: learn() ac[" << i << "].action  = " << ac[i].action << ", node_id  = " << node_id
              << std::endl;
    if (nodes[node_id].depth < binary_tree.depth())
    {
      node_costs_buffer.push_back({node_id, ac[i].cost / ac[i].probability});
    }
    else
    {
      node_costs.push_back({node_id, ac[i].cost / ac[i].probability});
    }
  }
  std::sort(node_costs_buffer.begin(), node_costs_buffer.end(), compareByid);
  std::sort(node_costs.begin(), node_costs.end(), compareByid);

  uint32_t iter_count = 0;
  while (!node_costs.empty() || !node_costs_buffer.empty())
  {
    if (node_costs.empty())
    {
      node_costs = node_costs_buffer;
      node_costs_buffer.clear();
    }
    std::vector<VW::offset_tree_cont::node_cost> node_costs_new;
    while (!node_costs.empty())
    {
      auto n = node_costs.back();
      node_costs.pop_back();
      //auto& n = *node_costs.begin();
      auto& v = nodes[n.node_id];
      auto& cost_v = n.cost;
      if (v.id == v.parent_id)  // if v is the root
        break;
      auto& v_parent = nodes[v.parent_id];
      auto& w = nodes[(v.id == v_parent.left_id) ? v_parent.right_id : v_parent.left_id];  // w is sibling of v
      float cost_w = 0.0f;
      float local_action = 1;
      VW_DBG(ec) << "otree_c: learn() v.id = " << v.id << ", cost_v = " << cost_v << std::endl;
      if (!node_costs.empty() && node_costs.back().node_id == w.id)
      {
        VW_DBG(ec) << "otree_c: learn() found the sibling" << std::endl;
        cost_w = node_costs.back().cost;
        if (cost_v != cost_w)
        {
          VW_DBG(ec) << "otree_c: learn() cost_w = " << cost_w << ", cost_v != cost_w" << std::endl;
          if (((cost_v < cost_w) ? v : w).id == v_parent.left_id) ////
            local_action = -1;
        }
        node_costs.pop_back();  // TODO
      }
      else
      {
        VW_DBG(ec) << "otree_c: learn() no sibling" << std::endl;
        if (v.id == v_parent.right_id) ////
          local_action = -1;
      }
      float cost_parent = cost_v;
      VW_DBG(ec) << "otree_c: learn() cost_w = " << cost_w << std::endl;
      if (cost_v != cost_w)  // learn and update the cost of the parent
      {
        ec.l.simple.label = local_action;  // TODO:scalar label type
        ec.l.simple.initial = 0.f;
        ec.weight = abs(cost_v - cost_w);
        VW_DBG(ec) << "otree_c: learn() #### binary learning the node " << v.parent_id << std::endl;
        base.learn(ec, v.parent_id);
        base.predict(ec, v.parent_id);
        VW_DBG(ec) << "otree_c: learn() after binary predict:" << scalar_pred_to_string(ec)
                  << ", local_action = " << (local_action) << std::endl;
        if (ec.pred.scalar == local_action)
        {
          cost_parent = (std::min)(cost_v, cost_w);
          VW_DBG(ec) << "otree_c: learn() ec.pred.scalar == local_action" << std::endl;
        }
        else
        {
          cost_parent = (std::max)(cost_v, cost_w);
          VW_DBG(ec) << "otree_c: learn() ec.pred.scalar != local_action" << std::endl;
        }
      }
      if (cost_parent > 0.0f)
      {
        node_costs_new.push_back({v.parent_id, cost_parent});
      }
    }
    if (iter_count == 0)
    {
      std::sort(node_costs_new.begin(), node_costs_new.end(), compareByid);
      node_costs_new.insert(node_costs_new.end(), std::make_move_iterator(node_costs_buffer.begin()),
          std::make_move_iterator(node_costs_buffer.end()));
      node_costs_buffer.clear();
    }
    iter_count++;
    node_costs = node_costs_new;
  }

  ec.l.cb = saved_label;
  ec.weight = saved_weight;
  ec.pred.multiclass = saved_pred;
}

void predict(offset_tree& ot, single_learner& base, example& ec)
{
  VW_DBG(ec) << "otree_c: before tree.predict() " << multiclass_pred_to_string(ec) << features_to_string(ec) << std::endl;
  ec.pred.multiclass = ot.predict(base, ec);  // TODO: check: making the prediction zero-based?
  VW_DBG(ec) << "otree_c: after tree.predict() " << multiclass_pred_to_string(ec) << features_to_string(ec) << std::endl;
}

void learn(offset_tree& tree, single_learner& base, example& ec)
{
  VW_DBG(ec) << "otree_c: before tree.learn() " << cb_label_to_string(ec) << features_to_string(ec) << std::endl;
  tree.learn(base, ec);
  VW_DBG(ec) << "otree_c: after tree.learn() " << cb_label_to_string(ec) << features_to_string(ec) << std::endl;
}

void finish(offset_tree& t)
{
  t.~offset_tree();
}

base_learner* offset_tree_cont_setup(VW::config::options_i& options, vw& all)
{
  option_group_definition new_options("Offset tree continuous Options");
  uint32_t num_actions;
  new_options.add(make_option("otc", num_actions).keep().help("Offset tree continuous with <k> labels"));  // TODO: oct
  options.add_and_parse(new_options);

  if (!options.was_supplied("otc"))  // todo: if num_actions = 0 throw error
    return nullptr;

  if (!options.was_supplied("binary"))  // TODO: instead of above
  {
    options.insert("binary", "");
  }

  auto otree = scoped_calloc_or_throw<offset_tree>();
  otree->init(num_actions);

  base_learner* base = setup_base(options, all);

  // all.delete_prediction = ACTION_SCORE::delete_action_scores; //TODO: commented

  learner<offset_tree, example>& l =
      init_learner(otree, as_singleline(base), learn, predict, otree->learner_count(), prediction_type::multiclass);
  // TODO: changed to prediction_type::multiclass

  l.set_finish(finish);
  return make_base(l);
}

}  // namespace offset_tree_cont
}  // namespace VW
