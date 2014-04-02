/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved. Released under a BSD (revised)
license as described in the file LICENSE.node
*/
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "parse_args.h"
#include "learner.h"
#include "txm_o.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "oaa.h"

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
  
  class txm_o_node_pred_type	
  {
  public:
    
    uint32_t Rk;	
    double Ehk;	
    float norm_Ehk;
    uint32_t nk;
    uint32_t label;	
    uint32_t label_cnt2;
 
    bool operator==(txm_o_node_pred_type v){
      return (label == v.label);
    }
    
    bool operator>(txm_o_node_pred_type v){
      if(label > v.label) return true;	
      return false;
    }
    
    bool operator<(txm_o_node_pred_type v){
      if(label < v.label) return true;	
      return false;
    }
    
    txm_o_node_pred_type(uint32_t l)
    {
      label = l;
      Ehk = 0.f;
      norm_Ehk = 0;
      nk = 0;
      label_cnt2 = 0;
      Rk = 0;	
    }
  };
  
  typedef struct
  {
    size_t id_left;
    size_t id_right;
    size_t level;
    size_t max_cnt2;
    size_t max_cnt2_label;

    uint32_t ec_count;
    uint32_t min_ec_count;

    uint32_t L;
    uint32_t R;
    float objective;
    
    uint32_t myL;
    uint32_t myR;
    bool leaf;
    v_array<txm_o_node_pred_type> node_pred;
    
    double Eh;	
    float norm_Eh;
    uint32_t n;	
  } txm_o_node_type;
  
  struct txm_o	
  {
    uint32_t k;	
    vw* all;	
    
    v_array<txm_o_node_type> nodes;	
    
    size_t max_depth;	
    size_t max_nodes;
    v_array<size_t> ec_path;
    v_array<size_t> min_ec_path;
    bool ec_cnt_update;

    size_t ex_num;
    FILE *ex_fp;
  };	
  
  txm_o_node_type init_node(size_t level)	
  {
    txm_o_node_type node; 
    
    node.id_left = 0;
    node.id_right = 0;
    node.Eh = 0;
    node.norm_Eh = 0;
    node.n = 0;
    node.level = level;
    node.leaf = true;
    node.max_cnt2 = 0;
    node.max_cnt2_label = 0;
    node.ec_count = 0;
    node.min_ec_count = 0;
    node.L = 0;
    node.R = 0;
    node.objective = 0;
    node.myL = 0;
    node.myR = 0;
    return node;
  }
  
  void init_tree(txm_o& d)
  {
    d.nodes.push_back(init_node(0));
    d.ex_num = 0;
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
  
  bool find_switch_nodes(txm_o& b, size_t& c, size_t& p, size_t& pp)	
  {
    c = 0;
    p = 0;
    pp = 0;
    bool lr;

    while(1)
      {
	b.min_ec_path.push_back(p);
	
	if(b.nodes[c].leaf)
	  {
	    return lr;
	  }
	
	if(b.nodes[b.nodes[c].id_left].min_ec_count < b.nodes[b.nodes[c].id_right].min_ec_count)
	  {
	    pp = p;
	    p = c;
	    c = b.nodes[c].id_left; 
	    lr = false;
	  }
	else
	  {
	    pp = p;
	    p = c;
	    c = b.nodes[c].id_right;
	    lr = true;
	  }     
      }    
  }
  
  void update_min_ec_count(txm_o& b, v_array<size_t>* arr)	
  {        
    while(arr->size() > 0)
      {
	size_t p = arr->pop();
	size_t l = b.nodes[p].id_left;
	size_t r = b.nodes[p].id_right;	
	
	if(b.nodes[l].leaf)
	  b.nodes[l].min_ec_count = b.nodes[l].ec_count;

	if(b.nodes[r].leaf)
	  b.nodes[r].min_ec_count = b.nodes[r].ec_count;

	if(b.nodes[l].min_ec_count < b.nodes[r].min_ec_count)
	  b.nodes[p].min_ec_count = b.nodes[l].min_ec_count;
	else
	  b.nodes[p].min_ec_count = b.nodes[r].min_ec_count;
      }
  }

  void display_tree2(txm_o& d)
  {
    size_t l, i;
    for(l = 0; l <= d.max_depth; l++)
      {
	for(i = 0; i < d.nodes.size(); i++)
	  {
	    if(d.nodes[i].level == l)
	      {	
		if(d.nodes[i].leaf)
		  cout << "[" << i << "," << d.nodes[i].max_cnt2_label << "," << d.nodes[i].max_cnt2 << "," << d.nodes[i].ec_count << "," << d.nodes[i].min_ec_count << "," << d.nodes[i].objective << "] ";
		else
		  cout << "(" << i << "," << d.nodes[i].max_cnt2_label << "," << d.nodes[i].max_cnt2 << "," << d.nodes[i].ec_count << "," << d.nodes[i].min_ec_count << "," << d.nodes[i].objective << ") ";
	      }
	  }
	cout << endl;
      }
    cout << endl;
    
    cout << endl;
    cout << "Tree depth: " << d.max_depth << endl;
    cout << "ceil of log2(k): " << ceil_log2(d.k) << endl;
  }

  void update_levels(txm_o& b, size_t n, size_t cl)
  {
    b.nodes[n].level = cl;

    if(b.nodes[n].id_left != 0)
      update_levels(b, b.nodes[n].id_left, cl + 1);

    if(b.nodes[n].id_right != 0)
      update_levels(b, b.nodes[n].id_right, cl + 1);
  }
          
  void train_node(txm_o& b, learner& base, example& ec, size_t& cn, size_t& index)
  {
    label_data* simple_temp = (label_data*)ec.ld;
       
    b.nodes[cn].objective = 0;
    float tmp1, tmp2, tmp3;
    for(size_t i = 0; i < b.nodes[cn].node_pred.size(); i++)
      {
	tmp1 = (float)b.nodes[cn].R / (float)b.nodes[cn].n;
	tmp2 = (float)b.nodes[cn].node_pred[i].Rk / (float)b.nodes[cn].node_pred[i].nk;
	tmp3 = (float)b.nodes[cn].node_pred[i].nk / (float)b.nodes[cn].n;
	b.nodes[cn].objective += tmp3 * fabs(tmp1 - tmp2);
      }
    
    float left_or_right = b.nodes[cn].node_pred[index].norm_Ehk - b.nodes[cn].norm_Eh;
    size_t id_left = b.nodes[cn].id_left;
    size_t id_right = b.nodes[cn].id_right;
    
    size_t id_left_right;	
    if(left_or_right < 0)
      {
	simple_temp->label = -1.f;
	id_left_right = id_left; 
	b.nodes[cn].myL++;
      }
    else
      {
	simple_temp->label = 1.f;
	id_left_right = id_right;
	b.nodes[cn].myR++;
      }
    
    if((b.nodes[cn].myR > 0) && (b.nodes[cn].myL > 0))
      {
	if(id_left_right == 0)
	  {	    
	    if(b.nodes.size() + 2 <= b.max_nodes)
	      {
		id_left_right = b.nodes.size();	
		b.nodes.push_back(init_node(b.nodes[cn].level + 1));	
		b.nodes.push_back(init_node(b.nodes[cn].level + 1));
		b.nodes[cn].id_left = id_left_right;
		b.nodes[cn].id_right = id_left_right + 1;
		
		b.nodes[b.nodes[cn].id_left].max_cnt2_label = b.nodes[cn].max_cnt2_label;
		b.nodes[b.nodes[cn].id_right].max_cnt2_label = b.nodes[cn].max_cnt2_label;

		if(b.nodes[cn].level + 1 > b.max_depth)
		    b.max_depth = b.nodes[cn].level + 1;	
	      }
	      else
	      {
		size_t nc, np, npp;
		bool lr = find_switch_nodes(b, nc, np, npp);
		size_t nc_l = b.nodes[nc].level;
		size_t trsh = b.nodes[0].ec_count >> (nc_l + 1);

		/*display_tree2(b);
		cout << cn << "\t" << nc << "\t" << np  << "\t" << npp << endl;
		cin.ignore();
		*/
		//if(((b.nodes[id_left].ec_count < trsh) || (b.nodes[id_right].ec_count < trsh)) && ratio_cn < ratio_cn_el && cn_el != 0 && id_left != cn && id_right != cn)
		if(b.nodes[0].min_ec_count < trsh && cn != nc && b.nodes[cn].ec_count > b.nodes[np].ec_count)
		{
		  //display_tree2(b);
		  cout << "\nSWAP!!" << endl;
		  cout << cn << "\t" << nc << "\t" << np  << "\t" << npp << endl;
		  // cin.ignore();

		  if(b.nodes[npp].id_left == np)
		    {
		      if(lr == false)
			b.nodes[npp].id_left = b.nodes[np].id_right;
		      else
			b.nodes[npp].id_left = b.nodes[np].id_left;

		      update_levels(b, b.nodes[npp].id_left, b.nodes[npp].level + 1);
		    }
		  else
		    {
		      if(lr == false)
			b.nodes[npp].id_right = b.nodes[np].id_right;
		      else
			b.nodes[npp].id_right = b.nodes[np].id_left;

		      update_levels(b, b.nodes[npp].id_right, b.nodes[npp].level + 1);
		    }

		  b.nodes[cn].id_left = np;
		  b.nodes[cn].id_right = nc;
		  b.nodes[np].leaf = true;
		  b.nodes[np].id_left = 0;
		  b.nodes[np].id_right = 0;
		  b.nodes[nc].leaf = true;
		  b.nodes[nc].id_left = 0;
		  b.nodes[nc].id_right = 0;
		  b.nodes[np].level = b.nodes[cn].level + 1;
		  b.nodes[nc].level = b.nodes[cn].level + 1;
		  
		  if(b.nodes[cn].level + 1 > b.max_depth)
		    b.max_depth = b.nodes[cn].level + 1;	    

		  b.min_ec_path.pop();
		  b.min_ec_path.pop();
		  update_min_ec_count(b, &b.min_ec_path);		    	
		  }
	      }
	  }	
      }
    base.learn(ec, cn);	

    simple_temp->label = FLT_MAX;
    base.predict(ec, cn);

    b.nodes[cn].Eh += (double)ec.partial_prediction;
    b.nodes[cn].node_pred[index].Ehk += (double)ec.partial_prediction;
    b.nodes[cn].n++;
    b.nodes[cn].node_pred[index].nk++;	
  
    b.nodes[cn].norm_Eh = b.nodes[cn].Eh / b.nodes[cn].n;          
    b.nodes[cn].node_pred[index].norm_Ehk = b.nodes[cn].node_pred[index].Ehk / b.nodes[cn].node_pred[index].nk;
    
    if(b.nodes[cn].id_left == 0)	
      b.nodes[cn].leaf = true;
    else	
      b.nodes[cn].leaf = false;	
  }
  
  void predict(txm_o& b, learner& base, example& ec)	
  {
    OAA::mc_label *mc = (OAA::mc_label*)ec.ld;
    
    label_data simple_temp;
    simple_temp.initial = 0.0;
    simple_temp.weight = mc->weight;	
    ec.ld = &simple_temp;
    size_t cn = 0;
    while(1)
      {
	if(b.nodes[cn].leaf)	
	  {	
	    ec.final_prediction = b.nodes[cn].max_cnt2_label;
	    ec.ld = mc;
	    break;	
	  }
	simple_temp.label = FLT_MAX;
	base.predict(ec, cn);
  
	if(ec.final_prediction < 0)//b.nodes[cn].Eh/b.nodes[cn].n)	
	  cn = b.nodes[cn].id_left;
	else
	  cn = b.nodes[cn].id_right;	
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
	fprintf(fp, "Node: %4d, Level: %2d, Leaf: %1d, Eh: %7.4f, n: %6d, \n", (int) i, (int) b->nodes[i].level, (int) b->nodes[i].leaf, b->nodes[i].Eh / b->nodes[i].n, b->nodes[i].n);
	
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
	fprintf(fp, "left: %4d, right: %4d", (int) b->nodes[i].id_left, (int) b->nodes[i].id_right);
	fprintf(fp, "\n\n");
      }
    
    fclose(fp);
  }	
  
  void learn(txm_o& b, learner& base, example& ec)
  {
    predict(b,base,ec);
    OAA::mc_label *mc = (OAA::mc_label*)ec.ld;
    b.ec_cnt_update = true;
    
    v_array<size_t> track;

    if(b.all->training && (mc->label != (uint32_t)-1) && !ec.test_only)	//if training the tree
      {
	size_t index = 0;
		
	label_data simple_temp;	
	simple_temp.initial = 0.0;
	simple_temp.weight = mc->weight;
	ec.ld = &simple_temp;	
	uint32_t oryginal_label = mc->label;	
		
	size_t tmp_final_prediction = ec.final_prediction;
	size_t cn = 0;
	size_t tmp;

	while(1)
	  {
	    //cout << cn << "\t";
	    
	    //if(!track.contain_sorted(cn, tmp))
	    //	track.unique_add_sorted(cn);
	    //else
	    //{	
	    //	cout << "loop detected!!";
	    //	cin.ignore();
	    //}
	
	    b.nodes[cn].ec_count++;

	    b.ec_path.push_back(cn);

	    index = b.nodes[cn].node_pred.unique_add_sorted(txm_o_node_pred_type(oryginal_label));
	    
	    b.nodes[cn].node_pred[index].label_cnt2++;
	    
	    if(b.nodes[cn].node_pred[index].label_cnt2 > b.nodes[cn].max_cnt2)
	      {
		b.nodes[cn].max_cnt2 = b.nodes[cn].node_pred[index].label_cnt2;
		b.nodes[cn].max_cnt2_label = b.nodes[cn].node_pred[index].label;
	      }
	    
	    train_node(b, base, ec, cn, index);

	    if(b.nodes[cn].leaf)
	      {	
		ec.final_prediction = tmp_final_prediction;	
		ec.ld = mc;

		b.ec_path.pop();
		update_min_ec_count(b, &b.ec_path);	
	       			
		break;	
	      }
	    
	    float left_or_right = b.nodes[cn].node_pred[index].norm_Ehk - b.nodes[cn].norm_Eh;
	    if(ec.final_prediction < 0)//b.nodes[cn].Eh/b.nodes[cn].n)
	      {
		b.nodes[cn].L++;
		cn = b.nodes[cn].id_left;
	      }	
	    else
	      {
		b.nodes[cn].R++;
		b.nodes[cn].node_pred[index].Rk++;
		cn = b.nodes[cn].id_right;	
	      }
	  }	
      }
      b.ex_num++;
      //cout << endl;
  }
  
  void finish(txm_o& b)
  {
    display_tree2(b);
    save_node_stats(b);
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
		b.nodes.push_back(init_node(0));
		
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].id_left = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].id_right = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].max_cnt2_label = v;
		brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
		b.nodes[j].leaf = v;
	      }
	  }
	else
	  {
	    //cout << endl;
	    // cout << "Tree depth: " << b.max_depth << endl;
	    // cout << "ceil of log2(k): " << ceil_log2(b.k) << endl;
	    
	    text_len = sprintf(buff, ":%d\n", (int) b.nodes.size());	//ilosc nodow
	    v = b.nodes.size();
	    brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
	    
	    for(i = 0; i < b.nodes.size(); i++)
	      {	
		text_len = sprintf(buff, ":%d", (int) b.nodes[i].id_left);
		v = b.nodes[i].id_left;
		brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
		
		text_len = sprintf(buff, ":%d", (int) b.nodes[i].id_right);
		v = b.nodes[i].id_right;
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
    OAA::output_example(all, ec);
    VW::finish_example(all, &ec);
  }
  
  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)	//learner setup
  {
    txm_o* data = (txm_o*)calloc(1, sizeof(txm_o));
    //first parse for number of actions
    if( vm_file.count("txm_o") )
      {
	data->k = (uint32_t)vm_file["txm_o"].as<size_t>();
	if( vm.count("txm_o") && (uint32_t)vm["txm_o"].as<size_t>() != data->k )
	  std::cerr << "warning: you specified a different number of actions through --txm_o than the one loaded from predictor. Pursuing with loaded value of: " << data->k << endl;
      }
    else
      {
	data->k = (uint32_t)vm["txm_o"].as<size_t>();
	
	//append txm_o with nb_actions to options_from_file so it is saved to regressor later
	std::stringstream ss;
	ss << " --txm_o " << data->k;
	all.options_from_file.append(ss.str());
      }	
    
    data->all = &all;
    (all.p->lp) = OAA::mc_label_parser;
    
    uint32_t i = ceil_log2(data->k);	
    data->max_nodes = (2 << i) - 1;
    
    learner* l = new learner(data, all.l, data->max_nodes + 1);
    l->set_save_load<txm_o,save_load_tree>();
    l->set_learn<txm_o,learn>();
    l->set_predict<txm_o,predict>();
    l->set_finish_example<txm_o,finish_example>();
    l->set_finish<txm_o,finish>();
    
    data->max_depth = 0;
    
    if(all.training)
      init_tree(*data);	
    
    return l;
  }	
}

