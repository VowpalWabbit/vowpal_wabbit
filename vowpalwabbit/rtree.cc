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

#include "reductions.h"
#include "simple_label.h"
#include "multiclass.h"

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
	
	typedef struct                                         //structure describing tree node
	{
		uint32_t* labels;								//total number of data points reaching the node
		uint32_t label;
	} rtree_node_type;
	
	struct rtree
	{
		uint32_t k;
		vw* all;
		uint32_t x;
		
		v_array<rtree_node_type> nodes;						//the nodes - our tree
		
		//default_random_engine *generator;
	};	

	rtree_node_type init_node(rtree& d)        //new node initialization
	{
		rtree_node_type node;
		size_t tmp = d.k / 32 + 1;	
		node.labels = new uint32_t[tmp];
		for(size_t i = 0; i < tmp; i++)
			node.labels[i] = 0;
		node.label = 0;
		return node;
	}
	
	bool node_check_label(rtree& d, size_t cn, uint32_t label) {
		size_t a = label / 32;
		size_t b = label % 32;
		uint32_t mask = 1 << b;
		if(d.nodes[cn].labels[a] & mask) 
			return true;
		return false;
	}
	
	void node_add_label(rtree& d, size_t cn, uint32_t label) {
		size_t a = label / 32;
		size_t b = label % 32;
		uint32_t mask = 1 << b;
		//cout << cn << ": " << a << "\t" << b << "\t" << mask << endl;
		d.nodes[cn].labels[a] |= mask;
	}
	
	void init_tree(rtree& d)                               	//inicjalizacja drzewa
	{
		size_t i = 0, j = 1;
		uint32_t *labels = new uint32_t[d.k];
		size_t lcnt = d.k;	
		size_t llen = d.k / 32 + 1;	
		d.x = (uint32_t)time(0);
		
		for(i = 0; i < d.k; i++)
			labels[i] = i + 1;
		
		for(i = 0; i < 2 * d.k; i++)
			d.nodes.push_back(init_node(d));
			
		j = 1;	
		for(i = d.k; i < 2 * d.k; i++) {
			d.x = my_rand(d.x);
			j = d.x % lcnt; 
			node_add_label(d, i, labels[j]);
			d.nodes[i].label = labels[j];
			lcnt--;
			labels[j] = labels[lcnt];
		}
		
		for(i = d.k - 1; i > 0; i--) {
			for(j = 0; j < llen; j++) 
				d.nodes[i].labels[j] = d.nodes[2 * i].labels[j] | d.nodes[2 * i + 1].labels[j];
		}
		
		/*for(i = 1; i < 2 * d.k; i++) {
			cout << i << ": ";
			for(j = 1; j <= d.k; j++) {
				if(node_check_label(d, i, j)) 
					cout << j << ", ";
			}
			cout << endl; 	
		}
		cin.ignore();*/
	} 
	
	size_t train_node(rtree& b, learner& base, example& ec, size_t& cn, size_t& index, uint32_t cl) //return true when reaching leaf
	{
		MULTICLASS::mc_label *mc = (MULTICLASS::mc_label*)ec.ld;
		
		label_data simple_temp; 
		simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;		
		
		size_t new_cn;
		
		if(node_check_label(b, 2 * cn, cl)) {
			new_cn = 2 * cn;
			simple_temp.label = -1.f;
		} else if(node_check_label(b, 2 * cn + 1, cl)) {
			new_cn = 2 * cn + 1;
			simple_temp.label = 1.f;
		} else {
			cout << "Error - label not found - " << cl << "\t" << cn << endl;
			new_cn = 2 * cn;
		}
			
		ec.ld = &simple_temp;
		base.learn(ec, cn);		
		ec.ld = mc;	
		
		return new_cn;
	}
	
	void predict(rtree& b, learner& base, example& ec) {
		MULTICLASS::mc_label *mc = (MULTICLASS::mc_label*)ec.ld;

		label_data simple_temp;
		simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;	
		ec.ld = &simple_temp;
		size_t cn = 0;
		while(1)
		{
			if(cn >= b.k)	
			{					
				ec.final_prediction = b.nodes[cn].label;
				ec.ld = mc;
				break;													//if mx depth is reached, finish
			}
			simple_temp.label = FLT_MAX;
			base.predict(ec, cn);

			if(ec.final_prediction < 0)
				cn = 2 * cn;
			else
				cn = 2 * cn + 1;	
		}	
	}

	void learn(rtree& b, learner& base, example& ec)//(void* d, example* ec) 
	{
		predict(b,base,ec);
		size_t tmp_final_prediction = ec.final_prediction;
		
		MULTICLASS::mc_label *mc = (MULTICLASS::mc_label*)ec.ld;
		
		if(b.all->training && (mc->label != (uint32_t)-1) && !ec.test_only)	//if training the tree
		{		
			size_t index = 0;
		
			label_data simple_temp;	
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
		
			vw* all = b.all;	
		
			uint32_t oryginal_label = mc->label;				

			size_t cn = 1;
			while(1)
			{
				if(cn >= b.k)	
				{					
					//tmp_final_prediction = b.nodes[cn].label;		//nie uwzgledniam oaa
					break;													//if mx depth is reached, finish
				}
			
				if(all->training && mc->label !=  (uint32_t)-1 && !ec.test_only) 
					cn = train_node(b, base, ec, cn, index, oryginal_label);
				else {			
					simple_temp.label = FLT_MAX;
					ec.ld = &simple_temp;
					ec.test_only = true;
					base.learn(ec, cn);	
					ec.test_only = false;
					ec.ld = mc;			
				
					if(ec.final_prediction < 0)
						cn = 2 * cn;
					else
						cn = 2 * cn + 1;
				}
			}		
			ec.final_prediction = tmp_final_prediction;
			ec.ld = mc;
		}
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
					b.nodes.push_back(init_node(b));
				
				for(j = 0; j < i; j++)
				{				
					b.nodes.push_back(init_node(b));
					
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b.nodes[j + i].label = v;
				}
			}
			else
			{	
				text_len = sprintf(buff, ":%d\n", (int) b.nodes.size());			
				v = b.k;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);

				for(i = 0; i < b.k; i++)
				{	
					text_len = sprintf(buff, ":%d", (int) b.nodes[i + b.k].label);
					v = b.nodes[i + b.k].label;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);					
				}	
			}	
		}	
	}	

  void finish_example(vw& all, rtree&, example& ec)
  {
    MULTICLASS::output_example(all, ec);
    VW::finish_example(all, &ec);
  }
  
  void finish(rtree& b)
  {
  
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
		(all.p->lp) = MULTICLASS::mc_label_parser;
	
		learner* l = new learner(data, all.l, data->k + 1);	//initialize learner (last input: number of regressors that the tree will use)	
		l->set_save_load<rtree,save_load_tree>();
		l->set_learn<rtree,learn>();
		l->set_predict<rtree,predict>();
		l->set_finish_example<rtree,finish_example>();
		l->set_finish<rtree,finish>();

		if(all.training)
			init_tree(*data);		
		return l;
	}	
}
