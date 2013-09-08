/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.node
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "txm.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "oaa.h"

using namespace std;

namespace TXM 
{
	class txm_node_pred_type
	{
		public:
		
		float 		Ehk;
		uint32_t 	nk;
		uint32_t	label;
		uint32_t	label_cnt;
		uint32_t	label_cnt2;
		
		void operator=(txm_node_pred_type v)
		{
			Ehk = v.Ehk;
			nk = v.nk;
			label = v.label;
			label_cnt = v.label_cnt;
			label_cnt2 = v.label_cnt2;
		}
		
		bool operator==(txm_node_pred_type v){
			return (label == v.label);
		}
		
		bool operator>(txm_node_pred_type v){
			if(label > v.label) return true;		
			return false;
		}
		
		bool operator<(txm_node_pred_type v){
			if(label < v.label) return true;		
			return false;
		}
		
		txm_node_pred_type(uint32_t l)
		{
			label = l;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			label_cnt2 = 0;
		}
		
		txm_node_pred_type()
		{
			label = 0;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			label_cnt2 = 0;
		}
	};
	
	typedef struct
	{
		size_t id;
		size_t id_left;
		size_t id_right;
		size_t id_parent;
		size_t level;
		uint32_t wrong;
		uint32_t correct;
		bool leaf;
		bool removed;
		size_t max_cnt2;
		size_t max_cnt2_label;
		size_t total_cnt2;
		
		v_array<txm_node_pred_type> node_pred;	

		float Eh;
		uint32_t n;		
	} txm_node_type;
	
	struct txm
	{
		uint32_t k;
		uint32_t increment;
		learner base;
		vw* all;
		
		v_array<size_t> ex_node;		//the node where example was sent
		v_array<txm_node_type> nodes;	//the nodes
		
		size_t cn;						//current node
		size_t ex_num;					//index of current example
		size_t ex_total;
		
		size_t current_pass;			//index of current pass through the data	
		size_t level_limit;
		
		bool only_leafs;
		bool tree_finished;
		
		uint32_t total_wrg;
		uint32_t total_cor;
	};	

	txm_node_type init_node(size_t id, size_t id_parent, size_t level)
	{
		txm_node_type node;
		
		node.id = id;
		node.id_parent = id_parent;
		node.id_left = 0;
		node.id_right = 0;
		node.Eh = 0;
		node.n = 0;
		node.level = level;
		node.leaf = false;
		node.removed = false;
		node.max_cnt2 = 0;
		node.max_cnt2_label = 0;
		node.total_cnt2 = 0;
		node.wrong = 0;
		node.correct = 0;
		
		return node;
	}
	
	void init_tree(txm* d)
	{
		d->cn = 0;
		d->ex_num = 0;
		d->nodes.push_back(init_node(0, 0, 0));
		d->tree_finished = false;
		d->only_leafs = false;
	}
	
	void display_tree(txm* d)
	{
		size_t i;
		size_t j;
		size_t level = 0;
		
		for(j = 0; j < d->nodes.size(); j++)
		{
			if(d->nodes[j].level > level)
			{
				cout << endl;
				level++;
			}
			
			if(d->nodes[j].removed)
				continue;
			
			cout << j << ":";
			
			if(d->nodes[j].leaf)
				cout << "[";
			else
				cout << "(";
			
			for(i = 0; i < d->nodes[j].node_pred.size(); i++)
			{
				cout << d->nodes[j].node_pred[i].label;
				
				if(i < d->nodes[j].node_pred.size() - 1)
					cout << ",";
			}
			
			if(d->nodes[j].leaf)
				cout << "] ";
			else
				cout << ") ";			
		}
		
		cout << endl;	
		cout << endl;		
	}
	
