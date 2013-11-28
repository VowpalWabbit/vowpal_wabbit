/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.node
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "parse_args.h"
#include "txm_o.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "oaa.h"

using namespace std;

namespace TXM_O 
{
	class txm_o_node_pred_type								//for every node I have a table of elements of this type with one entry per label
	{
		public:
		
		float 		Ehk;									//conditional margin over the label
		uint32_t 	nk;										//total number of data points labeled as 'label' reaching the node
		uint32_t	label;									//label
		uint32_t	label_cnt2;								//the number of examples reaching the node assigned through partitioner of the parent
		
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
		
		txm_o_node_pred_type(uint32_t l)					//constructor with label setting
		{
			label = l;
			Ehk = 0.f;
			nk = 0;
			label_cnt2 = 0;	
		}
	};
	
	typedef struct                                         //structure describing tree node
	{
		size_t id_left;
		size_t id_right;
		size_t level;                                      //root = 0
		size_t max_cnt2;                                   //maximal value of counter 2 in the node
		size_t max_cnt2_label;                             //label with maximal value of counter 2 in a node
		
		bool leaf;                                         //flag denoting that the node is a leaf
		v_array<txm_o_node_pred_type> node_pred;	

		float Eh;											//margin
		uint32_t n;											//total number of data points reaching the node
	} txm_o_node_type;
	
	struct txm_o
	{
		uint32_t k;
		vw* all;
		
		v_array<txm_o_node_type> nodes;						//the nodes - our tree
		v_array<size_t> out_node_list;						//the table of the indexes of leafs of principal path and alternative paths
		
		size_t max_depth; 	            					//maximal tree depth
	};	

	txm_o_node_type init_node(size_t id, size_t level)        //new node initialization
	{
		txm_o_node_type node;
		
		node.id_left = 0;
		node.id_right = 0;
		node.Eh = 0;
		node.n = 0;
		node.level = level;
		node.leaf = false;
		node.max_cnt2 = 0;
		node.max_cnt2_label = 0;
		
		return node;
	}
	
	void init_tree(txm_o* d)                               	//inicjalizacja drzewa
	{
		d->nodes.push_back(init_node(0, 0));
	}	
	
	void leafs_to_train(txm_o* d, learner& base, example* ec)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		label_data simple_temp;	
		bool empty_track = false;
	
		v_array<uint32_t> altdir_list;
		v_array<float> node_list_pred;
		
		d->out_node_list.erase();		
		
		uint32_t j, k;
		size_t cn = 0;		//ustawiamy root
		
		ec->test_only = true;
		
		for(j = 0; j <= d->max_depth; j++)				//loop over primal and alternative paths
		{
			while(!d->nodes[cn].leaf && !empty_track)
			{				
				simple_temp.initial = 0.0;				
				simple_temp.weight = mc->weight;
				simple_temp.label = FLT_MAX;
				ec->ld = &simple_temp.label;
				base.learn(ec, cn);	
				
				node_list_pred.push_back(fabs(ec->final_prediction));
	
				if(ec->final_prediction < 0)
				{					
					altdir_list.push_back(d->nodes[cn].id_right);
					cn = d->nodes[cn].id_left;
				}
				else
				{					
					altdir_list.push_back(d->nodes[cn].id_left);
					cn = d->nodes[cn].id_right;
				}
				
				if(cn == 0)								
					break;		
			}
			
			if(d->nodes[cn].leaf)
				d->out_node_list.push_back(cn);			
			
			float min_pred = node_list_pred[0];					//choosing the node with smallest prediction confidence (in the first run of loop for(j...) we choose only from principal path nodes)
			uint32_t min_pred_index = 0;
			for(k = 1; k < node_list_pred.size(); k++)
			{
				if(node_list_pred[k] < min_pred)
				{
					min_pred = node_list_pred[k];
					min_pred_index = k;
				}
			}
			
			cn = altdir_list[min_pred_index];
			
			node_list_pred[min_pred_index] = node_list_pred.pop();
			altdir_list[min_pred_index] = altdir_list.pop();			
			
			if(cn == 0)
				empty_track = true;	
			else
				empty_track = false;
		}		
		
