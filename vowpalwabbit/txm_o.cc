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
	uint32_t ceil_log2(uint32_t k)					//computing log2(value)
	{
		uint32_t i = 0;
		
		while (k > (uint32_t)(1 << i))
			i++;
			
		return i;
	}
	
	class txm_o_node_pred_type					//for every node I have a table (v_array) of elements of this type with one entry per label
	{
		public:

	        uint32_t Rk;		
		double 		Ehk;					//conditional margin over the label
		uint32_t 	nk;					//total number of data points labeled as 'label' reaching the node
		uint32_t	label;					//label
		uint32_t	label_cnt2;				//the number of examples reaching the node assigned through partitioner of the parent 
									//(for root it is equal to the total number of examples)
	        uint32_t        example_counter;
		
	        bool operator==(txm_o_node_pred_type v){		//operator == (we will be sorting the table of elements of type txm_o_node_pred_type label-wise)
			return (label == v.label);
		}
		
		bool operator>(txm_o_node_pred_type v){			//operator >
			if(label > v.label) return true;		
			return false;
		}
		
		bool operator<(txm_o_node_pred_type v){			//operator <
			if(label < v.label) return true;		
			return false;
		}
		
		txm_o_node_pred_type(uint32_t l)			//constructor with label setting
		{
			label = l;
			Ehk = 0.f;
			nk = 0;
			label_cnt2 = 0;
			Rk = 0;	
		}
	};
	
	typedef struct							//structure describing tree node
	{
		size_t id_left;						//index of left child of a node
		size_t id_right;                                        //index of right child of a node
		size_t level;						//level on which a node is located in the tree (for the root, level = 0), not used in the algorithm, but useful for debugging
		size_t max_cnt2;					//maximal value of counter 2 in a node = number of data points with the most frequent label that reached a node
		size_t max_cnt2_label;					//label with maximal value of counter 2 in a node = most frequent label that reached a node
		int8_t initial_dir;					//direction where a node regressor sends the first example that reached a node, thus it is an initial direction 
									//(the node creates children (it always creates both children) when it receives and example that its regressor 
									//decides to send opposite direction to the initial direction)	
		
	        uint32_t ec_count;
	        uint32_t L;
	        uint32_t R;
	        float objective;

	  uint32_t myL;
	  uint32_t myR;
	        bool leaf;						//flag denoting that the node is a leaf (true if it is)
		v_array<txm_o_node_pred_type> node_pred;		//table of tree nodes

		double Eh;						//margin
		uint32_t n;						//total number of data points reaching the node
	} txm_o_node_type;
	
	struct txm_o							//structure txm_o
	{
		uint32_t k;						//number of classes
		vw* all;						//pointer to vw structure

		v_array<txm_o_node_type> nodes;				//our tree: table of tree nodes
		
		size_t max_depth;					//maximal tree depth
		size_t max_nodes;                                       //maximal number of nodes allowed in the tree - stopping criterion 
	};	

	txm_o_node_type init_node(size_t level)				//function initializing new node (level - level on which a node is located in the tree)
	{
		txm_o_node_type node;                                   //new node of type txm_o_node_type
		
		node.id_left = 0;
		node.id_right = 0;
		node.Eh = 0;
		node.n = 0;
		node.level = level;
		node.leaf = true;
		node.max_cnt2 = 0;
		node.max_cnt2_label = 0;
		node.initial_dir = 0;
		node.ec_count = 0;
		node.L = 0;
		node.R = 0;
		node.objective = 0;
		node.myL = 0;
		node.myR = 0;
                return node;
	}
	
	void init_tree(txm_o& d)					//tree initialization
	{
		d.nodes.push_back(init_node(0));			//adding a node (root) to a table of tree nodes
	}
		
	void train_node(txm_o& b, learner& base, example& ec, size_t& cn, size_t& index)		//function training node (executed in learn during tree training), ec is the current example,
	{											//cn is the node that is being trained (current node), index is the index of the label of example ec  
	  	uint32_t LR_ths = 500;
	
		label_data* simple_temp = (label_data*)ec.ld;					//variable of type label_data containing binary label of an example, used by base.learn() while training a node regressor 
														//do the initial prediction to decide if going left or right	
		simple_temp->label = FLT_MAX;				//if simple_temp.label is set to FLT_MAX, base.learn() is not training a regressor, but only perform testing

		base.predict(ec, cn);					//running base.learn()
			
		b.nodes[cn].Eh += (double)ec.final_prediction;		//incrementing margin 
		b.nodes[cn].n++;					//incrementing counter of examples reaching node cn
		
		float norm_Eh = b.nodes[cn].Eh / b.nodes[cn].n;	//computing expected margin
		
		b.nodes[cn].node_pred[index].nk++;			//incrementing counter of examples with the same label as ec reaching node cn
		
		size_t treshold_n = 0;//((ceil_log2(b.k) + 1) * b.k);	//threshold on the number of examples that must reach cn before it can create children = k(log2(k) + 1)
		
		//float treshold = 1.0;					//threshold on the increment value for conditional margin over the example label
		//if(b.nodes[cn].n <= 5) treshold = 0.95;		//if ec is among first 5 example reaching ec, conditional margin is not allowed to be incremented by more than treshold
		
		//if(ec.final_prediction > treshold)			//execution of thresholding, where thresholding helps to avoid the following scenario:
		//	b.nodes[cn].node_pred[index].Ehk += treshold;  //Eh is always in (-1,1) since first prediction is 0 (weights are initialized to 0)
		//else if(ec.final_prediction < -treshold)		//if one Ehk has value -1 and the others have value 1, then Eh will never beat Ehk's that have value 1 and balancing doe not work
		//	b.nodes[cn].node_pred[index].Ehk -= treshold;
		//else
			b.nodes[cn].node_pred[index].Ehk += (double)ec.final_prediction;	
		
		float norm_Ehk = b.nodes[cn].node_pred[index].Ehk / b.nodes[cn].node_pred[index].nk;		//computing expected conditional margin over the example label
		//	if(cn == 0)
		//{
		    b.nodes[cn].objective = 0;
		    float tmp1, tmp2, tmp3;
                    //cout << "d" << endl;
		    for(size_t i = 0; i < b.nodes[cn].node_pred.size(); i++)
		      {
			tmp1 = (float)b.nodes[cn].R / (float)b.nodes[cn].n;
		        tmp2 = (float)b.nodes[cn].node_pred[i].Rk / (float)b.nodes[cn].node_pred[i].nk;
			tmp3 = (float)b.nodes[cn].node_pred[i].nk / (float)b.nodes[cn].n;
			//cout << "a" << endl;
			//cout << tmp1 << endl;
			//cout << tmp2 << endl;
			//cout << tmp3 << endl;
			b.nodes[cn].objective += tmp3 * fabs(tmp1 - tmp2);
			//cout << endl;
			//cout << "Class:\t\t" << i << "\t\tnorm_Eh (the best: 0):\t\t" << norm_Eh;
			//cout << endl;
		      }
		    //b.nodes[cn].objective /= b.k;
		    //cout << "objective:\t:" << b.nodes[cn].objective << endl;
		    // getchar();
		    //}

		float left_or_right = norm_Ehk - norm_Eh;// + 0.01*(log(b.nodes[cn].L) - log(b.nodes[cn].R));		//computing difference of expected conditional margin over the example label and expected margin
		
		size_t id_left = b.nodes[cn].id_left;			//index of left child of cn
		size_t id_right = b.nodes[cn].id_right;		//index of right child of cn
	
		size_t id_left_right;		
		//cout << left_or_right << " " << log(b.nodes[cn].L) << " " << log(b.nodes[cn].R) << endl;
		if(left_or_right < 0)					//if the difference of expectations < 0, train regressor with (example,-1)
		{
			simple_temp->label = -1.f;			//binary label for an example is set to -1
			id_left_right = id_left;                        //index of left child of cn
			b.nodes[cn].myL++;
		}
		else							//if the difference of expectations >= 0, train regressor with (example,1)
		{
			simple_temp->label = 1.f;			//binary label for an example is set to 1
			id_left_right = id_right;			//index of right child of cn
			b.nodes[cn].myR++;
		}
		
		//if(cn == 0)
		//{
		//	cout << "label: " << b.nodes[cn].node_pred[index].label << "\t\t" << "diff of exp: " << left_or_right << "\t\t" << "where we go: " << ec.final_prediction - norm_Eh << endl; 
		//}	
		
		//if(b.nodes[cn].initial_dir == 0)					//if ec is the first example reaching cn
		//{
		//	b.nodes[cn].initial_dir = (int8_t)simple_temp->label;		//set the initial direction to the direction where regressor (before training) in cn wants to send an example
		//}
		//else



		//I create children when there are log(k) points that want to go left and log(k) points that want to go right in the node  
		if((b.nodes[cn].myR > ceil_log2(b.k)) && (b.nodes[cn].myL > ceil_log2(b.k)))
		  {
		if(id_left_right == 0)						//if the node does not have children
		{	//condition checking whether cn can create children:		1) the number of examples that already reached cn > treshold_n, 
			//								2) regressor in node cn wants to send ec opposite to initial direction
			//								3) total number of nodes in the tree + 2 potential children is smaller than the maximal allowed number of nodes 
			//if the condition holds, we create two children of cn
		
		  if(b.nodes.size() + 2 <= b.max_nodes)// && b.nodes[cn].myL > LR_ths && b.nodes[cn].myR > LR_ths)
			{
			        id_left_right = b.nodes.size();				//index of one of new children of cn = number_of_existing_nodes
				b.nodes.push_back(init_node(b.nodes[cn].level + 1));		//add new node (the first child) to the tree on the level = level of cn + 1
				b.nodes.push_back(init_node(b.nodes[cn].level + 1));		//add new node (the second child) to the tree on the level = level of cn + 1
				b.nodes[cn].id_left = id_left_right;				//index of the first child of cn
				b.nodes[cn].id_right = id_left_right + 1;			//index of the second child of cn
				
				b.nodes[b.nodes[cn].id_left].max_cnt2_label = b.nodes[cn].max_cnt2_label;	//new child has the label of a parent assigned to it (it is a safety operation in 
				b.nodes[b.nodes[cn].id_right].max_cnt2_label = b.nodes[cn].max_cnt2_label;	//case later on no example will be directed to this node (happens very rarely))
				
				if(b.nodes[cn].level + 1 > b.max_depth)			//update the value of the tree depth
				{
					b.max_depth = b.nodes[cn].level + 1;	
				}	
			}
		}	
		  }
		base.learn(ec, cn);				//train regressor of node cn
		
		if(b.nodes[cn].id_left == 0)			//cn did not create children
			b.nodes[cn].leaf = true;		//cn is a leaf
		else	
			b.nodes[cn].leaf = false;		//cn is not a leaf
	}

	void predict(txm_o& b, learner& base, example& ec)		//function building the tree, ec is the current example
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec.ld;
				
		label_data simple_temp;				//variable of type label_data containing binary label of an example, used by base.learn() while training a node regressor 
		simple_temp.initial = 0.0;			//???????????
		simple_temp.weight = mc->weight;		//weight of an example ???????????
		ec.ld = &simple_temp;											//label data for an example is set to simple_temp containing binary label (in this case FLT_MAX)								
		size_t cn = 0;					//current node cn is the root (cn = 0)
		while(1)
		{
			if(b.nodes[cn].leaf)											//if cn is a leaf
			{					
			        ec.final_prediction = b.nodes[cn].max_cnt2_label;						//assign the most frequent label of a leaf to an example
				ec.ld = mc;												//we assign original label data to an example			
				break;												
			}
																//do the prediction to decide if going left or right using trained regressor in cn	
			simple_temp.label = FLT_MAX;										//if simple_temp.label is set to FLT_MAX, base.learn() is not training a regressor, but only perform testing
			base.predict(ec, cn);											//running base.learn()
			
			if(ec.final_prediction < b.nodes[cn].Eh/b.nodes[cn].n)										//if the regressor's prediction is < 0
				cn = b.nodes[cn].id_left;									//set cn to the left child of cn
			else													//if the regressor's prediction is >= 0
				cn = b.nodes[cn].id_right;									//set cn to the right child of cn
		}	
	}

  //-----------------------------------------------
  //DEBUGGING
	
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
					  cout << "[" << i << "," << d.nodes[i].max_cnt2_label << "," << d.nodes[i].max_cnt2 << "," << d.nodes[i].ec_count << "," << d.nodes[i].objective << "] ";
					else
					  cout << "(" << i << "," << d.nodes[i].max_cnt2_label << "," << d.nodes[i].max_cnt2 << "," << d.nodes[i].ec_count << "," << d.nodes[i].objective << ") ";
				}
			}
			cout << endl;
		}
		cout << endl;
		
		cout << endl;
		cout << "Tree depth: " << d.max_depth << endl;
		cout << "ceil of log2(k): " << ceil_log2(d.k) << endl;
	}
	
	void save_node_stats(txm_o& d)							//wyswietlam statystyke dla poszczegonym nodow
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
				
			fprintf(fp, "max(lab:cnt:tot):, %3d,%6d,%7d,\n",  (int) b->nodes[i].max_cnt2_label, (int) b->nodes[i].max_cnt2, (int) total);
			fprintf(fp, "left: %4d, right: %4d",  (int) b->nodes[i].id_left, (int) b->nodes[i].id_right);
			fprintf(fp, "\n\n");
		}
		
		fclose(fp);
	}	
  //-----------------------------------------------

	void learn(txm_o& b, learner& base, example& ec)		//function building the tree, ec is the current example
	{
	  static size_t ex_num = 0, a=0, bb=0, c=0, d=0;
	        predict(b,base,ec);
		ex_num++;
		OAA::mc_label *mc = (OAA::mc_label*)ec.ld;
				
		if(b.all->training && (mc->label !=  (uint32_t)-1) && !ec.test_only)					//if training the tree
	        {
		  size_t index = 0;				//only variable declaration
		
		  label_data simple_temp;				//variable of type label_data containing binary label of an example, used by base.learn() while training a node regressor 
		  simple_temp.initial = 0.0;			//???????????
		  simple_temp.weight = mc->weight;		//weight of an example ???????????
		  ec.ld = &simple_temp;											//label data for an example is set to simple_temp containing binary label (in this case FLT_MAX)								
		  uint32_t oryginal_label = mc->label;		//oryginal label of ec		                		

		  if (oryginal_label == 1) a++;
                  if (oryginal_label == 2) bb++;
                  if (oryginal_label == 3) c++;
                  if (oryginal_label == 4) d++;
		  //if(ex_num % 100 == 1)cout << ex_num << "\t" << a << "\t" << bb <<"\t" << c <<"\t" << d << endl;

		  size_t tmp_final_prediction = ec.final_prediction;			//label that the tree assigns to ec
		  size_t cn = 0;					//current node cn is the root (cn = 0)
		  while(1)
		    {
                      b.nodes[cn].ec_count++;
		      index = b.nodes[cn].node_pred.unique_add_sorted(txm_o_node_pred_type(oryginal_label));		//if the label of ec is not in cn, add it to table node_pred in cn, return an index in the tabel, where it was added (or if it was there, return the index where it was) 
					
		      b.nodes[cn].node_pred[index].label_cnt2++;							//increase the counter of the label of ec in table node_pred
					
		      if(b.nodes[cn].node_pred[index].label_cnt2 > b.nodes[cn].max_cnt2)				//update the most frequent label (and a correponding counter) for cn
			{
			    b.nodes[cn].max_cnt2 = b.nodes[cn].node_pred[index].label_cnt2;
			    b.nodes[cn].max_cnt2_label = b.nodes[cn].node_pred[index].label;
			}
				
		      train_node(b, base, ec, cn, index);								//train cn
		      
		      if(b.nodes[cn].leaf)											//if cn is a leaf
			{					
			  ec.final_prediction = tmp_final_prediction;						//assign the most frequent label of a leaf to an example
			  ec.ld = mc;												//we assign original label data to an example			
			
			  break;												
			}
		      //do the prediction to decide if going left or right using trained regressor in cn	
		      simple_temp.label = FLT_MAX;										//if simple_temp.label is set to FLT_MAX, base.learn() is not training a regressor, but only perform testing
		      base.predict(ec, cn);											//running base.learn()
		
		      //if(cn == 0)
			//{
			//  cout << ec.final_prediction - b.nodes[cn].Eh/b.nodes[cn].n << endl; 
			//}	
		      if(ec.final_prediction < b.nodes[cn].Eh/b.nodes[cn].n)
			{ 
			  b.nodes[cn].L++;
										//if the regressor's prediction is < 0
			  cn = b.nodes[cn].id_left;									//set cn to the left child of cn
			}		      
		      else													//if the regressor's prediction is >= 0
			{
			b.nodes[cn].R++;
			//	if(cn == 0)
			// {
			    //cout<< b.nodes[cn].R << endl;
			    //getchar();
			    //}
			b.nodes[cn].node_pred[index].Rk++;
			cn = b.nodes[cn].id_right;									//set cn to the right child of cn
			  }
		    }	
		}
	}
	
	void finish(txm_o& b)
	{
	  display_tree2(b);  
	  save_node_stats(b);  
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
			  //	cout << "Tree depth: " << b.max_depth << endl;
			  //	cout << "ceil of log2(k): " << ceil_log2(b.k) << endl;
				
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
		data->max_nodes = (2 << i) - 1;										//max number of nodes =2^(i+1)-1 = 2k-1 (root (1 node) - depth 0, 2^i (leaf level) +2^(i-1)+2^(i-2)+...+2^0 (root level) = 2^(i+1)-1 - depth log2(k) = i)
		
		learner* l = new learner(data, all.l, data->max_nodes + 1);					//initialize learner (last input: number of regressors that the tree will use)
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
