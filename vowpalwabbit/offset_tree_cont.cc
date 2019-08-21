#include "offset_tree_cont.h"
#include "parse_args.h"  // setup_base()
#include "learner.h"     // init_learner()
#include "action_score.h"
#include <algorithm>

using namespace VW::config;
using namespace LEARNER;

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
  // - static thread_local ensures only one copy per calling thread.  This is to reduce
  //   memory allocations in steady state.
  // TODO: check below
  auto& nodes = binary_tree.nodes;

  // Handle degenerate cases of zero node trees
  if (binary_tree.leaf_node_count() == 0)  // todo: chnage this to throw error at some point
    return 0;

  static thread_local CB::label saved_label;
  saved_label = ec.l.cb;
  ec.l.cb.costs.clear();
  ec.l.simple.label = FLT_MAX; // says it is a test example

  auto cur_node = nodes[0];

  while (!(cur_node.is_leaf))
  {
    /*std::cout << "tree predict: before binary precit:\n " << std::endl;*/
    base.predict(ec, cur_node.id);
    /*std::cout << "tree predict: after binary predict:\n " << std::endl;*/
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
  /*std::cout << "@predict: nodes.size() = " << nodes.size() << std::endl;*/
}

bool compareByid(const node_cost& a, const node_cost& b) { return a.node_id < b.node_id; }

void offset_tree::learn(LEARNER::single_learner& base, example& ec)
{
  // TODO: check below
  const auto saved_label = ec.l.cb;  // TODO save ec.l.cb and restore later shallow copy: check
  const auto saved_weight = ec.weight;

  std::vector<VW::offset_tree_cont::node_cost> node_costs;
  std::vector<VW::offset_tree_cont::node_cost> node_costs_buffer;

  auto& nodes = binary_tree.nodes;
  auto& ac = ec.l.cb.costs;
  /*std::cout << "@learn: nodes.size() = " << nodes.size() << std::endl;*/

  for (uint32_t i = 0; i < ac.size(); i++)
  {
    uint32_t node_id = ac[i].action + binary_tree.internal_node_count();
    std::cout << "\nac[" << i << "].action  = " << ac[i].action << std::endl;
    std::cout << "node_id  = " << node_id << std::endl;
    if (nodes[node_id].depth < binary_tree.depth())
    {
      node_costs_buffer.push_back({node_id, ac[i].cost / ac[i].probability});
      /*std::cout << "nodes[node_id].depth < binary_tree.depth()" << std::endl;*/
    }
    else
    {
      node_costs.push_back({node_id, ac[i].cost / ac[i].probability});
      /*std::cout << "nodes[node_id].depth == binary_tree.depth()" << std::endl;*/
    }
  }
  /*std::cout << "node_cost.size() = " << node_costs.size() << std::endl;
  std::cout << "node_cost_buffer.size() = " << node_costs_buffer.size() << std::endl;*/
  std::sort(node_costs_buffer.begin(), node_costs_buffer.end(), compareByid);
  std::sort(node_costs.begin(), node_costs.end(), compareByid);

  uint32_t iter_count = 0;
  while (!node_costs.empty())
  {
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
      std::cout << "\n\nv.id = " << v.id << std::endl;
      std::cout << "cost_v = " << cost_v << std::endl;
      if (!node_costs.empty() && node_costs.back().node_id == w.id)
      {
        std::cout << "found the sibling" << std::endl;
        cost_w = node_costs.back().cost;
        if (cost_v != cost_w)
        {
          std::cout << "cost_v != cost_w" << std::endl;
          if (((cost_v < cost_w) ? v : w).id == v_parent.left_id) ////
            local_action = -1;
        }
        node_costs.pop_back();  // TODO
      }
      else
      {
        std::cout << "no sibling" << std::endl;
        if (v.id == v_parent.right_id) ////
          local_action = -1;
      }
      float cost_parent = cost_v;
      std::cout << "cost_w = " << cost_w << std::endl;
      if (cost_v != cost_w)  // learn and update the cost of the parent
      {
        ec.l.simple.label = local_action;  // TODO:scalar label type
        ec.weight = abs(cost_v - cost_w);
        /*std::cout << "before binary learn:\n " << std::endl;*/
        std::cout << "binary learning the node " << v.parent_id << std::endl;
        base.learn(ec, v.parent_id);
        /*std::cout << "after binary learn:\n " << std::endl;
        std::cout << "before binary predict:\n " << std::endl;*/
        base.predict(ec, v.parent_id);
        std::cout << "after binary predict:\n " << std::endl;
        std::cout << "ec.pred.scalar = " << (ec.pred.scalar) << std::endl;
        std::cout << "local_action = " << (local_action) << std::endl;
        if (ec.pred.scalar == local_action)
        {
          cost_parent = min(cost_v, cost_w); ////
          std::cout << "ec.pred.scalar == local_action" << std::endl;
        }
        else
        {
          cost_parent = max(cost_v, cost_w); ////
          std::cout << "ec.pred.scalar != local_action" << std::endl;
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
    }
    // node_costs_new = node_costs_buffer;
    iter_count++;
    node_costs = node_costs_new;
    /*node_cost_new.clear();*/
  }

  ec.l.cb = saved_label;
  ec.weight = saved_weight;
}

// inline void copy_to_multiclass(const uint32_t& label, uint32_t& action) {
//   action = label; // TODO: not using it
//}

void predict(offset_tree& ot, single_learner& base, example& ec)
{
  ec.pred.multiclass = ot.predict(base, ec);  // TODO: check: making the prediction zero-based?
}

void learn(offset_tree& tree, single_learner& base, example& ec)
{
  static thread_local uint32_t saved_label;

  // store predictions before learning
  saved_label = tree.predict(base, ec);

  // learn
  tree.learn(base, ec);

  // restore predictions
  // copy_to_action_scores(saved_scores, ec.pred.a_s); //rm
  ec.pred.multiclass = saved_label;  // TODO: instead of above
}

void offset_tree::finish()
{
  binary_tree.nodes.clear();
  //binary_tree.nodes.resize(0);
  binary_tree.nodes.shrink_to_fit();
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

  // Ensure that cb_explore will be the base reduction
  /*if (!options.was_supplied("cb_explore")) //TODO
  {
    options.insert("cb_explore", "2");
  }*/

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