		ec->ld = mc;		
		ec->test_only = false;	
	}
	
	void train_node(void* d, learner& base, example* ec, size_t& cn, size_t& index)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		txm_o* b = (txm_o*)d;
		
		label_data simple_temp;	
        simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;
		
		//do the initial prediction to decide if going left or right	
		simple_temp.label = FLT_MAX;
		ec->ld = &simple_temp;
		ec->test_only = true;
		base.learn(ec, cn);	
		ec->test_only = false;
		
		b->nodes[cn].Eh += ec->final_prediction;
		b->nodes[cn].n++;
		
		float norm_Eh = b->nodes[cn].Eh / b->nodes[cn].n;
		
		b->nodes[cn].node_pred[index].Ehk += ec->final_prediction;
		b->nodes[cn].node_pred[index].nk++;
		
		float norm_Ehk = b->nodes[cn].node_pred[index].Ehk / b->nodes[cn].node_pred[index].nk;
		
		float left_or_right = norm_Ehk - norm_Eh;
		
		size_t id_left = b->nodes[cn].id_left;
		size_t id_right = b->nodes[cn].id_right;		
	
		size_t id_left_right;
		
		if(left_or_right < 0)
		{
			simple_temp.label = -1.f;
			id_left_right = id_left;
		}
		else
		{
			simple_temp.label = 1.f;
			id_left_right = id_right;
		}
		
		if(id_left_right == 0)												//child does not exist
		{
			id_left_right = b->nodes.size();									//node identifier = number-of_existing_nodes + 1
			b->nodes.push_back(init_node(id_left_right, b->nodes[cn].level + 1));	//add new node to the tree
			if(left_or_right < 0)
				b->nodes[cn].id_left = id_left_right;											//new node is the left child of the current node	
			else
				b->nodes[cn].id_right = id_left_right;										//new node is the right child of the current node
		}	
		
		base.learn(ec, cn);
	}

	void learn(void* d, learner& base, example* ec)//(void* d, example* ec) 
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		txm_o* b = (txm_o*)d;
		
		size_t index = 0;
		
		label_data simple_temp;	
        simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;
		
		vw* all = b->all;	
		
		uint32_t oryginal_label = mc->label;				

		size_t current_level = 0;							
		
		size_t tmp_final_prediction;
		size_t cn = 0;
		while(1)
		{
			if(all->training && mc->label !=  (uint32_t)-1)
			{
				index = b->nodes[cn].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));	//add the label to the list of labels 
					
				b->nodes[cn].node_pred[index].label_cnt2++;
					
				if(b->nodes[cn].node_pred[index].label_cnt2 > b->nodes[cn].max_cnt2)			
				{
					b->nodes[cn].max_cnt2 = b->nodes[cn].node_pred[index].label_cnt2;
					b->nodes[cn].max_cnt2_label = b->nodes[cn].node_pred[index].label;
				}
			}

			if(b->nodes[cn].leaf || current_level >= b->max_depth)	//leaf level
			{					
				b->nodes[cn].leaf = true;	
				leafs_to_train(b, base, ec);		
	
				//train for OAA		
				uint32_t max_oaa_index = 0;									
				float max_oaa = -1.f;			
				ec->ld = &simple_temp;			
				string loss_function = "squared";			//IN PARSE_ARGS YOU HAVE "squaredloss", WHICH DOES NOT EXIST
				all->loss = getLossFunction(all, loss_function, (float)0.0);
				for(size_t j = 0; j < b->out_node_list.size(); j++)	//node list ma indeksy wszystkich lisci od pryncypalnej sciezki i alternatywnych sciezek
				{
					size_t id_current = b->out_node_list[j];
					
					if(b->nodes[id_current].max_cnt2_label == oryginal_label)
						simple_temp.label = 1.f;
					else
						simple_temp.label = -1.f;
								
					base.learn(ec, id_current);							//WHY COMMENTING THIS HAS ANY INFLUENCE ON VW??????????????????, ALSO OAA SHOULD USE DIFFERENT LOSS FUNCTION THAN INTERNAL NODES		
					
					if(ec->final_prediction > max_oaa)
					{
						max_oaa = ec->final_prediction;
						max_oaa_index = j;
					}
				}			
				loss_function = "quantile"; 
				all->loss = getLossFunction(all, loss_function, (float)0.5);
				cn = b->out_node_list[max_oaa_index];					//zakomentuj jezeli chcesz patrzec na blad samego drzewa					
				tmp_final_prediction = b->nodes[cn].max_cnt2_label;		//nie uwzgledniam oaa
				break;													//if mx depth is reached, finish
			}
			
			if(all->training && mc->label !=  (uint32_t)-1)
				train_node(d, base, ec, cn, index);			
			
			current_level++;
			
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp;
			ec->test_only = true;
			base.learn(ec, cn);	
			ec->test_only = false;
			
			if(ec->final_prediction < 0)
				cn = b->nodes[cn].id_left;
			else
				cn = b->nodes[cn].id_right;
		}				
		ec->ld = mc;
		ec->final_prediction = tmp_final_prediction;
	}
	
	void finish(void* data)
	{    
		txm_o* o=(txm_o*)data;
		free(o);
	}
	
	void save_load_tree(void* data, io_buf& model_file, bool read, bool text)
	{ 
		if (model_file.files.size() > 0)
		{		
			txm_o* g = (txm_o*)data;
			vw* all = g->all;
			txm_o* b = (txm_o*)all->l->get_save_load_data();
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
					b->nodes.push_back(init_node(j, 0));
					
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b->nodes[j].id_left = v;
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b->nodes[j].id_right = v;				
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b->nodes[j].max_cnt2_label = v;				
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					b->nodes[j].leaf = v;
				}
			}
			else
			{
				text_len = sprintf(buff, ":%d\n", (int) b->nodes.size());			//ilosc nodow
				v = b->nodes.size();
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);

				for(i = 0; i < b->nodes.size(); i++)
				{	
					text_len = sprintf(buff, ":%d", (int) b->nodes[i].id_left);
					v = b->nodes[i].id_left;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
					
					text_len = sprintf(buff, ":%d", (int) b->nodes[i].id_right);
					v = b->nodes[i].id_right;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
					
					text_len = sprintf(buff, ":%d", (int) b->nodes[i].max_cnt2_label);
					v = b->nodes[i].max_cnt2_label;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);				
					
					text_len = sprintf(buff, ":%d\n", b->nodes[i].leaf);
					v = b->nodes[i].leaf;
					brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);				
				}	
			}	
		}	
	}

	learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
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
		*(all.p->lp) = OAA::mc_label_parser;
	
		uint32_t i = 0;
		while (data->k > (uint32_t)(1 << i))
			i++;
	
		data->max_depth = i;	
		learner* l = new learner(data, learn, all.l, save_load_tree, 2 << data->max_depth);			
		l->set_finish_example(OAA::finish_example);
		
		txm_o* b = (txm_o*)data;
		
		if(all.training)
			init_tree(b);		
		
		return l;
	}	
}
