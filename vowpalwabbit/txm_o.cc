/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved. Released under a BSD (revised)
license as described in the file LICENSE.node
*/
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "reductions.h"
#include "simple_label.h"
#include "multiclass.h"

using namespace std;
using namespace LEARNER;

namespace TXM_O
{
  class txm_o_node_pred	
  {
  public:
    
    double Ehk;	
    float norm_Ehk;
    uint32_t nk;
    uint32_t label;	
    uint32_t label_cnt2;
 
    bool operator==(txm_o_node_pred v){
      return (label == v.label);
    }
    
    bool operator>(txm_o_node_pred v){
      if(label > v.label) return true;	
      return false;
    }
    
    bool operator<(txm_o_node_pred v){
      if(label < v.label) return true;	
      return false;
    }
    
    txm_o_node_pred(uint32_t l)
    {
      label = l;
      Ehk = 0.f;
      norm_Ehk = 0;
      nk = 0;
      label_cnt2 = 0;
    }
  };
  
  typedef struct
  {//everyone has
    uint32_t parent;//the parent node
    v_array<txm_o_node_pred> node_pred;//per-class state
    uint32_t min_count;//the number of examples reaching this node (if it's a leaf) or the minimum reaching any grandchild.

    bool internal;//internal or leaf

    //internal nodes have
    uint32_t base_predictor;//id of the base predictor
    uint32_t left;//left child
    uint32_t right;//right child
    uint32_t myL;//number we tried to send left
    uint32_t myR;//number we tried to send right
    float norm_Eh;//the average margin at the node
    double Eh;//total margin at the node
    uint32_t n;//total events at the node

    //leaf has
    uint32_t max_cnt2;//the number of samples of the most common label
    uint32_t max_cnt2_label;//the most common label
  } txm_o_node;
  
  struct txm_o	
  {
    uint32_t k;	
    vw* all;	
    
    v_array<txm_o_node> nodes;	
    
    uint32_t max_predictors;
    uint32_t predictors_used;

    bool progress;
    uint32_t swap_resist;

    uint32_t nbofswaps;

    FILE *ex_fp;
  };	
  
  inline void internal_to_leaf(txm_o_node& n)
  {
    n.node_pred.erase();
    n.base_predictor = 0;
    n.myL = 0;
    n.myR = 0;
    n.norm_Eh = 0;
    n.Eh = 0;
    n.n = 0;
  }

  inline txm_o_node init_node()	
  {
    txm_o_node node; 
    
    node.parent = 0;
    node.min_count = 0;

    node.internal = false;

    internal_to_leaf(node);
    node.left = 0;
    node.right = 0;

    node.max_cnt2 = 0;
    node.max_cnt2_label = 0;
    return node;
  }
  
  void init_tree(txm_o& d)
  {
    d.nodes.push_back(init_node());
    d.nbofswaps = 0;
  }

  inline uint32_t min_left_right(txm_o& b, txm_o_node& n)
  {
    return min(b.nodes[n.left].min_count, b.nodes[n.right].min_count);
  }
  
  inline uint32_t find_switch_node(txm_o& b)	
  {
    uint32_t node = 0;
    while(b.nodes[node].internal)
      if(b.nodes[b.nodes[node].left].min_count 
	 < b.nodes[b.nodes[node].right].min_count)
	node = b.nodes[node].left; 
      else
	node = b.nodes[node].right;
    return node;
  }
  
  inline void update_min_count(txm_o& b, uint32_t node)	
  {//Constant time min count update.    
    while(node != 0)
      {
	uint32_t prev = node;
	node = b.nodes[node].parent;
	
	if (b.nodes[node].min_count == b.nodes[prev].min_count)
	  break;
	else 
	  b.nodes[node].min_count = min_left_right(b,b.nodes[node]);
      }
  }

  void display_tree_dfs(txm_o& b, txm_o_node node, uint32_t depth)
  {
    for (uint32_t i = 0; i < depth; i++)
      cout << "\t";
    cout << node.min_count << " " << node.myL << " " << node.left << endl;
    if (node.internal)
      display_tree_dfs(b, b.nodes[node.left], depth+1);
    
    for (uint32_t i = 0; i < depth; i++)
      cout << "\t";
    cout << " " << node.myR << " " << node.right << endl;
    if (node.internal)
      display_tree_dfs(b, b.nodes[node.right], depth+1);
  }

