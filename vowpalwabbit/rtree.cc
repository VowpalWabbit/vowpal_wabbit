/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.node
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <ctime>

#include "parse_args.h"
#include "learner.h"
#include "rtree.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "oaa.h"

using namespace std;
using namespace LEARNER;

namespace RTREE
{
	uint32_t ceil_log2(uint32_t k)
	{
		uint32_t i = 0;
		
		while (k > (uint32_t)(1 << i))
			i++;
			
		return i;
	}
	
	uint32_t my_rand(uint32_t &x)								//generator of pseudo-random numbers
	{
		x = x * 16807 % 2147483647;
		
		return x;
	}
	
	class rtree_node_pred_type								//for every node I have a table of elements of this type with one entry per label
	{
		public:
		
		uint32_t	label;									//label
		uint32_t	label_cnt;								//the number of examples reaching the node assigned through partitioner of the parent
		
		bool operator==(rtree_node_pred_type v){
			return (label == v.label);
		}
		
		bool operator>(rtree_node_pred_type v){
			if(label > v.label) return true;		
			return false;
		}
		
		bool operator<(rtree_node_pred_type v){
			if(label < v.label) return true;		
			return false;
		}
		
		rtree_node_pred_type(uint32_t l)					//constructor with label setting
		{
			label = l;
			label_cnt = 0;	
		}
	};
	
	typedef struct                                         //structure describing tree node
	{
		size_t id_left;
		size_t id_right;
		size_t level;                                      //root = 0
		size_t max_cnt;                                   //maximal value of counter 2 in the node
		size_t max_cnt_label;                             //label with maximal value of counter 2 in a node
		size_t initial_label;
		int8_t initial_dir;
		uint32_t rand_num;
				
		bool leaf;                                         //flag denoting that the node is a leaf
		v_array<rtree_node_pred_type> node_pred;	
		v_array<size_t> left_labels;
		v_array<size_t> right_labels;

		uint32_t n;											//total number of data points reaching the node
	} rtree_node_type;
	
	struct rtree
	{
		uint32_t k;
		vw* all;
		
		v_array<rtree_node_type> nodes;						//the nodes - our tree
		
		size_t max_depth; 	            					//maximal tree depth
		size_t max_nodes;
		
		//default_random_engine *generator;
	};	

	rtree_node_type init_node(size_t level, rtree& d)        //new node initialization
	{
		rtree_node_type node;	
		uint32_t now = (uint32_t)time(0);
		now += d.nodes.size();
		
		node.id_left = 0;
		node.id_right = 0;
		node.n = 0;
		node.level = level;
		node.leaf = true;
		node.max_cnt = 0;
		node.max_cnt_label = 0;
		node.initial_label = 0;
		node.initial_dir = 0;
		node.rand_num = my_rand(now);
		return node;
	}
	
	void init_tree(rtree& d)                               	//inicjalizacja drzewa
	{
		d.nodes.push_back(init_node(0, d));
	} 
	
	void train_node(rtree& b, learner& base, example& ec, size_t& cn, size_t& index) //return true when reaching leaf
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec.ld;
		
		label_data simple_temp; 
		simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;		
		
		bool left_or_right;
		size_t id_left = b.nodes[cn].id_left;
		size_t id_right = b.nodes[cn].id_right;		
		size_t id_left_right;
		size_t loc_index;	
		size_t cl = mc->label;
											//every node has two additional tables: a table of containing lables it was sending to the left and to the right
		if(b.nodes[cn].left_labels.contain_sorted(cl, loc_index))		//if the label of ec was in the table containing the labels that were directed to the left, set left_or_rght = 1
			left_or_right = (bool) 1;
		else if(b.nodes[cn].right_labels.contain_sorted(cl, loc_index))	//if the label of ec was in the table containing the labels that were directed to the left, set left_or_rght = 0
			left_or_right = (bool) 0;		
		else
			left_or_right = (bool)(my_rand(b.nodes[cn].rand_num) & 1);	//if the label of ec was not yet seen in cn, set left_or_right randomly (take LSB (least significant bit) of pseudo-random number)
			
		
		if(left_or_right)
		{
			simple_temp.label = -1.f;
			id_left_right = id_left;
			b.nodes[cn].left_labels.unique_add_sorted(cl);
		}
		else
		{
			simple_temp.label = 1.f;
			id_left_right = id_right;			
			b.nodes[cn].right_labels.unique_add_sorted(cl);
		}
		
		if(b.nodes[cn].initial_dir == 0)
		{
			b.nodes[cn].initial_label = cl;
			b.nodes[cn].initial_dir = (int8_t)simple_temp.label;
		}
		else if(id_left_right == 0)												//child does not exist
		{
			if(b.nodes[cn].initial_dir != (int8_t)simple_temp.label && b.nodes[cn].level < b.max_depth)			//stopping criterion is the tree depth
			{
				id_left_right = b.nodes.size();									//node identifier = number-of_existing_nodes + 1
				b.nodes.push_back(init_node(b.nodes[cn].level + 1, b));	//add new node to the tree
				b.nodes.push_back(init_node(b.nodes[cn].level + 1, b));	//add new node to the tree
				b.nodes[cn].id_left = id_left_right;											//new node is the left child of the current node	
				b.nodes[cn].id_right = id_left_right + 1;	
				
				if(b.nodes[cn].level + 1 > b.max_depth)
					b.max_depth = b.nodes[cn].level + 1;
			}
		}	
		
