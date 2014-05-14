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
  uint32_t ceil_log2(uint32_t k)
  {
    uint32_t i = 0;
    
    while (k > (uint32_t)(1 << i))
      i++;
    
    return i;
  }
  
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
  {
    size_t parent;
    size_t left;
    size_t right;
    size_t max_cnt2;
    size_t max_cnt2_label;

    uint32_t L;
    uint32_t R;
    
    uint32_t myL;
    uint32_t myR;
    bool leaf;
    v_array<txm_o_node_pred> node_pred;
    
    double Eh;	
    float norm_Eh;
    uint32_t n;	
  } txm_o_node;
  
  struct txm_o	
  {
    uint32_t k;	
    vw* all;	
    
    v_array<txm_o_node> nodes;	
    
    size_t max_nodes;
    uint32_t min_count;
    bool progress;
    uint32_t swap_resist;

    uint32_t nbofswaps;

    size_t ex_num;
    FILE *ex_fp;
  };	
  
  txm_o_node init_node()	
  {
    txm_o_node node; 
    
    node.parent = 0;
    node.left = 0;
    node.right = 0;
    node.Eh = 0;
    node.norm_Eh = 0;
    node.n = 0;
    node.leaf = true;
    node.max_cnt2 = 0;
    node.max_cnt2_label = 0;
    node.L = 0;
    node.R = 0;
    node.myL = 0;
    node.myR = 0;
    return node;
  }
  
  void init_tree(txm_o& d)
  {
    d.nodes.push_back(init_node());
    d.ex_num = 0;
    d.nbofswaps = 0;
    //d.ex_fp = fopen("ex_nums.txt", "wt");
  }

 float print_intercept(vw& all, example& ec, learner& base, size_t& cn)
  {
    float w_1 = 0.;
    float w_0 = 0.;
					       
    bool got_first = true;

    ec.ft_offset += (uint32_t)(base.increment*cn);
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) 
      if (got_first)
	{
	  w_1 = all.reg.weight_vector[((ec.atomics[*i].begin)->weight_index + ec.ft_offset) & all.reg.weight_mask];
	  got_first = false;
	}
      else
	w_0 = all.reg.weight_vector[((ec.atomics[*i].begin)->weight_index + ec.ft_offset) & all.reg.weight_mask];
    ec.ft_offset -= (uint32_t)(base.increment*cn);

    float w_ratio = -w_0/w_1;

    return w_ratio;
  }
  
  inline uint32_t min_left_right(txm_o_node n)
  {
    return min(n.L, n.R);
  }

  size_t find_switch_child(txm_o& b)	
  {
    size_t child = 0;
    while(! b.nodes[child].leaf)
      if(min_left_right(b.nodes[b.nodes[child].left]) 
	 < min_left_right(b.nodes[b.nodes[child].right]))
	child = b.nodes[child].left; 
      else
	child = b.nodes[child].right;
    return child;
  }
  
  void update_min_count(txm_o& b, size_t node)	
  {//Constant time min count update.    
    while(node != 0)
      {
	size_t prev = node;
	node = b.nodes[node].parent;
	
	uint32_t prev_min = min_left_right(b.nodes[prev]);
	
	if (b.nodes[node].left == prev)
	  if (b.nodes[node].L == prev_min)
	    break;
	  else
	    b.nodes[node].L = prev_min;
	else
	  if (b.nodes[node].R == prev_min)
	    break;
	  else 
	    b.nodes[node].R = prev_min;
      }
    if (node == 0)
      b.min_count = min_left_right(b.nodes[node]);
  }

  void display_tree2(txm_o& d)
  {
    size_t i;
    float ratio;
    
    for(i = 0; i < d.nodes.size(); i++)
      {
	if(d.nodes[i].leaf)
	  {
	    ratio = (float)d.nodes[i].max_cnt2;
	    //ratio /= (float)d.nodes[i].ec_count;
	    ratio /= (float)(d.nodes[i].L + d.nodes[i].R);
	    ratio = 1 - ratio;
	    cout << "[" << i << "," << d.nodes[i].max_cnt2_label << "," << d.nodes[i].max_cnt2 << "," << ratio << "] ";
	  }
	else
	  cout << "(" << i << "," << d.nodes[i].max_cnt2_label << "," << d.nodes[i].max_cnt2 << ") ";
      }
    cout << endl;
  }

  void display_tree_dfs(txm_o& b, txm_o_node node, size_t depth)
  {
    for (size_t i = 0; i < depth; i++)
      cout << "\t";
    cout << "L = " << node.L << " " << node.myL << " " << node.left << endl;
    if (!node.leaf)
      display_tree_dfs(b, b.nodes[node.left], depth+1);
    
    for (size_t i = 0; i < depth; i++)
      cout << "\t";
    cout << "R = " << node.R << " " << node.myR << " " << node.right << endl;
    if (!node.leaf)
      display_tree_dfs(b, b.nodes[node.right], depth+1);
  }
  
  size_t verify_LR_dfs(txm_o& b, txm_o_node node)
  {
    if (node.leaf)
      return min_left_right(node);
    else
      {
	if (node.L != verify_LR_dfs(b, b.nodes[node.left]) 
	    || node.R != verify_LR_dfs(b, b.nodes[node.right]))
	  {
	    cout << "badness! " << node.L << " != " << verify_LR_dfs(b, b.nodes[node.left]) 
		 << " or " <<  node.R << " != " << verify_LR_dfs(b, b.nodes[node.right]) << endl;
	    display_tree_dfs(b, b.nodes[0], 0);
	  }
	return min_left_right(node);
      }
  }
  
  size_t sum_LR_dfs(txm_o& b, txm_o_node node)
  {
    if (node.leaf)
      return node.R + node.L;
    else
      return sum_LR_dfs(b, b.nodes[node.left]) + sum_LR_dfs(b, b.nodes[node.right]);
  }
  
  void update_new_children(txm_o& b, size_t& current)
  {
    b.nodes[current].L = min_left_right(b.nodes[b.nodes[current].left]);
    b.nodes[current].R = min_left_right(b.nodes[b.nodes[current].right]);
    update_min_count(b, current);
  }
      
  void train_node(txm_o& b, learner& base, example& ec, size_t& current, size_t& class_index)
  {
    label_data* simple_temp = (label_data*)ec.ld;
       
    float left_or_right = b.nodes[current].node_pred[class_index].norm_Ehk - b.nodes[current].norm_Eh;
    
    if(left_or_right < 0)
      {
	simple_temp->label = -1.f;
	b.nodes[current].myL++;
      }
    else
      {
	simple_temp->label = 1.f;
	b.nodes[current].myR++;
      }
    
    if( b.nodes[current].myR > 0 && b.nodes[current].myL > 0 && b.nodes[current].leaf)
      {//need children but have no children
	if(b.nodes.size() + 2 <= b.max_nodes || min_left_right(b.nodes[current]) > b.swap_resist*(b.min_count + 1))
	  {
	    size_t left_child;
	    size_t right_child;
	    if (b.nodes.size() + 2 <= b.max_nodes)
	      {
		left_child = b.nodes.size();
		b.nodes.push_back(init_node());	
		b.nodes.push_back(init_node());
		right_child = left_child+1;
	      }
	    else
	      {
		size_t swap_child = find_switch_child(b);
		size_t swap_parent = b.nodes[swap_child].parent;
		size_t swap_grandparent = b.nodes[swap_parent].parent;
		if (min_left_right(b.nodes[swap_child]) != b.min_count)
		  cout << "glargh " << min_left_right(b.nodes[swap_child]) << " != " << b.min_count << endl;
		b.nbofswaps++;
		
		size_t nonswap_child;
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
		
		b.nodes[swap_parent].leaf = true;
		b.nodes[swap_child].leaf = true;
		
		left_child = swap_child;
		right_child = swap_parent;
	      }
	    b.nodes[current].left = left_child;
	    b.nodes[left_child].parent = current;
	    b.nodes[current].right = right_child;
	    b.nodes[right_child].parent = current;
	    	    
	    b.nodes[left_child].L = b.nodes[current].L/2;
	    b.nodes[left_child].R = b.nodes[current].L - b.nodes[current].L/2;
	    b.nodes[right_child].L = b.nodes[current].R/2;
	    b.nodes[right_child].R = b.nodes[current].R - b.nodes[current].R/2;
	    
	    update_new_children(b, current);
	    b.nodes[current].leaf = false;
	  }
      }
    base.learn(ec, current);	
    
    simple_temp->label = FLT_MAX;
    base.predict(ec, current);

    b.nodes[current].Eh += (double)ec.partial_prediction;
    b.nodes[current].node_pred[class_index].Ehk += (double)ec.partial_prediction;
    b.nodes[current].n++;
    b.nodes[current].node_pred[class_index].nk++;	
  
    b.nodes[current].norm_Eh = b.nodes[current].Eh / b.nodes[current].n;          
    b.nodes[current].node_pred[class_index].norm_Ehk = b.nodes[current].node_pred[class_index].Ehk / b.nodes[current].node_pred[class_index].nk;
  }
  
  void predict(txm_o& b, learner& base, example& ec)	
  {
    MULTICLASS::multiclass* mc = (MULTICLASS::multiclass*)ec.ld;

    label_data simple_temp;
    simple_temp.initial = 0.0;
    simple_temp.weight = mc->weight;	
    ec.ld = &simple_temp;
    size_t cn = 0;
    while(1)
      {
	if(b.nodes[cn].leaf)	
	  {	
	    mc->prediction = b.nodes[cn].max_cnt2_label;
	    ec.ld = mc;
	    break;	
	  }
	simple_temp.label = FLT_MAX;
	base.predict(ec, cn);
  
	if(simple_temp.prediction < 0)//b.nodes[cn].Eh/b.nodes[cn].n)	
	  cn = b.nodes[cn].left;
	else
	  cn = b.nodes[cn].right;	
      }	
  }

  void save_node_stats(txm_o& d)
  {
    FILE *fp;
    uint32_t i, j;
    size_t total;
    txm_o* b = &d;
    
    fp = fopen("atxm_debug.csv", "wt");
    
    for(i = 0; i < b->nodes.size(); i++)
      {
	fprintf(fp, "Node: %4d, Leaf: %1d, Eh: %7.4f, n: %6d, \n", (int) i, (int) b->nodes[i].leaf, b->nodes[i].Eh / b->nodes[i].n, b->nodes[i].n);
	
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
  
  void learn(txm_o& b, learner& base, example& ec)
  {
    MULTICLASS::multiclass *mc = (MULTICLASS::multiclass*)ec.ld;
    
    /*  verify_LR_dfs(b, b.nodes[0]);
    if (b.ex_num != sum_LR_dfs(b, b.nodes[0]))
      cout << "argagghgh " << b.ex_num << " " << sum_LR_dfs(b, b.nodes[0]) << endl;
    */
    if (mc->label == (uint32_t)-1 || !b.all->training || ec.test_only || b.progress)
      predict(b,base,ec);

    static size_t  ec_cnt = 0;

    if(b.all->training && (mc->label != (uint32_t)-1) && !ec.test_only)	//if training the tree
      {
	ec_cnt++;
	if(ec_cnt % 100000 == 0)
	  cout << ec_cnt << endl;	
	
	size_t class_index = 0;
	
	label_data simple_temp;
	simple_temp.initial = 0.0;
	simple_temp.weight = mc->weight;
	ec.ld = &simple_temp;	
	uint32_t oryginal_label = mc->label;	

	size_t cn = 0;

	while(1)
	  {
	    class_index = b.nodes[cn].node_pred.unique_add_sorted(txm_o_node_pred(oryginal_label));
	    
	    b.nodes[cn].node_pred[class_index].label_cnt2++;
	    
	    if(b.nodes[cn].node_pred[class_index].label_cnt2 > b.nodes[cn].max_cnt2)
	      {
		b.nodes[cn].max_cnt2 = b.nodes[cn].node_pred[class_index].label_cnt2;
		b.nodes[cn].max_cnt2_label = b.nodes[cn].node_pred[class_index].label;
	      }
	    
	    train_node(b, base, ec, cn, class_index);
	    
	    if(b.nodes[cn].leaf)
	      {	
		if(simple_temp.prediction < 0)
		  b.nodes[cn].L++;
		else
		  b.nodes[cn].R++;
		
		update_min_count(b, cn);	
	       	
		ec.ld = mc;
		break;	
	      }
	   
	    if(simple_temp.prediction < 0)
	      cn = b.nodes[cn].left;
	    else
	      cn = b.nodes[cn].right;	
	  }	
      }
      b.ex_num++;
  }
  
  void finish(txm_o& b)
  {
    //display_tree2(b);
    save_node_stats(b);
    cout << b.nbofswaps << endl;
    //fclose(b.ex_fp);
  }
  
  void save_load_tree(txm_o& b, io_buf& model_file, bool read, bool text)
  {
    if (model_file.files.size() > 0)
      {	
	char buff[512];
	uint32_t i = 0;
	uint32_t j = 0;
	size_t brw = 1;
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
		b.nodes[j].leaf = v;
	      }
      	    
	    cout << endl << endl;
	    cout << "ceil of log2(k): " << ceil_log2(b.k) << endl;
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
		
		text_len = sprintf(buff, ":%d\n", b.nodes[i].leaf);
		v = b.nodes[i].leaf;
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

    uint32_t i = ceil_log2(data->k);	
    data->max_nodes = (2 << (i+0)) - 1;
    
    learner* l = new learner(data, all.l, data->max_nodes + 1);
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

