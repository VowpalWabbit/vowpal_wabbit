/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

#include "txm_o.h"
#include "simple_label.h"
#include "cache.h"
#include "v_hashmap.h"
#include "vw.h"
#include "oaa.h"

using namespace std;

namespace TXM_O 
{
	class txm_o_node_pred_type
	{
		public:
		
		float 		Ehk;
		uint32_t 	nk;
		uint32_t	label;
		uint32_t	label_cnt;
		float		direction;
		
		void operator=(txm_o_node_pred_type v)
		{
			Ehk = v.Ehk;
			nk = v.nk;
			label = v.label;
			label_cnt = v.label_cnt;
			direction = v.direction;
		}
		
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
			nk = 0;
			label_cnt = 1;
			direction = 0;
		}
		
		txm_o_node_pred_type()
		{
			label = 0;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			direction = 0;
		}
	};
	
	typedef struct
	{
		size_t id;
		size_t id_left;
		size_t id_right;
		size_t id_parent;
		size_t level;		
		
		v_array<txm_o_node_pred_type> node_pred;		
		
		float Eh;
		uint32_t n;		
	} txm_o_node_type;
	
	struct txm_o
	{
		uint32_t k;
		uint32_t increment;
		learner base;
		vw* all;
		
		v_array<txm_o_node_type> nodes;	//the nodes
		
		size_t id_root;
		size_t cn;						//current node
		size_t id_removed1;
		size_t id_removed2;
		size_t ex_num;					//index of current example
		size_t ex_total;
		
		size_t current_pass;			//index of current pass through the data		
	};	

	txm_o_node_type init_node(size_t id, size_t id_parent, size_t level)
	{
		txm_o_node_type node;
		
		node.id = id;
		node.id_parent = id_parent;
		node.id_left = 0;
		node.id_right = 0;
		node.Eh = 0;
		node.n = 0;
		node.level = level;
		
		return node;
	}
	
	void clear_node(txm_o_node_type* node)
	{
		node->id_left = 0;
		node->id_right = 0;
		node->Eh = 0;
		node->n = 0;
		
		node->node_pred.delete_v();
	}
	
	void init_tree(txm_o* d)
	{
		d->cn = 0;
		d->id_root = 0;
		d->ex_num = 0;
		d->id_removed1 = 0;
		d->id_removed2 = 0;
		d->nodes.push_back(init_node(0, 0, 0));
	}
	
	void display_tree(txm_o* d)
	{
		size_t i;
		size_t j;
		size_t level = 0;
		size_t nodes_on_level;
		
		do
		{
			nodes_on_level = 0;
			
			for(j = 0; j < d->nodes.size(); j++)
			{
				if(d->nodes[j].level == level)
				{
					nodes_on_level++;
					cout << d->nodes[j].id << ":(";
					
					for(i = 0; i < d->nodes[j].node_pred.size(); i++)
					{
						cout << d->nodes[j].node_pred[i].label;
						
						if(i < d->nodes[j].node_pred.size() - 1)
							cout << ",";
					}
					
					cout << ") ";	
				}		
			}
			
			level++;
			cout << endl;	
		}while(nodes_on_level > 0);
		
		cout << endl;	
		cout << endl;		
	}

	void predict(txm_o* d, example* ec)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		vw* all = d->all;
		
		#ifdef TXM_O_DEBUG_PRED
		int level = 0;
		#endif
		
		if(command_example(all,ec))
		{
			return;
		}
		
		d->cn = 0;
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "\nExample: " << d->ex_num << endl;
		#endif
		
		while(d->nodes[d->cn].node_pred.size() > 1)
		{
			ec->test_only = true;
			d->base.learn(ec);
			
			#ifdef TXM_O_DEBUG_PRED
			cout << "level: " << level++ << endl;
			cout << "node: " << d->cn << endl;
			#endif
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			if(ec->final_prediction < 0)
			{
				d->cn = d->nodes[d->cn].id_left;
			}
			else
			{
				d->cn = d->nodes[d->cn].id_right;
			}
			
			update_example_indicies(all->audit, ec, d->increment * d->cn);
			
			if(d->cn == 0)
				break;
		}
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "Prediction finished...\n";
		#endif
		
		update_example_indicies(all->audit, ec, -d->increment * d->cn);
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "Nb of labels in leaf: " << d->nodes[d->cn].node_pred.size() << endl;
		#endif
		
		if(d->nodes[d->cn].node_pred.size() == 1 && d->cn > 0)
			ec->final_prediction = d->nodes[d->cn].node_pred[0].label;		//necessary for the external evaluator to compute the average loss
		else
			ec->final_prediction = -1;	
		
		d->ex_num++;
	}
	
	void learn(void* d, example* ec) 
	{
		unsigned char* i;
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		txm_o* b = (txm_o*)d;
		uint32_t oryginal_label;
		float norm_Eh;
		float norm_Ehk;
		float left_or_right;
		size_t index = 0;
		size_t j;
		size_t id_left;
		size_t id_right;
		size_t id_left_right;
		size_t id_other;
		size_t id_tmp;
		bool creating_childs_flag;		
		
		vw* all = b->all;	
		
		oryginal_label = mc->label;
		
		if(command_example(all,ec))
		{		
			b->base.learn(ec);
			
			if(ec->end_pass)
			{
				b->ex_total = b->ex_num;
				
				b->ex_num = 0;			
			}	
			return;
		}		
		
		b->cn = b->id_root;						//root
		
		while(true)
		{		
			update_example_indicies(all->audit, ec, b->increment * b->cn);
			
			if(b->cn == b->id_root)			//root
			{
				//add label to label list
				if(!b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))		//if the label is not in the root
				{
					b->nodes[b->cn].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));	//add the label to the list of labels in the root
					
					if(b->nodes[b->cn].node_pred.size() == 1)
						b->nodes[b->cn].node_pred[0].direction = -1.f;
					else if(b->nodes[b->cn].node_pred.size() == 2)
					{
						b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index);
						b->nodes[b->cn].node_pred[index].direction = 1.f;
					}
				}	
				else
				{
					b->nodes[b->cn].node_pred[index].label_cnt++;
				}
			}	
			else if(b->nodes[b->cn].node_pred.size() == 1)					//leaf level of the tree - stopping criterion
			{
				#ifdef TXM_O_DEBUG
				cout << "Current node: " << b->cn << ":" <<  b->nodes[b->cn].id_left << ":" << b->nodes[b->cn].id_right << endl;
				cout << "Example number: " << b->ex_num << endl;
				cout << "Leaf!!\n";
				
				cout << "all labels: \t";
				for(j = 0; j < b->nodes[b->cn].node_pred.size(); j++) cout << b->nodes[b->cn].node_pred[j].label << ":" << b->nodes[b->cn].node_pred[j].label_cnt << "\t";
				cout << endl;
				#endif
				
				b->ex_num++;
				
				#ifdef TXM_O_DEBUG
				cout << "\n";
				#endif
				break;
			}
			
			#ifdef TXM_O_DEBUG
			cout << "Current node: " << b->cn << ":" <<  b->nodes[b->cn].id_left << ":" << b->nodes[b->cn].id_right << endl;
			cout << "Example number: " << b->ex_num << endl;
			#endif		
			
			//do the initial prediction to decide if going left or right
			all->sd->min_label = -TXM_O_PRED_LIM;
			all->sd->max_label = TXM_O_PRED_LIM;		
			ec->test_only = true;
			b->base.learn(ec);	
			
			#ifdef TXM_O_DEBUG
			cout << "raw prediction: " << ec->final_prediction << endl;
			#endif		
			
			b->nodes[b->cn].Eh += ec->final_prediction;
			b->nodes[b->cn].n++;
			
			norm_Eh = b->nodes[b->cn].Eh / b->nodes[b->cn].n;
			
			b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index);
			b->nodes[b->cn].node_pred[index].Ehk += ec->final_prediction;
			b->nodes[b->cn].node_pred[index].nk++;
			
			norm_Ehk = b->nodes[b->cn].node_pred[index].Ehk / b->nodes[b->cn].node_pred[index].nk;
			
			left_or_right = norm_Ehk - TXM_O_PRED_ALFA * norm_Eh;
			
			#ifdef TXM_O_DEBUG
			cout << "norm Ehk:" << norm_Ehk << endl;
			cout << "norm Eh:" << norm_Eh << endl;
			cout << "left_or_right: " << left_or_right << endl;	
			#endif	
			
			if(b->nodes[b->cn].node_pred.size() == 1)			
				mc->label = -1.f;
			else if(b->nodes[b->cn].node_pred.size() == 2)
			{
				b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index);
				mc->label = b->nodes[b->cn].node_pred[index].direction;
				left_or_right = mc->label;
			}
			else
			{
				if(left_or_right < 0)
					mc->label = -1.f;
				else
					mc->label = 1.f;
			}	
			
			//learn!!
			ec->partial_prediction = 0;
			ec->final_prediction = 0;	
			all->sd->min_label = -TXM_O_PRED_LIM;
			all->sd->max_label = TXM_O_PRED_LIM;
			ec->test_only = false;
			b->base.learn(ec);	
			mc->label = oryginal_label;		

			if(b->nodes[b->cn].node_pred.size() == 1)					//root is a leaf of the tree - stopping criterion
			{
				#ifdef TXM_O_DEBUG
				cout << "Leaf!!\n";
				
				cout << "all labels: \t";
				for(j = 0; j < b->nodes[b->cn].node_pred.size(); j++) cout << b->nodes[b->cn].node_pred[j].label << ":" << b->nodes[b->cn].node_pred[j].label_cnt << "\t";
				cout << endl;
				#endif
				
				b->ex_num++;
				
				#ifdef TXM_O_DEBUG
				cout << "\n";
				#endif
				break;
			}	
			
			id_left = b->nodes[b->cn].id_left;
			id_right = b->nodes[b->cn].id_right;
			
			if(left_or_right < 0)
			{
				id_left_right = id_left;
				id_other = id_right;				
			}
			else
			{
				id_left_right = id_right;
				id_other = id_left;		
			}		
			
			if(id_left != 0 && id_right != 0)
			{			
				//removing the label from the other child 
				
				if(b->nodes[id_other].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index)) 
				{
					cout << "\nDeleting label... \n";
					
					cout << id_other << "\t" << oryginal_label << "\t" << index << endl;
				
					b->id_removed1 = id_other;
					b->nodes[b->id_removed1].node_pred.remove_sorted(index);				
					
					while(b->nodes[b->id_removed1].id_left != 0 && b->nodes[b->id_removed1].id_right != 0)
					{
						if(b->nodes[b->nodes[b->id_removed1].id_left].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))
						{
							cout << b->nodes[b->id_removed1].id_left << "\t" << oryginal_label << "\t" << index << endl;
							
							b->id_removed1 = b->nodes[b->id_removed1].id_left;
							b->nodes[b->id_removed1].node_pred.remove_sorted(index);
						}
						else if(b->nodes[b->nodes[b->id_removed1].id_right].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))
						{
							cout << b->nodes[b->id_removed1].id_left << "\t" << oryginal_label << "\t" << index << endl;
							
							b->id_removed1 = b->nodes[b->id_removed1].id_right;							
							b->nodes[b->id_removed1].node_pred.remove_sorted(index);
						}										
					}
					
					b->id_removed2 = b->nodes[b->id_removed1].id_parent;
					
					if(b->nodes[b->id_removed2].id_left == b->id_removed1)
						id_tmp = b->nodes[b->id_removed2].id_right;
					else
						id_tmp = b->nodes[b->id_removed2].id_left;
						
					if(b->id_removed2 == 0)
						b->id_root = id_tmp;
					else
					{
						if(b->nodes[b->nodes[b->id_removed2].id_parent].id_left == b->id_removed2)
							b->nodes[b->nodes[b->id_removed2].id_parent].id_left = id_tmp;
						else
							b->nodes[b->nodes[b->id_removed2].id_parent].id_right = id_tmp;
					}					
					
					clear_node(&b->nodes[b->id_removed1]);
					clear_node(&b->nodes[b->id_removed2]);	

					b->nodes[id_tmp].level--;
					
					if(b->id_removed2 == b->cn)
						b->cn = id_tmp;

					cout << "\nDeleting label finished!!\n\n";
				}			
			}
			
			if(b->nodes[b->cn].id_left == 0 && b->nodes[b->cn].id_right == 0)									//if childs do not exist
			{
				if(b->id_removed1 > 0)
				{
					b->nodes[b->cn].id_left = b->id_removed1;
					b->nodes[b->id_removed1].id_parent = b->cn;
					b->nodes[b->id_removed1].level = b->nodes[b->cn].level + 1;
					b->id_removed1 = 0;
					
					b->nodes[b->cn].id_right = b->id_removed2;
					b->nodes[b->id_removed2].id_parent = b->cn;
					b->nodes[b->id_removed2].level = b->nodes[b->cn].level + 1;
					b->id_removed2 = 0;
				}
				else
				{
					b->nodes[b->cn].id_left = b->nodes.size();										//node identifier = number-of_existing_nodes + 1
					b->nodes.push_back(init_node(b->nodes[b->cn].id_left, b->cn, b->nodes[b->cn].level + 1));					//add new node to the tree
					b->nodes[b->cn].id_right = b->nodes.size();										//node identifier = number-of_existing_nodes + 1
					b->nodes.push_back(init_node(b->nodes[b->cn].id_right, b->cn, b->nodes[b->cn].level + 1));					//add new node to the tree
				}				
				
				creating_childs_flag = true;
			}	
			else
			{
				creating_childs_flag = false;
			}
							
			id_left = b->nodes[b->cn].id_left;
			id_right = b->nodes[b->cn].id_right;
			
			if(left_or_right < 0)
			{
				id_left_right = id_left;
				id_other = id_right;				
			}
			else
			{
				id_left_right = id_right;
				id_other = id_left;		
			}			
				
			if(!b->nodes[id_left_right].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))  //if the label does not exist in the left/right child of the current node
				b->nodes[id_left_right].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));		//add the label to the left/right child of the current node			
			else
				b->nodes[id_left_right].node_pred[index].label_cnt++;
			
			
			if(creating_childs_flag)
			{
				if(b->nodes[b->cn].node_pred[0].label == oryginal_label)
					b->nodes[id_other].node_pred.push_back_sorted(b->nodes[b->cn].node_pred[1]);
				else
					b->nodes[id_other].node_pred.push_back_sorted(b->nodes[b->cn].node_pred[0]);				
			}			
					
			
			#ifdef TXM_O_DEBUG		
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
			cout << endl << endl;
			#endif			
			
			update_example_indicies(all->audit, ec, -b->increment * b->cn);

			b->cn = id_left_right;
		}
		
		b->ex_num++;
		
		#ifdef TXM_O_DEBUG
		display_tree(b);
		#endif
		
		#ifdef TXM_O_DEBUG_EX_STOP
		cin.ignore();
		#endif
	}
	
	void drive(vw* all, void* d)
	{	
		txm_o* b = (txm_o*)d;	
		example* ec = NULL;	
		
		while ( true )
		{
			if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
			{
				learn((txm_o*)d, ec);
				
				//if (!command_example(all, ec))
				//	OAA::output_example(*all, ec);

				VW::finish_example(*all, ec);				
			}
			else if (parser_done(all->p))
			{				
				return;
			}
		}		
		
		//Evaluation
		b->ex_num = 0;
		ec = NULL;	
		while ( true )
		{
			if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
			{				
				predict((txm_o*)b, ec);
				
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

				return;
			}
		}
	}

	void finish(void* data)
	{    
		txm_o* o=(txm_o*)data;
		o->base.finish();
		free(o);
	}

	learner setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
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
		data->increment = all.reg.stride * all.weights_per_problem;
		all.weights_per_problem *= data->k;
		data->base = all.l;
		learner l(data, drive, learn, finish, all.l.sl);
		
		txm_o* b = (txm_o*)data;
		
		b->current_pass = 0;
		
		init_tree(b);
		
		return l;
	}
}