	void display_node_stats(txm* b)
	{
		uint32_t i, j;
		
		cout << endl;
				
		for(i = 0; i < b->nodes.size(); i++)
		{
			//if(b->nodes[i].level == b->current_pass)
			{					
				if(b->nodes[i].removed)
					continue;
				
				cout << "Node:Level: " << i << ":" << b->nodes[i].level << endl;
				cout << "labels: ";
				
				for(j = 0; j < b->nodes[i].node_pred.size(); j++)
				{
					printf("[%3d:%6d] ", b->nodes[i].node_pred[j].label, b->nodes[i].node_pred[j].label_cnt);//cout << b->nodes[i].node_pred[j].label << ":" << b->nodes[i].node_pred[j].label_cnt << "\t";
				}						
				cout << endl;
				
				cout << "labels: ";
				
				for(j = 0; j < b->nodes[i].node_pred.size(); j++)
				{
					printf("[%3d:%6d] ", b->nodes[i].node_pred[j].label, b->nodes[i].node_pred[j].label_cnt2);//cout << b->nodes[i].node_pred[j].label << ":" << b->nodes[i].node_pred[j].label_cnt2 << "\t";
				}
				cout << endl;
				
				printf("max(label:cnt:total): %3d:%6d:%7d",  b->nodes[i].max_cnt2_label, b->nodes[i].max_cnt2, b->nodes[i].total_cnt2);
				//cout << "max:\t" << b->nodes[i].max_cnt2_label << ":" << b->nodes[i].max_cnt2 << "\ttotal:\t" << b->nodes[i].total_cnt2;
				
				cout << endl;
				cout << endl;
			}
		}
	}

	void predict(txm* d, example* ec)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		vw* all = d->all;
		static int wrong = 0;
		static int correct = 0;	
		size_t new_cn;
		static uint32_t err_cnt = 0;
		//float Eh_norm;
		
		#ifdef TXM_DEBUG_PRED
		int level = 0;
		#endif
		
		if(command_example(all,ec))
		{	
			return;
		}
		
		d->cn = 0;
		
		#ifdef TXM_DEBUG_PRED
		cout << "\nExample: " << d->ex_num << endl;
		#endif
		
		while(!d->nodes[d->cn].leaf && d->nodes[d->cn].level < d->level_limit)
		{
			ec->test_only = true;
			d->base.learn(ec);
			
			#ifdef TXM_DEBUG_PRED
			cout << "level: " << level++ << endl;
			cout << "node: " << d->cn << endl;
			#endif
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			//Eh_norm = d->nodes[d->cn].Eh;
			//Eh_norm /= d->nodes[d->cn].n;
			
			if(ec->final_prediction < 0)
			{
				new_cn = d->nodes[d->cn].id_left;
			}
			else
			{
				new_cn = d->nodes[d->cn].id_right;
			}
			
			if(d->nodes[d->cn].node_pred.contain_sorted(mc->label))
			{
				if(d->nodes[new_cn].node_pred.contain_sorted(mc->label))
					d->nodes[d->cn].correct++;
				else
					d->nodes[d->cn].wrong++;
			}			
			
			if(new_cn != 0)
				d->cn = new_cn;
			
			update_example_indicies(all->audit, ec, d->increment * d->cn);	

			if(new_cn == 0)
				break;		
		}
		
		#ifdef TXM_DEBUG_PRED
		cout << "Prediction finished...\n";
		#endif
		
		update_example_indicies(all->audit, ec, -d->increment * d->cn);
		
		#ifdef TXM_DEBUG_PRED
		cout << "Nb of labels in leaf: " << d->nodes[d->cn].node_pred.size() << endl;
		#endif
		
		ec->final_prediction = d->nodes[d->cn].max_cnt2_label;		//necessary for the external evaluator to compute the average loss
		
		/*if(d->cn == 0)
		{
			ec->final_prediction = -1;
			cout << "Prediction error: " << err_cnt++ << endl;
 		}*/
		
		#ifdef TXM_DEBUG_PRED
		cout << "labels O:P:\t" << mc->label << ":" << ec->final_prediction << endl;
		#endif
		
		if(mc->label != ec->final_prediction)
		{
			wrong++;
		}
		else
			correct++;
			
		d->total_wrg = wrong;
		d->total_cor = correct;
		
