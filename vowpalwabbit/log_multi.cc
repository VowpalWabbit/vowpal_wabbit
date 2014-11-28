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
#include "vw.h"

using namespace std;
using namespace LEARNER;

namespace LOG_MULTI
{
  class node_pred	
  {
  public:
    
    double Ehk;	
    float norm_Ehk;
    uint32_t nk;
    uint32_t label;	
    uint32_t label_count;
 
    bool operator==(node_pred v){
      return (label == v.label);
    }
    
    bool operator>(node_pred v){
      if(label > v.label) return true;	
      return false;
    }
    
    bool operator<(node_pred v){
      if(label < v.label) return true;	
      return false;
    }
    
    node_pred(uint32_t l)
    {
      label = l;
      Ehk = 0.f;
      norm_Ehk = 0;
      nk = 0;
      label_count = 0;
    }
  };
  
  typedef struct
  {//everyone has
    uint32_t parent;//the parent node
    v_array<node_pred> preds;//per-class state
    uint32_t min_count;//the number of examples reaching this node (if it's a leaf) or the minimum reaching any grandchild.

    bool internal;//internal or leaf

    //internal nodes have
    uint32_t base_predictor;//id of the base predictor
    uint32_t left;//left child
    uint32_t right;//right child
    float norm_Eh;//the average margin at the node
    double Eh;//total margin at the node
    uint32_t n;//total events at the node

    //leaf has
    uint32_t max_count;//the number of samples of the most common label
    uint32_t max_count_label;//the most common label
  } node;
  
  struct log_multi
  {
    uint32_t k;	
    vw* all;	
    
    v_array<node> nodes;	
    
    uint32_t max_predictors;
    uint32_t predictors_used;

    bool progress;
    uint32_t swap_resist;

    uint32_t nbofswaps;
  };	
  
  inline void init_leaf(node& n)
  {
    n.internal = false;
    n.preds.erase();
    n.base_predictor = 0;
    n.norm_Eh = 0;
    n.Eh = 0;
    n.n = 0;
    n.max_count = 0;
    n.max_count_label = 1;
    n.left = 0;
    n.right = 0;
  }

  inline node init_node()	
  {
    node node; 
    
    node.parent = 0;
    node.min_count = 0;
    init_leaf(node);

    return node;
  }
  
  void init_tree(log_multi& d)
  {
    d.nodes.push_back(init_node());
    d.nbofswaps = 0;
  }

  inline uint32_t min_left_right(log_multi& b, node& n)
  {
    return min(b.nodes[n.left].min_count, b.nodes[n.right].min_count);
  }
  
  inline uint32_t find_switch_node(log_multi& b)	
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
  
  inline void update_min_count(log_multi& b, uint32_t node)	
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

  void display_tree_dfs(log_multi& b, node node, uint32_t depth)
  {
    for (uint32_t i = 0; i < depth; i++)
      cout << "\t";
    cout << node.min_count << " " << node.left
	 << " " << node.right;
    cout << " label = " << node.max_count_label << " labels = ";
    for (size_t i = 0; i < node.preds.size(); i++)
      cout << node.preds[i].label << ":" << node.preds[i].label_count << "\t";
    cout << endl;
    
    if (node.internal)
      {
	cout << "Left";
	display_tree_dfs(b, b.nodes[node.left], depth+1);
	
	cout << "Right";
	display_tree_dfs(b, b.nodes[node.right], depth+1);
      }	
  }