  bool children(txm_o& b, uint32_t& current, uint32_t& class_index, uint32_t label)
  {
    class_index = b.nodes[current].node_pred.unique_add_sorted(txm_o_node_pred(label));
    b.nodes[current].node_pred[class_index].label_cnt2++;
    
    if(b.nodes[current].node_pred[class_index].label_cnt2 > b.nodes[current].max_cnt2)
      {
	b.nodes[current].max_cnt2 = b.nodes[current].node_pred[class_index].label_cnt2;
	b.nodes[current].max_cnt2_label = b.nodes[current].node_pred[class_index].label;
      }

    if (b.nodes[current].internal)
      return true;
    else if( b.nodes[current].node_pred.size() > 1 
	     && (b.predictors_used < b.max_predictors 
				     || b.nodes[current].min_count > b.swap_resist*(b.nodes[0].min_count + 1)))
      { //need children and we can make them.
	uint32_t left_child;
	uint32_t right_child;
	if (b.predictors_used < b.max_predictors)
	  {
	    left_child = b.nodes.size();
	    b.nodes.push_back(init_node());
	    right_child = b.nodes.size();
	    b.nodes.push_back(init_node());
	    b.nodes[current].base_predictor = b.predictors_used++;
	  }
	else
	  {
	    uint32_t swap_child = find_switch_node(b);
	    uint32_t swap_parent = b.nodes[swap_child].parent;
	    uint32_t swap_grandparent = b.nodes[swap_parent].parent;
	    if (b.nodes[swap_child].min_count != b.nodes[0].min_count)
	      cout << "glargh " << b.nodes[swap_child].min_count << " != " << b.nodes[0].min_count << endl;
	    b.nbofswaps++;
	    
	    uint32_t nonswap_child;
	    if(swap_child == b.nodes[swap_parent].right)
	      nonswap_child = b.nodes[swap_parent].left;
	    else
	      nonswap_child = b.nodes[swap_parent].right;
	    
	    if(swap_parent == b.nodes[swap_grandparent].left)
	      b.nodes[swap_grandparent].left = nonswap_child;
	    else
	      b.nodes[swap_grandparent].right = nonswap_child;
	    b.nodes[nonswap_child].parent = swap_grandparent;
	    update_min_count(b, nonswap_child);
	    
	    left_child = swap_child;
	    b.nodes[current].base_predictor = b.nodes[swap_parent].base_predictor;
	    internal_to_leaf(b.nodes[swap_parent]);
	    right_child = swap_parent;
	  }
	b.nodes[current].left = left_child;
	b.nodes[left_child].parent = current;
	b.nodes[current].right = right_child;
	b.nodes[right_child].parent = current;
	
	b.nodes[left_child].min_count = b.nodes[current].min_count/2;
	b.nodes[right_child].min_count = b.nodes[left_child].min_count - b.nodes[current].min_count/2;
	
	update_min_count(b, left_child);
	b.nodes[current].internal = true;
      }
    return b.nodes[current].internal;
  }
  
  void train_node(txm_o& b, learner& base, example& ec, uint32_t& current, uint32_t& class_index)
  {
    label_data* simple_temp = (label_data*)ec.ld;
       
    if(b.nodes[current].norm_Eh < b.nodes[current].node_pred[class_index].norm_Ehk)
      {
	simple_temp->label = -1.f;
	b.nodes[current].myL++;
      }
    else
      {
	simple_temp->label = 1.f;
	b.nodes[current].myR++;
      }
    
    base.learn(ec, b.nodes[current].base_predictor);	

    simple_temp->label = FLT_MAX;
    base.predict(ec, b.nodes[current].base_predictor);
    
    b.nodes[current].Eh += (double)ec.partial_prediction;
    b.nodes[current].node_pred[class_index].Ehk += (double)ec.partial_prediction;
    b.nodes[current].n++;
    b.nodes[current].node_pred[class_index].nk++;	
  
    b.nodes[current].norm_Eh = b.nodes[current].Eh / b.nodes[current].n;          
    b.nodes[current].node_pred[class_index].norm_Ehk = b.nodes[current].node_pred[class_index].Ehk / b.nodes[current].node_pred[class_index].nk;
  }
  