		ec.ld = &simple_temp;
		base.learn(ec, cn);		
		ec.ld = mc;
		
		if(b.nodes[cn].id_left == 0)
			b.nodes[cn].leaf = true;
		else	
			b.nodes[cn].leaf = false;
	}

	void learn(rtree& b, learner& base, example& ec)//(void* d, example* ec) 
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec.ld;
		
		size_t index = 0;
		
		label_data simple_temp;	
		simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;
		
		vw* all = b.all;	
		
		uint32_t oryginal_label = mc->label;				

		size_t current_level = 0;			

		size_t tmp_final_prediction;
		size_t cn = 0;
		while(1)
		{
			if(all->training && mc->label !=  (uint32_t)-1 && !ec.test_only)
			{
				index = b.nodes[cn].node_pred.unique_add_sorted(rtree_node_pred_type(oryginal_label));	//add the label to the list of labels 
					
				b.nodes[cn].node_pred[index].label_cnt++;
					
				if(b.nodes[cn].node_pred[index].label_cnt > b.nodes[cn].max_cnt)			
				{
					b.nodes[cn].max_cnt = b.nodes[cn].node_pred[index].label_cnt;
					b.nodes[cn].max_cnt_label = b.nodes[cn].node_pred[index].label;
				}
			}
			
			if(all->training && mc->label !=  (uint32_t)-1 && !ec.test_only)
				train_node(b, base, ec, cn, index);

			if(b.nodes[cn].leaf)	
			{					
				tmp_final_prediction = b.nodes[cn].max_cnt_label;		//nie uwzgledniam oaa
				break;													//if mx depth is reached, finish
			}		
			
			simple_temp.label = FLT_MAX;
			ec.ld = &simple_temp;
			ec.test_only = true;
			base.learn(ec, cn);	
			ec.test_only = false;
			ec.ld = mc;			
				
			if(ec.final_prediction < 0)
				cn = b.nodes[cn].id_left;
			else
				cn = b.nodes[cn].id_right;
			
			current_level++;
		}		
		ec.final_prediction = tmp_final_prediction;
	}
	
	void finish(void* data)
	{    
		rtree* o=(rtree*)data;
		free(o);
	}
	
	void save_load_tree(rtree& b, io_buf& model_file, bool read, bool text)
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
					b.nodes.push_back(init_node(0, b));
					
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b.nodes[j].id_left = v;
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b.nodes[j].id_right = v;				
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b.nodes[j].max_cnt_label = v;				
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b.nodes[j].leaf = v;
				}
			}
			else
			{
				cout << endl;
				cout << "Tree depth: " << b.max_depth << endl;
				cout << "ceil of log2(k): " << ceil_log2(b.k) << endl;
				
				text_len = sprintf(buff, ":%d\n", (int) b.nodes.size());			//ilosc nodow
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
					
					text_len = sprintf(buff, ":%d", (int) b.nodes[i].max_cnt_label);
					v = b.nodes[i].max_cnt_label;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);				
					
					text_len = sprintf(buff, ":%d\n", b.nodes[i].leaf);
					v = b.nodes[i].leaf;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);				
				}	
			}	
		}	
	}	

  void finish_example(vw& all, rtree&, example& ec)
  {
    OAA::output_example(all, ec);
    VW::finish_example(all, &ec);
  }

	learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
	{
		rtree* data = (rtree*)calloc(1, sizeof(rtree));
		//first parse for number of actions
		if( vm_file.count("rtree") ) 
		{
			data->k = (uint32_t)vm_file["rtree"].as<size_t>();
			if( vm.count("rtree") && (uint32_t)vm["rtree"].as<size_t>() != data->k )
				std::cerr << "warning: you specified a different number of actions through --rtree than the one loaded from predictor. Pursuing with loaded value of: " << data->k << endl;
		}
		else 
		{
			data->k = (uint32_t)vm["rtree"].as<size_t>();

			//append rtree with nb_actions to options_from_file so it is saved to regressor later
			std::stringstream ss;
			ss << " --rtree " << data->k;
			all.options_from_file.append(ss.str());
		}
				
		data->all = &all;
		(all.p->lp) = OAA::mc_label_parser;
	
		uint32_t i = ceil_log2(data->k);
	
		data->max_depth = i;	//max tree depth					
		data->max_nodes = (2 << data->max_depth) - 1;	 //max number of nodes
		
		learner* l = new learner(data, all.l, 2 << data->max_depth);	//initialize learner (last input: number of regressors that the tree will use)	
		l->set_save_load<rtree,save_load_tree>();
		l->set_learn<rtree,learn>();
		l->set_finish_example<rtree,finish_example>();

		if(all.training)
			init_tree(*data);		
		return l;
	}	
}