  bool children(log_multi& b, uint32_t& current, uint32_t& class_index, uint32_t label)
  {
    class_index = (uint32_t)b.nodes[current].preds.unique_add_sorted(node_pred(label));
    b.nodes[current].preds[class_index].label_count++;
    
    if(b.nodes[current].preds[class_index].label_count > b.nodes[current].max_count)
      {
	b.nodes[current].max_count = b.nodes[current].preds[class_index].label_count;
	b.nodes[current].max_count_label = b.nodes[current].preds[class_index].label;
      }

    if (b.nodes[current].internal)
      return true;
    else if( b.nodes[current].preds.size() > 1 
	     && (b.predictors_used < b.max_predictors 
				     || b.nodes[current].min_count - b.nodes[current].max_count > b.swap_resist*(b.nodes[0].min_count + 1)))
      { //need children and we can make them.
	uint32_t left_child;
	uint32_t right_child;
	if (b.predictors_used < b.max_predictors)
	  {
	    left_child = (uint32_t)b.nodes.size();
	    b.nodes.push_back(init_node());	
	    right_child = (uint32_t)b.nodes.size();
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
	    
	    init_leaf(b.nodes[swap_child]);
	    left_child = swap_child;
	    b.nodes[current].base_predictor = b.nodes[swap_parent].base_predictor;
	    init_leaf(b.nodes[swap_parent]);
	    right_child = swap_parent;
	  }
	b.nodes[current].left = left_child;
	b.nodes[left_child].parent = current;
	b.nodes[current].right = right_child;
	b.nodes[right_child].parent = current;
	
	b.nodes[left_child].min_count = b.nodes[current].min_count/2;
	b.nodes[right_child].min_count = b.nodes[current].min_count - b.nodes[left_child].min_count;
	update_min_count(b, left_child);

	b.nodes[left_child].max_count_label = b.nodes[current].max_count_label;
	b.nodes[right_child].max_count_label = b.nodes[current].max_count_label;

	b.nodes[current].internal = true;
      }
    return b.nodes[current].internal;
  }
  
  void train_node(log_multi& b, learner& base, example& ec, uint32_t& current, uint32_t& class_index)
  {
    if(b.nodes[current].norm_Eh > b.nodes[current].preds[class_index].norm_Ehk)
      ec.l.simple.label = -1.f;
    else
      ec.l.simple.label = 1.f;
    
    base.learn(ec, b.nodes[current].base_predictor);	

    ec.l.simple.label = FLT_MAX;
    base.predict(ec, b.nodes[current].base_predictor);
    
    b.nodes[current].Eh += (double)ec.partial_prediction;
    b.nodes[current].preds[class_index].Ehk += (double)ec.partial_prediction;
    b.nodes[current].n++;
    b.nodes[current].preds[class_index].nk++;	
  
    b.nodes[current].norm_Eh = (float)b.nodes[current].Eh / b.nodes[current].n;          
    b.nodes[current].preds[class_index].norm_Ehk = (float)b.nodes[current].preds[class_index].Ehk / b.nodes[current].preds[class_index].nk;
  }
  
  void verify_min_dfs(log_multi& b, node node)
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
  
  size_t sum_count_dfs(log_multi& b, node node)
  {
    if (node.internal)
      return sum_count_dfs(b, b.nodes[node.left]) + sum_count_dfs(b, b.nodes[node.right]);
    else
      return node.min_count;
  }

  inline uint32_t descend(node& n, float prediction)
  {
    if (prediction < 0)
      return n.left;
    else
      return n.right;
  }

  void predict(log_multi& b, learner& base, example& ec)	
  {
    MULTICLASS::multiclass mc = ec.l.multi;

    label_data simple_temp;
    simple_temp.initial = 0.0;
    simple_temp.weight = 0.0;	
    simple_temp.label = FLT_MAX;
    ec.l.simple = simple_temp;
    uint32_t cn = 0;
    while(b.nodes[cn].internal)
      {
	base.predict(ec, b.nodes[cn].base_predictor);
	cn = descend(b.nodes[cn], ec.pred.scalar);
      }	
    ec.pred.multiclass = b.nodes[cn].max_count_label;
    ec.l.multi = mc;
  }

  void learn(log_multi& b, learner& base, example& ec)
  {
    //    verify_min_dfs(b, b.nodes[0]);

    if (ec.l.multi.label == (uint32_t)-1 || !b.all->training || b.progress)
      predict(b,base,ec);
    
    if(b.all->training && (ec.l.multi.label != (uint32_t)-1) && !ec.test_only)	//if training the tree
      {
	MULTICLASS::multiclass mc = ec.l.multi;
    
	uint32_t class_index = 0;	
	label_data simple_temp;
	simple_temp.initial = 0.0;
	simple_temp.weight = mc.weight;
	ec.l.simple = simple_temp;	

	uint32_t cn = 0;

	while(children(b, cn, class_index, mc.label))
	  {	    
	    train_node(b, base, ec, cn, class_index);
	    cn = descend(b.nodes[cn], ec.pred.scalar);
	  }
	
	b.nodes[cn].min_count++;
	update_min_count(b, cn);	
	
	ec.l.multi = mc;
      }
  }
  