  void verify_min_dfs(txm_o& b, txm_o_node node)
  {
    if (node.internal)
      {
	if (node.min_count != min_left_right(b, node))
	  {
	    cout << "badness! " << endl;
	    display_tree_dfs(b, b.nodes[0], 0);
	  }
	verify_min_dfs(b, b.nodes[node.left]);
	verify_min_dfs(b, b.nodes[node.right]);
      }
  }
  
  size_t sum_count_dfs(txm_o& b, txm_o_node node)
  {
    if (node.internal)
      return sum_count_dfs(b, b.nodes[node.left]) + sum_count_dfs(b, b.nodes[node.right]);
    else
      return node.min_count;
  }

  inline uint32_t descend(txm_o_node& n, float prediction)
  {
    if (prediction < 0)
      return n.left;
    else
      return n.right;
  }

  void predict(txm_o& b, learner& base, example& ec)	
  {
    MULTICLASS::multiclass* mc = (MULTICLASS::multiclass*)ec.ld;

    label_data simple_temp;
    simple_temp.initial = 0.0;
    simple_temp.weight = 0.0;	
    simple_temp.label = FLT_MAX;
    ec.ld = &simple_temp;
    uint32_t cn = 0;
    while(b.nodes[cn].internal)
      {
	base.predict(ec, b.nodes[cn].base_predictor);
	cn = descend(b.nodes[cn], simple_temp.prediction);
      }	
    mc->prediction = b.nodes[cn].max_cnt2_label;
    ec.ld = mc;
  }

  void learn(txm_o& b, learner& base, example& ec)
  {
    //verify_min_dfs(b, b.nodes[0]);
    MULTICLASS::multiclass *mc = (MULTICLASS::multiclass*)ec.ld;
    
    if (mc->label == (uint32_t)-1 || !b.all->training || ec.test_only || b.progress)
      predict(b,base,ec);

    if(b.all->training && (mc->label != (uint32_t)-1) && !ec.test_only)	//if training the tree
      {
	uint32_t class_index = 0;	
	label_data simple_temp;
	simple_temp.initial = 0.0;
	simple_temp.weight = mc->weight;
	ec.ld = &simple_temp;	

	uint32_t cn = 0;

	while(children(b, cn, class_index, mc->label))
	  {	    
	    train_node(b, base, ec, cn, class_index);
	    cn = descend(b.nodes[cn], simple_temp.prediction);
	  }
	
	b.nodes[cn].min_count++;
	update_min_count(b, cn);	
	
	ec.ld = mc;
      }
  }
  
  void save_node_stats(txm_o& d)
  {
    FILE *fp;
    uint32_t i, j;
    uint32_t total;
    txm_o* b = &d;
    
    fp = fopen("atxm_debug.csv", "wt");
    
    for(i = 0; i < b->nodes.size(); i++)
      {
	fprintf(fp, "Node: %4d, Internal: %1d, Eh: %7.4f, n: %6d, \n", (int) i, (int) b->nodes[i].internal, b->nodes[i].Eh / b->nodes[i].n, b->nodes[i].n);
	
	fprintf(fp, "Label:, ");
	for(j = 0; j < b->nodes[i].node_pred.size(); j++)
	  {
	    fprintf(fp, "%6d,", (int) b->nodes[i].node_pred[j].label);
	  }	
	fprintf(fp, "\n");
	
	fprintf(fp, "Ehk:, ");
	for(j = 0; j < b->nodes[i].node_pred.size(); j++)
	  {
	    fprintf(fp, "%7.4f,", b->nodes[i].node_pred[j].Ehk / b->nodes[i].node_pred[j].nk);
	  }	
	fprintf(fp, "\n");
	
	total = 0;
	
	fprintf(fp, "nk:, ");
	for(j = 0; j < b->nodes[i].node_pred.size(); j++)
	  {
	    fprintf(fp, "%6d,", (int) b->nodes[i].node_pred[j].nk);
	    total += b->nodes[i].node_pred[j].nk;	
	  }	
	fprintf(fp, "\n");
	
	fprintf(fp, "max(lab:cnt:tot):, %3d,%6d,%7d,\n", (int) b->nodes[i].max_cnt2_label, (int) b->nodes[i].max_cnt2, (int) total);
	fprintf(fp, "left: %4d, right: %4d", (int) b->nodes[i].left, (int) b->nodes[i].right);
	fprintf(fp, "\n\n");
      }
    
    fclose(fp);
  }	
  