		d->ex_num++;
	}	
	
	uint32_t predict_node(txm* d, example* ec, size_t level_limit)
	{
		vw* all = d->all;
		size_t level = 0;
		//float Eh_norm;
		
		if(command_example(all,ec))
		{
			return 0;
		}
		
		d->cn = 0;
		
		while(!d->nodes[d->cn].leaf && level < level_limit)
		{
			ec->test_only = true;
			d->base.learn(ec);
			
			#ifdef TXM_DEBUG_PRED
			cout << "level: " << level << endl;
			cout << "node: " << d->cn << endl;
			#endif
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			//Eh_norm = d->nodes[d->cn].Eh;
			//Eh_norm /= d->nodes[d->cn].n;
			
			if(ec->final_prediction < 0)
			{
				d->cn = d->nodes[d->cn].id_left;
			}
			else
			{
				d->cn = d->nodes[d->cn].id_right;
			}
			
			update_example_indicies(all->audit, ec, d->increment * d->cn);
			
			level++;
			
			if(d->cn == 0)
			{
				//cout << "Node prediction error!!\n";
				break;
			}
		}		
		
		update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
		return d->cn;
	}
	
	void learn(void* d, example* ec) 
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		txm* b = (txm*)d;
		uint32_t oryginal_label;
		float norm_Eh;
		float norm_Ehk;
		float left_or_right;
		size_t index = 0;		
		size_t id_left;
		size_t id_right;
		size_t id_parent;
		size_t id_left_right;
		float ftmp;
		int i;
		
		#ifdef TXM_DEBUG
		unsigned char* i;
		size_t j;
		#endif
		
		vw* all = b->all;	
		
		oryginal_label = mc->label;
		
		if(command_example(all,ec))
		{		
			b->base.learn(ec);
			
			if(ec->end_pass)
			{
				b->ex_total = b->ex_num;
				
				b->ex_num = 0;
			
				if(b->only_leafs)
				{
					b->tree_finished = true;	
				
					return;
				}
				else
				{
					b->only_leafs = true;
				}
				
				display_tree(b);	

				/*cout << "Root Weights: \n";

				for(size_t i = 0; i < 1000000; i++)
				{
					if(all->reg.weight_vector[i] != 0)
						cout << i << "\t" << all->reg.weight_vector[i] << endl;
				}

				cout << endl << endl;
				cin.ignore();	*/
				
				b->current_pass++;
				//display_node_stats(b);
				#ifdef TXM_DEBUG_PASS_STOP
				cin.ignore();
				#endif
			}	
			return;
		}		
		
		if(b->current_pass == 0)			//first pass through the data
		{
			b->cn = 0;						//root
			
			//add label to label list
			if(!b->nodes[b->cn].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index))	//if the label is not in the root
			{
				b->nodes[b->cn].node_pred.push_back_sorted(txm_node_pred_type(oryginal_label));	//add the label to the list of labels in the root
				b->nodes[b->cn].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index);
				b->nodes[b->cn].node_pred[index].label_cnt++;
				b->nodes[b->cn].node_pred[index].label_cnt2++;
			}
			else
			{
				b->nodes[b->cn].node_pred[index].label_cnt++;				
				b->nodes[b->cn].node_pred[index].label_cnt2++;
			}
		}
		else								//not the first pass through the data
		{
			b->cn = predict_node(b, ec, b->current_pass);
			
			if(b->nodes[b->cn].leaf || b->cn == 0)
			{
				b->ex_num++;
				return;
			}			
			
			if(b->nodes[b->cn].id_parent > 0)
			{
				id_parent = b->nodes[b->cn].id_parent;
				
				ftmp = b->nodes[id_parent].max_cnt2;
				ftmp /= b->nodes[id_parent].total_cnt2;
				
				if(ftmp > TXM_LEAF_TH)
				{
					b->nodes[b->cn].leaf = true;	
					b->nodes[id_parent].leaf = true;
					
					id_left = b->nodes[id_parent].id_left;
					id_right = b->nodes[id_parent].id_right;
					
					if(id_left)
						b->nodes[id_left].removed = true;
					
					if(id_right)
						b->nodes[id_right].removed = true;
						
					cout << "\n\nLEAF!!:\t" << id_parent << ":" << b->nodes[id_parent].max_cnt2_label << endl << endl;
				}				
			}
			
			if(b->nodes[b->cn].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index))
			{
				b->nodes[b->cn].node_pred[index].label_cnt2++;
				
				if(b->nodes[b->cn].node_pred[index].label_cnt2 > b->nodes[b->cn].max_cnt2)
				{
					b->nodes[b->cn].max_cnt2 = b->nodes[b->cn].node_pred[index].label_cnt2;
					b->nodes[b->cn].max_cnt2_label = b->nodes[b->cn].node_pred[index].label;
				}
			}
			else
			{
				b->nodes[b->cn].node_pred.push_back_sorted(txm_node_pred_type(oryginal_label));
				b->nodes[b->cn].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index);
				b->nodes[b->cn].node_pred[index].label_cnt2++;
			}
			
			b->nodes[b->cn].total_cnt2++;
			
			if(b->nodes[b->cn].level >= b->level_limit)
				return;
			
			b->only_leafs = false;
			
			update_example_indicies(all->audit, ec, b->increment * b->cn);
		}
		
		#ifdef TXM_DEBUG
		cout << "Current node: " << b->cn << endl;
		cout << "Example number: " << b->ex_num << endl;
		#endif		
			
		//do the initial prediction to decide if going left or right
		all->sd->min_label = -TXM_PRED_LIM;
		all->sd->max_label = TXM_PRED_LIM;	
		mc->label = 0;
		ec->test_only = true;
		b->base.learn(ec);		
		
		#ifdef TXM_DEBUG
		cout << "raw prediction: " << ec->final_prediction << endl;
		#endif		
		
		b->nodes[b->cn].Eh += ec->final_prediction;
		b->nodes[b->cn].n++;
		
		norm_Eh = b->nodes[b->cn].Eh / b->nodes[b->cn].n;
		
		b->nodes[b->cn].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index);
		b->nodes[b->cn].node_pred[index].Ehk += ec->final_prediction;
		b->nodes[b->cn].node_pred[index].nk++;
		
		norm_Ehk = b->nodes[b->cn].node_pred[index].Ehk / b->nodes[b->cn].node_pred[index].nk;
		
		left_or_right = norm_Ehk - TXM_PRED_ALFA * norm_Eh;
		
		#ifdef TXM_DEBUG
		cout << "norm Ehk:" << norm_Ehk << endl;
		cout << "norm Eh:" << norm_Eh << endl;
		cout << "left_or_right: " << left_or_right << endl;	
		#endif		
		
		id_left = b->nodes[b->cn].id_left;
		id_right = b->nodes[b->cn].id_right;		
		
		if(left_or_right < 0)
		{
			mc->label = -1.f;
			id_left_right = id_left;
			
			if(b->nodes[b->cn].id_left == 0)												//if the left child does not exist
			{
				id_left_right = id_left = b->nodes.size();									//node identifier = number-of_existing_nodes + 1
				b->nodes.push_back(init_node(id_left, b->cn, b->nodes[b->cn].level + 1));	//add new node to the tree
				b->nodes[b->cn].id_left = id_left;											//new node is the left child of the current node				
			}			
		}
		else
		{
			mc->label = 1.f;
			id_left_right = id_right;
			
			if(b->nodes[b->cn].id_right == 0)												//if the right child does not exist
			{
				id_left_right = id_right = b->nodes.size();									//node identifier = number-of_existing_nodes + 1
				b->nodes.push_back(init_node(id_right, b->cn, b->nodes[b->cn].level + 1));	//add new node to the tree
				b->nodes[b->cn].id_right = id_right;										//new node is the right child of the current node	
			}
		}	
			
		if(!b->nodes[id_left_right].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index))  //if the label does not exist in the left/right child of the current node
		{
			b->nodes[id_left_right].node_pred.push_back_sorted(txm_node_pred_type(oryginal_label));		//add the label to the left/right child of the current node		
			b->nodes[id_left_right].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index);
			b->nodes[id_left_right].node_pred[index].label_cnt++;
		}	
		else
		{
			b->nodes[id_left_right].node_pred[index].label_cnt++;
		}
		
		//learn!!
		ec->partial_prediction = 0;
		ec->final_prediction = 0;	
		all->sd->min_label = -TXM_PRED_LIM;
		all->sd->max_label = TXM_PRED_LIM;
		ec->test_only = false;
		b->base.learn(ec);	
		mc->label = oryginal_label;		
			
		#ifdef TXM_DEBUG		
		cout << "left:  \t";
		if(id_left > 0) for(j = 0; j < b->nodes[id_left].node_pred.size(); j++) cout << b->nodes[id_left].node_pred[j].label << ":" << b->nodes[id_left].node_pred[j].label_cnt <<"\t";
		
		cout << "right: \t";
		if(id_right > 0) for(j = 0; j < b->nodes[id_right].node_pred.size(); j++) cout << b->nodes[id_right].node_pred[j].label << ":" << b->nodes[id_right].node_pred[j].label_cnt << "\t";
		cout << endl;
		
		cout << "label: " << mc->label << endl;
		
		cout << "features: ";

		for (i = ec->indices.begin; (i + 1) < ec->indices.end; i++) 
		{
			feature* end = ec->atomics[*i].end;

			for (feature* f = ec->atomics[*i].begin; f!= end; f++) 
			{
				cout << f->x << "\t";         
			}
		}
		cout << endl;
		#endif			

		b->ex_num++;
		
		if(b->cn > 0)
			update_example_indicies(all->audit, ec, -b->increment * b->cn);

		#ifdef TXM_DEBUG
		cout << "Current Pass: " << b->current_pass << endl;
		cout << endl;
		#endif		
		
		#ifdef TXM_DEBUG_EX_STOP
		cin.ignore();
		#endif				
	}
	
	void drive(vw* all, void* d)
	{	
		uint32_t i, j, corsum = 0, wrgsum = 0;
		float tmp;
		txm* b = (txm*)d;	
		example* ec = NULL;	
		
		while ( !b->tree_finished )
		{
			if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
			{
				learn((txm*)d, ec);
				
				//if (!command_example(all, ec))
				//	OAA::output_example(*all, ec);

				VW::finish_example(*all, ec);				
			}
			else if (parser_done(all->p))
			{				
				return;
			}
		}		
		
		display_node_stats(b);
		//Evaluation
		b->ex_num = 0;
		ec = NULL;	
		while ( true )
		{
			if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
			{				
				predict((txm*)b, ec);
				
				if (!command_example(all, ec))
					OAA::output_example(*all, ec);
				
				VW::finish_example(*all, ec);				
			}
			else if (parser_done(all->p))
			{				
				cout<<"The number of training data points: "<< b->ex_total << endl;
				cout<<"The number of labels: "<< b->k << endl;
				cout<<"log2(the number of labels): "<< (float)log2(b->k) << endl;
				cout<<"Tree depth: "<< b->current_pass << endl;
				cout << endl;

				/*for(i = 0; i < b->nodes.size(); i++)
				{
					if(b->nodes[i].removed)
							continue;				
					
					if(b->nodes[i].node_pred.size() > 1)
					{
						tmp = b->nodes[i].wrong;
						tmp /= b->nodes[i].wrong + b->nodes[i].correct;
						cout << "Node:\t" << i << "\terr:\t" <<  tmp << "\tcor:\t" <<  b->nodes[i].correct << "\twrg:\t" << b->nodes[i].wrong << "\tEh:\t"<< b->nodes[i].Eh / b->nodes[i].n << endl; 
						corsum += b->nodes[i].correct;
						wrgsum += b->nodes[i].wrong;
					}
				}
				
				cout << "Total:\t\t\t\t\tcor:\t" << b->total_cor << "\twrg:\t" << b->total_wrg << endl;*/

				return;
			}
		}
	}

	void finish(void* data)
	{    
		txm* o=(txm*)data;
		o->base.finish();
		free(o);
	}

	learner setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
	{
		txm* data = (txm*)calloc(1, sizeof(txm));
		//first parse for number of actions
		if( vm_file.count("txm") ) 
		{
			data->k = (uint32_t)vm_file["txm"].as<size_t>();
			if( vm.count("txm") && (uint32_t)vm["txm"].as<size_t>() != data->k )
				std::cerr << "warning: you specified a different number of actions through --txm than the one loaded from predictor. Pursuing with loaded value of: " << data->k << endl;
		}
		else 
		{
			data->k = (uint32_t)vm["txm"].as<size_t>();

			//append txm with nb_actions to options_from_file so it is saved to regressor later
			std::stringstream ss;
			ss << " --txm " << data->k;
			all.options_from_file.append(ss.str());
		}		

		data->all = &all;
		*(all.p->lp) = OAA::mc_label_parser;
		
		data->increment = all.reg.stride * all.weights_per_problem;
		all.weights_per_problem *= 1 << (TXM_LEVEL_LIM + 1);
		data->base = all.l;
		learner l(data, drive, learn, finish, all.l.sl);
		
		txm* b = (txm*)data;
		
		b->current_pass = 0;
		b->level_limit = TXM_LEVEL_LIM;
		
		init_tree(b);
		
		return l;
	}
}