  void save_node_stats(log_multi& d)
  {
    FILE *fp;
    uint32_t i, j;
    uint32_t total;
    log_multi* b = &d;
    
    fp = fopen("atxm_debug.csv", "wt");
    
    for(i = 0; i < b->nodes.size(); i++)
      {
	fprintf(fp, "Node: %4d, Internal: %1d, Eh: %7.4f, n: %6d, \n", (int) i, (int) b->nodes[i].internal, b->nodes[i].Eh / b->nodes[i].n, b->nodes[i].n);
	
	fprintf(fp, "Label:, ");
	for(j = 0; j < b->nodes[i].preds.size(); j++)
	  {
	    fprintf(fp, "%6d,", (int) b->nodes[i].preds[j].label);
	  }	
	fprintf(fp, "\n");
	
	fprintf(fp, "Ehk:, ");
	for(j = 0; j < b->nodes[i].preds.size(); j++)
	  {
	    fprintf(fp, "%7.4f,", b->nodes[i].preds[j].Ehk / b->nodes[i].preds[j].nk);
	  }	
	fprintf(fp, "\n");
	
	total = 0;
	
	fprintf(fp, "nk:, ");
	for(j = 0; j < b->nodes[i].preds.size(); j++)
	  {
	    fprintf(fp, "%6d,", (int) b->nodes[i].preds[j].nk);
	    total += b->nodes[i].preds[j].nk;	
	  }	
	fprintf(fp, "\n");
	
	fprintf(fp, "max(lab:cnt:tot):, %3d,%6d,%7d,\n", (int) b->nodes[i].max_count_label, (int) b->nodes[i].max_count, (int) total);
	fprintf(fp, "left: %4d, right: %4d", (int) b->nodes[i].left, (int) b->nodes[i].right);
	fprintf(fp, "\n\n");
      }
    
    fclose(fp);
  }	
  
  void finish(log_multi& b)
  {
    save_node_stats(b);
    cout << "used " << b.nbofswaps << " swaps" << endl;
  }
  