  void finish(txm_o& b)
  {
    save_node_stats(b);
    cout << b.nbofswaps << endl;
    for (size_t i = 0; i < b.nodes.size(); i++)
      b.nodes[i].node_pred.delete_v();
    b.nodes.delete_v();
  }
  
  void save_load_tree(txm_o& b, io_buf& model_file, bool read, bool text)
  {
    if (model_file.files.size() > 0)
      {	
	char buff[512];
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t brw = 1;
	uint32_t v;
	int text_len;
	
	if(read)
	  { 
	    brw = bin_read_fixed(model_file, (char*)&i, sizeof(i), "");
	    
	    for(j = 0; j < i; j++)
	      {	
		b.nodes.push_back(init_node());
		
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].parent = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].left = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].right = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].max_cnt2_label = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].internal = v;
	      }
      	    
	    cout << endl << endl;
	    cout << "Number of swaps: " << b.nbofswaps << endl << endl;
	  }
	else
	  {    
	    text_len = sprintf(buff, ":%d\n", (int) b.nodes.size());	//ilosc nodow
	    v = b.nodes.size();
	    brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
	    
	    for(i = 0; i < b.nodes.size(); i++)
	      {	
		text_len = sprintf(buff, ":%d", (int) b.nodes[i].parent);
		v = b.nodes[i].parent;
		brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
		
		text_len = sprintf(buff, ":%d", (int) b.nodes[i].left);
		v = b.nodes[i].left;
		brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
		
		text_len = sprintf(buff, ":%d", (int) b.nodes[i].right);
		v = b.nodes[i].right;
		brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
		
		text_len = sprintf(buff, ":%d", (int) b.nodes[i].max_cnt2_label);
		v = b.nodes[i].max_cnt2_label;
		brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);	
		
		text_len = sprintf(buff, ":%d\n", b.nodes[i].internal);
		v = b.nodes[i].internal;
		brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);	
	      }	
	  }
      }
  }
  
  void finish_example(vw& all, txm_o&, example& ec)
  {
    MULTICLASS::output_example(all, ec);
    VW::finish_example(all, &ec);
  }
  
  learner* setup(vw& all, po::variables_map& vm)	//learner setup
  {
    txm_o* data = (txm_o*)calloc(1, sizeof(txm_o));

    po::options_description txm_o_opts("TXM Online options");
    txm_o_opts.add_options()
      ("no_progress", "disable progressive validation")
      ("swap_resistance", po::value<uint32_t>(&(data->swap_resist))->default_value(64), "higher = more resistance to swap, default=64");
    
    vm = add_options(all, txm_o_opts);
    
    data->k = (uint32_t)vm["txm_o"].as<size_t>();
    
    //append txm_o with nb_actions to options_from_file so it is saved to regressor later
    std::stringstream ss;
    ss << " --txm_o " << data->k;
    all.file_options.append(ss.str());
    
    if (vm.count("no_progress"))
      data->progress = false;
    else
      data->progress = true;

    data->all = &all;
    (all.p->lp) = MULTICLASS::mc_label;
    
    string loss_function = "quantile"; 
    float loss_parameter = 0.5;
    delete(all.loss);
    all.loss = getLossFunction(&all, loss_function, loss_parameter);

    data->max_predictors = data->k - 1;
    
    learner* l = new learner(data, all.l, data->max_predictors);
    l->set_save_load<txm_o,save_load_tree>();
    l->set_learn<txm_o,learn>();
    l->set_predict<txm_o,predict>();
    l->set_finish_example<txm_o,finish_example>();
    l->set_finish<txm_o,finish>();
    
    if(all.training)
      init_tree(*data);	
    
    return l;
  }	
}
