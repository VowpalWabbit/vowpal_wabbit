#include "offset_tree_cont.h"
#include "parse_args.h"  // setup_base()
#include "learner.h"     // init_learner()
#include <algorithm>
#include "reductions.h"
#include "debug_log.h"
#include <cassert>

using namespace VW::config;
using namespace LEARNER;

using std::vector;
using CB::cb_class;

VW_DEBUG_ENABLE(false)

namespace VW
{
namespace offset_tree_cont
{
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

  try
  {
    // Number of nodes in a minimal binary tree := (2 * LeafCount) - 1
    nodes.reserve(2 * _num_leaf_nodes - 1);

    //  Insert Root Node: First node in the collection, Parent is itself
    //  {node_id, left_id, right_id, parent_id, depth, is_leaf}
    nodes.emplace_back(0, 0, 0, 0, 0, true);

    uint32_t depth = 0, depth_const = 1;
    for (uint32_t i = 0; i < _num_leaf_nodes - 1; ++i)
    {
      nodes[i].left_id = 2 * i + 1;
      nodes[i].right_id = 2 * i + 2;
      nodes[i].is_leaf = false;
      if (2 * i + 1 >= depth_const)
        depth_const = (1 << (++depth + 1)) - 1;
      nodes.emplace_back(2 * i + 1, 0, 0, i, depth, true);
      nodes.emplace_back(2 * i + 2, 0, 0, i, depth, true);
    }

    _initialized = true;
    _depth = depth;
  }
  catch (std::bad_alloc& e)
  {
    THROW("Unable to allocate memory for offset tree.  Label count:" << _num_leaf_nodes << " bad_alloc:" << e.what());
  }
}

uint32_t min_depth_binary_tree::internal_node_count() const { return (uint32_t)nodes.size() - _num_leaf_nodes; }

uint32_t min_depth_binary_tree::leaf_node_count() const { return _num_leaf_nodes; }

uint32_t min_depth_binary_tree::depth() const { return _depth; }

const tree_node& min_depth_binary_tree::get_sibling(const tree_node& v)
{
  // We expect not to get called on root
  const tree_node& v_parent = nodes[v.parent_id];
  return nodes[(v.id == v_parent.left_id) ? v_parent.right_id : v_parent.left_id];
}

void offset_tree::init(uint32_t num_actions) { _binary_tree.build_tree(num_actions); }

int32_t offset_tree::learner_count() const { return _binary_tree.internal_node_count(); }

uint32_t offset_tree::predict(LEARNER::single_learner& base, example& ec)
{
  const vector<tree_node>& nodes = _binary_tree.nodes;

  // Handle degenerate cases of zero node trees
  if (_binary_tree.leaf_node_count() == 0)  // todo: chnage this to throw error at some point
    return 0;
  const CB::label saved_label = ec.l.cb;
  ec.l.simple.label = FLT_MAX;  // says it is a test example
  auto cur_node = nodes[0];

  while (!(cur_node.is_leaf))
  {
    ec.partial_prediction = 0.f;
    ec.pred.scalar = 0.f;
    ec.l.simple.initial = 0.f;  // needed for gd.predict()
    base.predict(ec, cur_node.id);
    VW_DBG(_dd) << "otree_c: predict() after base.predict() " << scalar_pred_to_string(ec)
                << ", nodeid = " << cur_node.id << std::endl;
    if (ec.pred.scalar < 0)  // TODO: check
    {
      cur_node = nodes[cur_node.left_id];
    }
    else
    {
      cur_node = nodes[cur_node.right_id];
    }
  }
  ec.l.cb = saved_label;
  return (cur_node.id - _binary_tree.internal_node_count() + 1);  // 1 to k
}

bool compareByid(const node_cost& a, const node_cost& b) { return a.node_id < b.node_id; }
bool compareByid_1(const node_cost& a, const node_cost& b) { return a.node_id > b.node_id; }

void offset_tree::init_node_sets(v_array<cb_class>& ac)
{
  for (uint32_t i = 0; i < ac.size(); i++)
  {
    // Sanity check. actions are 1 based index
    assert(ac[i].action > 0);
    // TODO: Handle negative cost
    assert(ac[i].cost >= 0.f);

    uint32_t node_id = ac[i].action + _binary_tree.internal_node_count() - 1;
    VW_DBG(_dd) << "otree_c: learn() ac[" << i << "].action  = " << ac[i].action << ", node_id  = " << node_id
                << std::endl;
    if (_binary_tree.nodes[node_id].depth < _binary_tree.depth())
      _nodes_depth_1.push_back({node_id, ac[i].cost / ac[i].probability});
    else
      _nodes_depth.push_back({node_id, ac[i].cost / ac[i].probability});
  }

  if (_nodes_depth.empty())
  {
    _nodes_depth = _nodes_depth_1;
    _nodes_depth_1.clear();
  }

  std::sort(_nodes_depth_1.begin(), _nodes_depth_1.end(), compareByid_1);
  std::sort(_nodes_depth.begin(), _nodes_depth.end(), compareByid);
}

constexpr float RIGHT = 1.0f;
constexpr float LEFT = -1.0f;

void offset_tree::reduce_depth() {
  // Completed processing all nodes at current depth
  // Moving on to depth current-1
  // std::sort(_nodes_depth_1.begin(), _nodes_depth_1.end(), compareByid); // do not need to sort
  _nodes_depth = _nodes_depth_1;
  _nodes_depth_1.clear();
}

void offset_tree::learn(LEARNER::single_learner& base, example& ec)
{
  const polylabel saved_label = ec.l;
  const float saved_weight = ec.weight;
  const polyprediction saved_pred = ec.pred;

  const vector<tree_node>& nodes = _binary_tree.nodes;
  v_array<cb_class>& ac = ec.l.cb.costs;

  _dd = ec.stack_depth;

  VW_DBG(_dd) << "otree_c: learn() -- tree_traversal -- " << std::endl;

  init_node_sets(ac);

  while (!_nodes_depth.empty())
  {
    const node_cost& n_c = _nodes_depth.back();
    _nodes_depth.pop_back();

    const tree_node& v = nodes[n_c.node_id];
    if (v.is_root())
      break;

    const float cost_v = n_c.cost;
    const tree_node& v_parent = nodes[v.parent_id];
    const tree_node& w = _binary_tree.get_sibling(v);  // w is sibling of v
    float cost_w = 0.0f;
    float local_action = RIGHT;
    VW_DBG(_dd) << "otree_c: learn() v.id = " << v.id << ", cost_v = " << cost_v << std::endl;

    if (!_nodes_depth.empty() && _nodes_depth.back().node_id == w.id)
    {
      VW_DBG(_dd) << "otree_c: learn() found the sibling" << std::endl;
      cost_w = _nodes_depth.back().cost;
      if (cost_v != cost_w)
      {
        VW_DBG(_dd) << "otree_c: learn() cost_w = " << cost_w << ", cost_v != cost_w" << std::endl;
        if (((cost_v < cost_w) ? v : w).id == v_parent.left_id)  ////
          local_action = LEFT;
      }
      _nodes_depth.pop_back(); // Remove sibling
    }
    else
    {
      VW_DBG(_dd) << "otree_c: learn() no sibling" << std::endl;
      // If v is right node, right node has non-zero cost so go left
      // TODO: Handle negative cost
      if (v.id == v_parent.right_id)
        local_action = LEFT;
    }

    float cost_parent = cost_v;
    VW_DBG(_dd) << "otree_c: learn() cost_w = " << cost_w << std::endl;
    if (cost_v != cost_w)  // learn and update the cost of the parent
    {
      ec.l.simple.label = local_action;  // TODO:scalar label type
      ec.l.simple.initial = 0.f;
      ec.weight = abs(cost_v - cost_w);
      VW_DBG(_dd) << "otree_c: learn() #### binary learning the node " << v.parent_id << std::endl;
      base.learn(ec, v.parent_id);
      base.predict(ec, v.parent_id);
      VW_DBG(_dd) << "otree_c: learn() after binary predict:" << scalar_pred_to_string(ec)
                  << ", local_action = " << (local_action) << std::endl;
      float trained_action = (ec.pred.scalar < 0) ? LEFT: RIGHT;
      if (trained_action == local_action)
      {
        cost_parent = (std::min)(cost_v, cost_w)*fabs(ec.pred.scalar) + (std::max)(cost_v, cost_w)*(1 - fabs(ec.pred.scalar));
        VW_DBG(_dd) << "otree_c: learn() ec.pred.scalar == local_action" << std::endl;
      }
      else
      {
        cost_parent = (std::max)(cost_v, cost_w)*fabs(ec.pred.scalar) + (std::min)(cost_v, cost_w)*(1 - fabs(ec.pred.scalar));
        VW_DBG(_dd) << "otree_c: learn() ec.pred.scalar != local_action" << std::endl;
      }
    }
    if (cost_parent > 0.0f)
      _nodes_depth_1.push_back({v.parent_id, cost_parent});

    if (_nodes_depth.empty())
      reduce_depth();
  } // End while (!nodes_depth.empty())

  ec.l = saved_label;
  ec.weight = saved_weight;
  ec.pred = saved_pred;
}

void predict(offset_tree& ot, single_learner& base, example& ec)
{
  VW_DBG(ec) << "otree_c: before tree.predict() " << multiclass_pred_to_string(ec) << features_to_string(ec)
             << std::endl;
  ec.pred.multiclass = ot.predict(base, ec);  // TODO: check: making the prediction zero-based?
  VW_DBG(ec) << "otree_c: after tree.predict() " << multiclass_pred_to_string(ec) << features_to_string(ec)
             << std::endl;
}

void learn(offset_tree& tree, single_learner& base, example& ec)
{
  VW_DBG(ec) << "otree_c: before tree.learn() " << cb_label_to_string(ec) << features_to_string(ec) << std::endl;
  tree.learn(base, ec);
  VW_DBG(ec) << "otree_c: after tree.learn() " << cb_label_to_string(ec) << features_to_string(ec) << std::endl;
}

void finish(offset_tree& t) { t.~offset_tree(); }

base_learner* offset_tree_cont_setup(VW::config::options_i& options, vw& all)
{
  option_group_definition new_options("Offset tree continuous Options");
  uint32_t num_actions;
  uint32_t scorer_flag;
  new_options.add(make_option("otc", num_actions).keep().help("Offset tree continuous with <k> labels"))
    .add(make_option("scorer_option", scorer_flag).default_value(0).keep()
      .help("Offset tree continuous reduction to scorer [-1, 1] versus binary -1/+1"));  // TODO: oct

  options.add_and_parse(new_options);

  if (!options.was_supplied("otc"))  // todo: if num_actions = 0 throw error
    return nullptr;

  if (scorer_flag)
  {
    options.insert("link", "glf1");
  }
  else //if (!options.was_supplied("binary"))
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