  void save_load_tree(log_multi& b, io_buf& model_file, bool read, bool text)
  {
    if (model_file.files.size() > 0)
      {	
	char buff[512];
	
	uint32_t text_len = sprintf(buff, "k = %d ",b.k);
	bin_text_read_write_fixed(model_file,(char*)&b.max_predictors, sizeof(b.k), "", read, buff, text_len, text);
	uint32_t temp = (uint32_t)b.nodes.size();
	text_len = sprintf(buff, "nodes = %d ",temp);
	bin_text_read_write_fixed(model_file,(char*)&temp, sizeof(temp), "", read, buff, text_len, text);
	if (read)
	  for (uint32_t j = 1; j < temp; j++)
	    b.nodes.push_back(init_node());
	text_len = sprintf(buff, "max_predictors = %d ",b.max_predictors);
	bin_text_read_write_fixed(model_file,(char*)&b.max_predictors, sizeof(b.max_predictors), "", read, buff, text_len, text);

	text_len = sprintf(buff, "predictors_used = %d ",b.predictors_used);
	bin_text_read_write_fixed(model_file,(char*)&b.predictors_used, sizeof(b.predictors_used), "", read, buff, text_len, text);

	text_len = sprintf(buff, "progress = %d ",b.progress);
	bin_text_read_write_fixed(model_file,(char*)&b.progress, sizeof(b.progress), "", read, buff, text_len, text);
	
	text_len = sprintf(buff, "swap_resist = %d\n",b.swap_resist);
	bin_text_read_write_fixed(model_file,(char*)&b.swap_resist, sizeof(b.swap_resist), "", read, buff, text_len, text);
	
	for (size_t j = 0; j < b.nodes.size(); j++)
	  {//Need to read or write nodes.
	    node& n = b.nodes[j];
	    text_len = sprintf(buff, " parent = %d",n.parent);
	    bin_text_read_write_fixed(model_file,(char*)&n.parent, sizeof(n.parent), "", read, buff, text_len, text);

	    uint32_t temp = (uint32_t)n.preds.size();
	    text_len = sprintf(buff, " preds = %d",temp);
	    bin_text_read_write_fixed(model_file,(char*)&temp, sizeof(temp), "", read, buff, text_len, text);
	    if (read)
	      for (uint32_t k = 0; k < temp; k++)
		n.preds.push_back(node_pred(1));

	    text_len = sprintf(buff, " min_count = %d",n.min_count);
	    bin_text_read_write_fixed(model_file,(char*)&n.min_count, sizeof(n.min_count), "", read, buff, text_len, text);

	    uint32_t text_len = sprintf(buff, " internal = %d",n.internal);
	    bin_text_read_write_fixed(model_file,(char*)&n.internal, sizeof(n.internal), "", read, buff, text_len, text)
;

	    if (n.internal)
	      {
		text_len = sprintf(buff, " base_predictor = %d",n.base_predictor);
		bin_text_read_write_fixed(model_file,(char*)&n.base_predictor, sizeof(n.base_predictor), "", read, buff, text_len, text);

		text_len = sprintf(buff, " left = %d",n.left);
		bin_text_read_write_fixed(model_file,(char*)&n.left, sizeof(n.left), "", read, buff, text_len, text);
		
		text_len = sprintf(buff, " right = %d",n.right);
		bin_text_read_write_fixed(model_file,(char*)&n.right, sizeof(n.right), "", read, buff, text_len, text);

		text_len = sprintf(buff, " norm_Eh = %f",n.norm_Eh);
		bin_text_read_write_fixed(model_file,(char*)&n.norm_Eh, sizeof(n.norm_Eh), "", read, buff, text_len, text);

		text_len = sprintf(buff, " Eh = %f",n.Eh);
		bin_text_read_write_fixed(model_file,(char*)&n.Eh, sizeof(n.Eh), "", read, buff, text_len, text);

		text_len = sprintf(buff, " n = %d\n",n.n);
		bin_text_read_write_fixed(model_file,(char*)&n.n, sizeof(n.n), "", read, buff, text_len, text);
	      }
	    else
	      {
		text_len = sprintf(buff, " max_count = %d",n.max_count);
		bin_text_read_write_fixed(model_file,(char*)&n.max_count, sizeof(n.max_count), "", read, buff, text_len, text);
		text_len = sprintf(buff, " max_count_label = %d\n",n.max_count_label);
		bin_text_read_write_fixed(model_file,(char*)&n.max_count_label, sizeof(n.max_count_label), "", read, buff, text_len, text);		
	      }

	    for (size_t k = 0; k < n.preds.size(); k++)
	      {
		node_pred& p = n.preds[k];
		
		text_len = sprintf(buff, "  Ehk = %f",p.Ehk);
		bin_text_read_write_fixed(model_file,(char*)&p.Ehk, sizeof(p.Ehk), "", read, buff, text_len, text);

		text_len = sprintf(buff, " norm_Ehk = %f",p.norm_Ehk);
		bin_text_read_write_fixed(model_file,(char*)&p.norm_Ehk, sizeof(p.norm_Ehk), "", read, buff, text_len, text);

		text_len = sprintf(buff, " nk = %d",p.nk);
		bin_text_read_write_fixed(model_file,(char*)&p.nk, sizeof(p.nk), "", read, buff, text_len, text);

		text_len = sprintf(buff, " label = %d",p.label);
		bin_text_read_write_fixed(model_file,(char*)&p.label, sizeof(p.label), "", read, buff, text_len, text);

		text_len = sprintf(buff, " label_count = %d\n",p.label_count);
		bin_text_read_write_fixed(model_file,(char*)&p.label_count, sizeof(p.label_count), "", read, buff, text_len, text);	
	      }
	  }
      }
  }
  
  void finish_example(vw& all, log_multi&, example& ec)
  {
    MULTICLASS::output_example(all, ec);
    VW::finish_example(all, &ec);
  }
  
  learner* setup(vw& all, po::variables_map& vm)	//learner setup
  {
    log_multi* data = (log_multi*)calloc(1, sizeof(log_multi));

    po::options_description opts("TXM Online options");
    opts.add_options()
      ("no_progress", "disable progressive validation")
      ("swap_resistance", po::value<uint32_t>(&(data->swap_resist))->default_value(4), "higher = more resistance to swap, default=4");
    
    vm = add_options(all, opts);
    
    data->k = (uint32_t)vm["log_multi"].as<size_t>();
    
    //append log_multi with nb_actions to options_from_file so it is saved to regressor later
    std::stringstream ss;
    ss << " --log_multi " << data->k;
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
    l->set_save_load<log_multi,save_load_tree>();
    l->set_learn<log_multi,learn>();
    l->set_predict<log_multi,predict>();
    l->set_finish_example<log_multi,finish_example>();
    l->set_finish<log_multi,finish>();
    
    init_tree(*data);	
    
    return l;
  }	
}
