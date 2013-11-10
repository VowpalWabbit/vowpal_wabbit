/*\t

Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.node
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
	class txm_o_node_labels_type
	{
		public:
		
		uint32_t	label;
		uint32_t	label_cnt2;
		
		void operator=(txm_o_node_labels_type v)
		{
			label = v.label;
			label_cnt2 = v.label_cnt2;
		}
		
		bool operator==(txm_o_node_labels_type v){
			return (label_cnt2 == v.label_cnt2);
		}
		
		bool operator>(txm_o_node_labels_type v){
			if(label_cnt2 > v.label_cnt2) return true;		
			return false;
		}
		
		bool operator<(txm_o_node_labels_type v){
			if(label_cnt2 < v.label_cnt2) return true;		
			return false;
		}
		
		txm_o_node_labels_type(uint32_t l, uint32_t label_cnt2)					
		{
			label = l;
			label_cnt2 = label_cnt2;
		}
		
		txm_o_node_labels_type()							
		{
			label = 0;
			label_cnt2 = 0;
		}		
	};
	
	class txm_o_node_pred_type								//w kazdym nodzie mam jedna tablice elementow tego typu, kazdej wejscie w tej tablicy odpowiada laelowi ktory wpadl do tego noda podczas trenowania lub predykcji (odpalanie funkcji predict_node)
	{
		public:
		
		float 		Ehk;
		uint32_t 	nk;
		uint32_t	label;									//dla kazdego labela mam Ehk, nk, tu jest suma labeli ktore trafily do noda czy w czasie trenowania czy w czasie predykcji
		uint32_t	label_cnt;								//ilosc przykladow o tym labelu w tym nodzie przypisanych przez expected
		uint32_t	label_cnt2;								//ilosc przykladow o tym labelu w tym nodzie przypisanych przez predictor
		bool 		assigned;
		
		void operator=(txm_o_node_pred_type v)
		{
			Ehk = v.Ehk;
			nk = v.nk;
			label = v.label;
			label_cnt = v.label_cnt;
			label_cnt2 = v.label_cnt2;
			assigned = v.assigned;
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
		
		txm_o_node_pred_type(uint32_t l)						//konstruktor z ustawianiem labela
		{
			label = l;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			label_cnt2 = 0;	
			assigned = false;
		}
		
		txm_o_node_pred_type()								//konstruktor bez ustawiania labela
		{
			label = 0;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			label_cnt2 = 0;		
			assigned = false;
		}
	};
	
	typedef struct                                         //struktura opisujaca wezel w drzewie
	{
		size_t id;                                         //root = 0
		size_t id_left;
		size_t id_right;
		size_t id_parent;                                  //root = 0
		size_t level;                                      //root = 0
		bool leaf;                                         //flaga - mamy lisc - ustawiane na podstawie prediction statistics
		bool removed;                                      //node zostal usuniety, detekuje ze cos jest leafem jak juz mam stworzone jego dzieci i po prostu nie rozwijam tych dzieci dalej a ich samych nie biore pod uwage
		size_t max_cnt2;                                   //maxymalna wartosc countera 2 w nodzie
		size_t max_cnt2_label;                             //label z maxymalnym counterem w nodzie
		size_t total_cnt2;                                 //laczna ilosc punktow nodzie prxzypisanych przez predyktor do noda
		
		v_array<txm_o_node_pred_type> node_pred;	
		v_array<txm_o_node_labels_type> node_labels;

		float Eh;
		uint32_t n;		
	} txm_o_node_type;
	
	struct txm_o
	{
		uint32_t k;
		uint32_t increment;
		learner base;
		vw* all;
		
		v_array<size_t> ex_node;		//pozostalosc po starym kodzie - nie korzystam teraz
		v_array<txm_o_node_type> nodes;	//the nodes - czyli nasze drzewo
		
		size_t cn;						//current node
		size_t cl;						//current level
		size_t ex_num;					//index of current example
		size_t ex_total;
		
		size_t current_pass;			//index of current pass through the data	
		size_t level_limit;             //maxymlane glebokosc drzewa (liscie maja byc na tym levelu, czyli 1 odpowiada rootowi i dwom lisciom)
		
		bool only_leafs;                //only leafs jest wystaione na true gdy mam same liscie (czyste liscie lub drzewo maxymalnej glebokosci i wtedy wszystkie nody na tej glebokosci sa liscmi)
		bool tree_finished;             //zakonczenie drzewa
		
		uint32_t total_wrg;             //w predictie liczone, sluzone do liczenia tego samego bledu ktory liczy VW
		uint32_t total_cor;             //w predictie liczone, sluzone do liczenia tego samego bledu ktory liczy VW
		
		#ifdef TXM_O_DEBUG_FILE1
		FILE *debug1_fp;
		#endif
		
		#ifdef TXM_O_DEBUG_FILE2
		FILE *debug2_fp;
		#endif
		
		#ifdef TXM_O_ARBITRARY_ROOT
		FILE *arbitrary_fp;
		v_array<uint32_t> arbitrary;
		#endif
	};	

	txm_o_node_type init_node(size_t id, size_t id_parent, size_t level)        //inicjalizowanie nowego noda drzewa, odpalane przy tworzeniu nowego noda
	{
		txm_o_node_type node;
		
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
		
		return node;
	}
	
	void init_tree(txm_o* d)                               //inicjalizacja drzewa
	{
		d->cn = 0;
		d->ex_num = 0;
		d->nodes.push_back(init_node(0, 0, 0));
		d->tree_finished = false;
		d->only_leafs = false;
		
		#ifdef TXM_O_DEBUG_FILE1
		d->debug1_fp = fopen("atxm_o_debug1.csv", "wt");
		#endif		
	}
	
	#ifdef TXM_O_DEBUG_FILE1
	void save_node_stats(txm_o* b)							//wyswietlam statystyke dla poszczegonym nodow
	{
		uint32_t i, j;
		#ifdef TXM_O_ARBITRARY_ROOT
		uint32_t total = 0;
		uint32_t err = 0;
		float ftmp;
		#endif
				
		for(i = 0; i < b->nodes.size(); i++)
		{
			if(b->nodes[i].removed)
				continue;
			
			fprintf(b->debug1_fp, "Node: %4d, Level: %2d, Leaf: %1d, Eh: %7.4f,\n", (int) i, (int) b->nodes[i].level, (int) b->nodes[i].leaf, b->nodes[i].Eh / b->nodes[i].n);
			
			fprintf(b->debug1_fp, "Label:, ");
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{
				fprintf(b->debug1_fp, "%6d,", (int) b->nodes[i].node_pred[j].label);
			}						
			fprintf(b->debug1_fp, "\n");
			
			fprintf(b->debug1_fp, "Ehk:, ");
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{
				fprintf(b->debug1_fp, "%7.4f,", b->nodes[i].node_pred[j].Ehk / b->nodes[i].node_pred[j].nk);
			}						
			fprintf(b->debug1_fp, "\n");
			
			fprintf(b->debug1_fp, "cnt1:, ");
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{
				fprintf(b->debug1_fp, "%6d,", (int) b->nodes[i].node_pred[j].label_cnt);
			}						
			fprintf(b->debug1_fp, "\n");
			
			fprintf(b->debug1_fp, "cnt2:, ");
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{				
				fprintf(b->debug1_fp, "%6d,", (int) b->nodes[i].node_pred[j].label_cnt2);
				
				#ifdef TXM_O_ARBITRARY_ROOT
				if(i > 0)
				{
					total += b->nodes[i].node_pred[j].label_cnt2;
					
					if(b->nodes[i].id == b->nodes[0].id_left)
					{
						if(b->arbitrary.contain_sorted(b->nodes[i].node_pred[j].label))
							err += b->nodes[i].node_pred[j].label_cnt2;
					}
					else
					{
						if(!b->arbitrary.contain_sorted(b->nodes[i].node_pred[j].label))
							err += b->nodes[i].node_pred[j].label_cnt2;
					}
				}
				#endif
			}
			fprintf(b->debug1_fp, "\n");
			
			fprintf(b->debug1_fp, "max(label:cnt:total):, %3d,%6d,%7d,\n",  (int) b->nodes[i].max_cnt2_label, (int) b->nodes[i].max_cnt2, (int) b->nodes[i].total_cnt2);
			fprintf(b->debug1_fp, "left: %4d, right: %4d, parent: %4d,\n",  (int) b->nodes[i].id_left, (int) b->nodes[i].id_right, (int) b->nodes[i].id_parent);
			fprintf(b->debug1_fp, "\n");
		}
		
		#ifdef TXM_O_ARBITRARY_ROOT
		ftmp = err;
		ftmp /= total;
		printf("\nerror: %.4f\n", ftmp);
		#endif
	}
	#endif
	
	#ifdef TXM_O_DEBUG_FILE2
	void save_node_stats_pred(txm_o* b)							//wyswietlam statystyke dla poszczegonym nodow
	{
		uint32_t i, j;
		size_t index;
		float ftmp;
		uint32_t total = 0;
		uint32_t err = 0;
		
		for(i = 0; i < b->nodes.size(); i++)
		{
			if(b->nodes[i].removed)
				continue;
			
			fprintf(b->debug2_fp, "Node: %4d, Level: %2d, Leaf: %1d,\n", (int) i, (int) b->nodes[i].level, (int) b->nodes[i].leaf);
			
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{
				fprintf(b->debug2_fp, "%6d,", (int) b->nodes[i].node_pred[j].label);
			}						
			fprintf(b->debug2_fp, "\n");
			
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{				
				fprintf(b->debug2_fp, "%6d,", (int) b->nodes[i].node_pred[j].label_cnt2);
				
				#ifdef TXM_O_ARBITRARY_ROOT
				if(i > 0)
				{
					total += b->nodes[i].node_pred[j].label_cnt2;
					
					if(b->nodes[i].id == b->nodes[0].id_left)
					{
						if(b->arbitrary.contain_sorted(b->nodes[i].node_pred[j].label))
							err += b->nodes[i].node_pred[j].label_cnt2;
					}
					else
					{
						if(!b->arbitrary.contain_sorted(b->nodes[i].node_pred[j].label))
							err += b->nodes[i].node_pred[j].label_cnt2;
					}
				}
				#endif
			}
			fprintf(b->debug2_fp, "\n");
			
			if(b->nodes[i].node_pred.contain_sorted(txm_o_node_pred_type(b->nodes[i].max_cnt2_label), &index))
			{
				b->nodes[i].max_cnt2 = b->nodes[i].node_pred[index].label_cnt2;
				
				ftmp = b->nodes[i].max_cnt2;
				ftmp /= b->nodes[i].total_cnt2;
				ftmp = 1 - ftmp;
			}
			else
			{
				b->nodes[i].max_cnt2 = 0;
				ftmp = 1;
			}
			
			fprintf(b->debug2_fp, "max(label:cnt:total:error):, %3d,%6d,%7d,%7.5f,\n",  (int) b->nodes[i].max_cnt2_label, (int) b->nodes[i].max_cnt2, (int) b->nodes[i].total_cnt2, ftmp);
			fprintf(b->debug2_fp, "left: %4d, right: %4d, parent: %4d\n",  (int) b->nodes[i].id_left, (int) b->nodes[i].id_right, (int) b->nodes[i].id_parent);
			fprintf(b->debug2_fp, "\n");
		}
		
		ftmp = err;
		ftmp /= total;
		printf("\nerror: %.4f\n", ftmp);
	}
	#endif
	
	void clear_cnt2(txm_o* b)
	{
		uint32_t i, j;
	
		for(i = 0; i < b->nodes.size(); i++)
		{
			for(j = 0; j < b->nodes[i].node_pred.size(); j++)
			{
				b->nodes[i].node_pred[j].label_cnt2 = 0;
			}
		}
	}

	void predict(txm_o* d, example* ec)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		vw* all = d->all;
		size_t new_cn;
		size_t index;	
		uint32_t j;
		//float Eh_norm;
		
		#ifdef TXM_O_DEBUG_PRED
		int level = 0;
		#endif
		
		if(command_example(all,ec))
		{	
			d->base.learn(ec);
			return;
		}
		
		d->cn = 0;
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "\nExample: " << d->ex_num << endl;
		#endif
		
		#ifdef TXM_O_DEBUG_FILE2
		if(!d->nodes[d->cn].node_pred.contain_sorted(txm_o_node_pred_type(mc->label), &index))	//if the label is not in the root
		{
			d->nodes[d->cn].node_pred.push_back_sorted(txm_o_node_pred_type(mc->label));	//add the label to the list of labels in the root
			d->nodes[d->cn].node_pred.contain_sorted(txm_o_node_pred_type(mc->label), &index);
			d->nodes[d->cn].node_pred[index].label_cnt2++;
		}
		else
		{			
			d->nodes[d->cn].node_pred[index].label_cnt2++;
		}
		d->nodes[d->cn].total_cnt2++;
		#endif
		
		while(!d->nodes[d->cn].leaf)// && d->nodes[d->cn].level < d->level_limit)
		{
			update_example_indicies(all->audit, ec, d->increment * d->cn);	
		
			ec->test_only = true;
			label_data simple_temp;	
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp.label;
			d->base.learn(ec);
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			#ifdef TXM_O_DEBUG_PRED
			cout << "level: " << level++ << endl;
			cout << "node: " << d->cn << endl;
			#endif
			
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
		
			if(new_cn != 0)			
				d->cn = new_cn;

			if(new_cn == 0)		//blad - dziecko ma id 0, nigdy w to nie wchodze
				break;	

			#ifdef TXM_O_DEBUG_FILE2
			if(!d->nodes[d->cn].node_pred.contain_sorted(txm_o_node_pred_type(mc->label), &index))	//if the label is not in the root
			{
				d->nodes[d->cn].node_pred.push_back_sorted(txm_o_node_pred_type(mc->label));	//add the label to the list of labels in the root
				d->nodes[d->cn].node_pred.contain_sorted(txm_o_node_pred_type(mc->label), &index);
				d->nodes[d->cn].node_pred[index].label_cnt2++;
			}
			else
			{			
				d->nodes[d->cn].node_pred[index].label_cnt2++;
			}
			d->nodes[d->cn].total_cnt2++;
			#endif
		}
		
		ec->ld = mc;
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "Prediction finished...\n";
		#endif
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "Nb of labels in leaf: " << d->nodes[d->cn].node_pred.size() << endl;
		#endif
		
		//ec->final_prediction = d->nodes[d->cn].max_cnt2_label;		//PRZYPISYWANIE PREDYKCJI DO PRZYKLADU (necessary for external evaluation)
		
		ec->final_prediction = d->nodes[d->cn].node_labels.last().label;				
						
		for(j = 0; j < TXM_O_LEAF_TOL && j < d->nodes[d->cn].node_labels.size(); j++)
		{
			if(d->nodes[d->cn].node_labels[d->nodes[d->cn].node_labels.size() - j - 1].label == mc->label)
			{
				ec->final_prediction = mc->label;
				break;
			}
		}
		
		/*if(d->cn == 0)
		{
			ec->final_prediction = -1;
			cout << "Prediction error: " << err_cnt++ << endl;
 		}*/
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "labels O:P:\t" << mc->label << ":" << ec->final_prediction << endl;
		#endif
		
		d->ex_num++;					//pozostalosc po starym kodzie
	}	
	
	uint32_t predict_node(txm_o* d, example* ec, size_t level_limit)		//to samo co predict tylko nie to samo do konca, minimalny argument z jakim to jest odpalane to jest 1 (czyli masz root i dwa liscie)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		vw* all = d->all;
		size_t level = 0;	
		//float Eh_norm;
		
		if(command_example(all,ec))
		{
			d->base.learn(ec);
			return 0;
		}
		
		d->cn = 0;
		
		while(!d->nodes[d->cn].leaf && level < level_limit)
		{
			update_example_indicies(all->audit, ec, d->increment * d->cn);
		
			ec->test_only = true;					
			label_data simple_temp;
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp.label;
			d->base.learn(ec);			
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			#ifdef TXM_O_DEBUG_PRED
			cout << "level: " << level << endl;
			cout << "node: " << d->cn << endl;
			#endif			
			
			if(ec->final_prediction < 0)
			{
				d->cn = d->nodes[d->cn].id_left;
			}
			else
			{
				d->cn = d->nodes[d->cn].id_right;
			}
			
			level++;
			
			if(d->cn == 0)								//zabezpieczenie, nigdy nie jest wywolywane
			{
				//cout << "Node prediction error!!\n";
				break;
			}
		}		
		
		ec->ld = mc;
			
		return d->cn;
	}
	
	void learn(void* d, example* ec) 
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		txm_o* b = (txm_o*)d;
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
		size_t j, k;
		label_data simple_temp;	
		txm_o_node_labels_type node_label_tmp;
		
        simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;
		
		#ifdef TXM_O_DEBUG
		unsigned char* i;
		#endif
		
		vw* all = b->all;	
		
		if(!all->training)
		{
			predict(b, ec);
			return;
		}
		
		oryginal_label = mc->label;	
		
		if(command_example(all,ec))		//po kazdym przejsciu przez zbior danych on wchodzi w command example
		{		
			b->base.learn(ec);
			
			if(ec->end_pass)
			{
				b->ex_total = b->ex_num;
				
				b->ex_num = 0;				
							
				b->current_pass++;

				#ifdef TXM_O_DEBUG_PASS_STOP
				cin.ignore();
				#endif
				
				//printf("current pass: %d\n", (int) b->current_pass);
			}	
			return;
		}	

		b->cl = 0;	
		
		while(b->cl <= TXM_O_LEVEL_LIM)
		{
			if(b->cl == 0)			
			{			
				b->cn = 0;
				
				//add label to label list
				if(!b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))	//if the label is not in the root
				{
					b->nodes[b->cn].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));	//add the label to the list of labels in the root
					b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index);
					b->nodes[b->cn].node_pred[index].label_cnt++;
					b->nodes[b->cn].node_pred[index].label_cnt2++;
				}
				else
				{
					b->nodes[b->cn].node_pred[index].label_cnt++;				
					b->nodes[b->cn].node_pred[index].label_cnt2++;	
				}
			}
			else								//not the first pass through the data, jezeli b->current_pass == 1 to tworze drugi level drzewa, czyli trenuje regresory pierwszego levela
			{
				b->cn = predict_node(b, ec, b->cl);		//current node, ktory ja zamierzam trenowac
				
				if(b->cn == 0)   //blad, wychodze z learn i dalej bede analizowac kolejny przyklad kolejny przyklad
				{
					b->ex_num++;
					return;
				}			
				
				if(b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))		//jezeli label jest w tablicy node_pred'ow 
				{
					if(b->nodes[b->cn].node_pred[index].label_cnt2 > b->nodes[b->cn].max_cnt2)					
					{
						b->nodes[b->cn].max_cnt2 = b->nodes[b->cn].node_pred[index].label_cnt2;
						b->nodes[b->cn].max_cnt2_label = b->nodes[b->cn].node_pred[index].label;
					}
				}
				else
				{
					index = b->nodes[b->cn].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));
					node_label_tmp.label_cnt2 = 0;
					node_label_tmp.label = oryginal_label;
					b->nodes[b->cn].node_labels.push_back_sorted(node_label_tmp);		
				}
				
				b->nodes[b->cn].node_pred[index].label_cnt2++;						
				b->nodes[b->cn].total_cnt2++;
				
				if(b->nodes[b->cn].node_pred[index].label_cnt2 > b->nodes[b->cn].max_cnt2)
				{
					b->nodes[b->cn].max_cnt2 = b->nodes[b->cn].node_pred[index].label_cnt2;
					b->nodes[b->cn].max_cnt2_label = b->nodes[b->cn].node_pred[index].label;
				}

				if(b->cl >= TXM_O_LEVEL_LIM)
				{
					for(j = 0; j < b->nodes[b->cn].node_labels.size(); j++)
					{
						if(b->nodes[b->cn].node_labels[j].label == oryginal_label)
						{
							b->nodes[b->cn].node_labels.remove_sorted(j);
							node_label_tmp.label_cnt2 = b->nodes[b->cn].node_pred[index].label_cnt2;
							node_label_tmp.label = oryginal_label;
							b->nodes[b->cn].node_labels.push_back_sorted(node_label_tmp);	
							break;
						}
					}			
					
					ec->final_prediction = b->nodes[b->cn].node_labels.last().label;				
						
					if(b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))
					{
						for(j = 0; j < TXM_O_LEAF_TOL && j < b->nodes[b->cn].node_labels.size(); j++)
						{
							if(b->nodes[b->cn].node_labels[b->nodes[b->cn].node_labels.size() - j - 1].label == oryginal_label)
							{
								ec->final_prediction = oryginal_label;
								break;
							}
						}
					}
					
					b->nodes[b->cn].leaf = true;				
					break;
				}
			}
			
			#ifdef TXM_O_DEBUG
			cout << "Current node: " << b->cn << endl;
			cout << "Example number: " << b->ex_num << endl;
			#endif		
				
			//do the initial prediction to decide if going left or right
			all->sd->min_label = -TXM_O_PRED_LIM;
			all->sd->max_label = TXM_O_PRED_LIM;	
			//mc->label = 0;	//jezeli jest cos innego tu to on moze zmieniac clipping zakresy
			update_example_indicies(all->audit, ec, b->increment * b->cn);
			ec->test_only = true;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp;
			b->base.learn(ec);	
			//mc->label = oryginal_label;	
			
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
			
			id_left = b->nodes[b->cn].id_left;
			id_right = b->nodes[b->cn].id_right;	

			#ifdef TXM_O_ARBITRARY_ROOT
			if(b->cn == 0)
			{
				if(b->arbitrary.contain_sorted(oryginal_label))
					left_or_right = 1;
				else
					left_or_right = -1;
			}
			#endif		
					
			if(left_or_right < 0)
			{
				//mc->label = -1.f;
				simple_temp.label = -1.f;
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
				//mc->label = 1.f;
				simple_temp.label = 1.f;
				id_left_right = id_right;
				
				if(b->nodes[b->cn].id_right == 0)												//if the right child does not exist
				{
					id_left_right = id_right = b->nodes.size();									//node identifier = number-of_existing_nodes + 1
					b->nodes.push_back(init_node(id_right, b->cn, b->nodes[b->cn].level + 1));	//add new node to the tree
					b->nodes[b->cn].id_right = id_right;										//new node is the right child of the current node	
				}
			}	
				
			if(!b->nodes[id_left_right].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))  //if the label does not exist in the left/right child of the current node
			{
				index = b->nodes[id_left_right].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));		//add the label to the left/right child of the current node					
				node_label_tmp.label_cnt2 = 0;
				node_label_tmp.label = oryginal_label;
				b->nodes[id_left_right].node_labels.push_back_sorted(node_label_tmp);	
			}
			
			b->nodes[id_left_right].node_pred[index].label_cnt++;
			b->nodes[id_left_right].node_pred[index].assigned = true;
			
			//learn!!
			ec->partial_prediction = 0;
			ec->final_prediction = 0;	
			all->sd->min_label = -TXM_O_PRED_LIM;
			all->sd->max_label = TXM_O_PRED_LIM;
			ec->test_only = false;
			b->base.learn(ec);	
			//mc->label = oryginal_label;	
			ec->ld = mc;	
				
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
			cout << endl;
			#endif				
			
			if(b->cn > 0)
				update_example_indicies(all->audit, ec, -b->increment * b->cn);

			#ifdef TXM_O_DEBUG
			cout << "Current Pass: " << b->current_pass << endl;
			cout << endl;
			#endif		
			
			#ifdef TXM_O_DEBUG_EX_STOP
			cin.ignore();
			#endif
			
			b->cl++;
		}	

		b->ex_num++;			
	}
	
	void drive(vw* all, void* d)
	{	
		txm_o* b = (txm_o*)d;	
		example* ec = NULL;			
		
		while (true)
		{
			if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
			{
				learn((txm_o*)d, ec);
				
				if (!command_example(all, ec))
					OAA::output_example(*all, ec);
					
				VW::finish_example(*all, ec);				
			}
			else if (parser_done(all->p))
			{				
				if(all->training)
				{
					#ifdef TXM_O_DEBUG_FILE1
					save_node_stats(b);
					fclose(b->debug1_fp);
					#endif
				}
				else
				{
					#ifdef TXM_O_DEBUG_FILE2
					save_node_stats_pred(b);
					fclose(b->debug2_fp);
					#endif		
				}	
					
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
	
	void txm_o_save_load_regressor(vw& all, io_buf& model_file, bool read, bool text)
	{
		uint32_t length = 1 << all.num_bits;
		uint32_t stride = all.reg.stride;
		int c = 0;
		uint32_t i = 0;
		size_t brw = 1;

		if(all.print_invert){ //write readable model with feature names           
			weight* v;
			char buff[512];
			int text_len; 
			typedef std::map< std::string, size_t> str_int_map;  

			for(str_int_map::iterator it = all.name_index_map.begin(); it != all.name_index_map.end(); ++it){              
				v = &(all.reg.weight_vector[stride*(it->second)]);
				if(*v != 0.){
					text_len = sprintf(buff, "%s", (char*)it->first.c_str());
					brw = bin_text_write_fixed(model_file, (char*)it->first.c_str(), sizeof(*it->first.c_str()), buff, text_len, true);
					text_len = sprintf(buff, ":%f\n", *v);
					brw+= bin_text_write_fixed(model_file,(char *)v, sizeof (*v), buff, text_len, true);
				}	
			}			
			
			return;
		} 

		do 
		{
			brw = 1;
			weight* v;
			if (read)
			{
				c++;
				brw = bin_read_fixed(model_file, (char*)&i, sizeof(i),"");
				if (brw > 0)
				{
					if(i == 0x7FFFFFFF)
					{
						//cout << "TEST!!\n";
						//cin.ignore();
						return;
					}
	
					assert (i< length);		
					v = &(all.reg.weight_vector[stride*i]);
					brw += bin_read_fixed(model_file, (char*)v, sizeof(*v), "");
				}
			}
			else// write binary or text
			{
				v = &(all.reg.weight_vector[stride*i]);
				if (*v != 0.)
				{
					c++;
					char buff[512];
					int text_len;

					text_len = sprintf(buff, " %d", i);
					brw = bin_text_write_fixed(model_file,(char *)&i, sizeof (i), buff, text_len, text);

					text_len = sprintf(buff, ":%f\n", *v);
					brw+= bin_text_write_fixed(model_file,(char *)v, sizeof (*v), buff, text_len, text);
				}
			}

			if (!read)
				i++;
		}while ((!read && i < length) || (read && brw > 0));  
		
		if(!read)		
		{
			char buff[512];
			int text_len;
			
			i = 0x7FFFFFFF;
			text_len = sprintf(buff, " %d", i);
			brw = bin_text_write_fixed(model_file,(char *)&i, sizeof (i), buff, text_len, text);
		}
	}

	void save_load_tree(vw& all, io_buf& model_file, bool read, bool text)
	{ 
		txm_o* b = (txm_o*)all.l.sl.sldata;
		char buff[512];
		uint32_t i = 0;
		uint32_t j = 0;
		uint32_t k = 0;
		size_t brw = 1; 
		uint32_t v, w;
		int text_len;
		txm_o_node_labels_type node_label_tmp;
		
		if(read)
		{
			brw = bin_read_fixed(model_file, (char*)&i, sizeof(i), "");
			
			for(j = 0; j < i; j++)
			{				
				b->nodes.push_back(init_node(j, 0, 0));
				
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].id = v;
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].id_left = v;
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].id_right = v;
				
				/*brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].max_cnt2_label = v;*/
				
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				w = v;
				
				for(k = 0; k < w; k++)
				{
					brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
					node_label_tmp.label_cnt2 = 0;
					node_label_tmp.label = v;
					b->nodes[j].node_labels.push_back_sorted(node_label_tmp);
				}
				
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].leaf = v;
				//printf("%ld, %ld, %ld, %d, %d, \n", b->nodes[j].id, b->nodes[j].id_left, b->nodes[j].id_right, b->nodes[j].max_cnt2_label, b->nodes[j].leaf);
			}
		}
		else
		{
			text_len = sprintf(buff, ":%d\n", (int) b->nodes.size());
			v = b->nodes.size();
			brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);

			for(i = 0; i < b->nodes.size(); i++)
			{
				text_len = sprintf(buff, ":%d", (int) b->nodes[i].id);
				v = b->nodes[i].id;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				text_len = sprintf(buff, ":%d", (int) b->nodes[i].id_left);
				v = b->nodes[i].id_left;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				text_len = sprintf(buff, ":%d", (int) b->nodes[i].id_right);
				v = b->nodes[i].id_right;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				/*text_len = sprintf(buff, ":%d", (int) b->nodes[i].max_cnt2_label);
				v = b->nodes[i].max_cnt2_label;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);*/
				
				if(b->nodes[i].node_labels.size() < TXM_O_LEAF_TOL)
					v = b->nodes[i].node_labels.size();
				else
					v = TXM_O_LEAF_TOL;
				text_len = sprintf(buff, ":%d", (int) v);				
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				for(k = 0; k < v; k++)
				{
					w = b->nodes[i].node_labels[b->nodes[i].node_labels.size() - k - 1].label;
					text_len = sprintf(buff, ":%d", (int) w);				
					brw = bin_text_write_fixed(model_file,(char *)&w, sizeof (w), buff, text_len, text);
				}
				
				text_len = sprintf(buff, ":%d\n", b->nodes[i].leaf);
				v = b->nodes[i].leaf;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);		
			}	
		}		
	}
	
	void txm_o_save_load(void* data, io_buf& model_file, bool read, bool text)
	{
		txm_o* g = (txm_o*)data;			//BARDZO WAZNE JEST ZOSTAWIENIE TEGO g, W PRZECIWNYM WYPADKU ON SIE ODWOLUJE DO TEGO CZEGO NIE MA
		vw* all = g->all;
		
		if(read)
		{
			initialize_regressor(*all);
			if(all->adaptive && all->initial_t > 0)
			{
				uint32_t length = 1 << all->num_bits;
				uint32_t stride = all->reg.stride;
				for (size_t j = 1; j < stride*length; j+=stride)
				{
					all->reg.weight_vector[j] = all->initial_t;   
				}
			}
		}

		if (model_file.files.size() > 0)
		{
			txm_o_save_load_regressor(*all, model_file, read, text);
			save_load_tree(*all, model_file, read, text);
		}
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
		all.weights_per_problem *= TXM_O_MULTIPLICATIVE_FACTOR*data->k;
		data->base = all.l;
		//TUTAJ ZAMIAST all.l.sl BEDE MIALA WLASNA STRUKTURE DO ZAPAMIETANIA, powinnam odkomentowac te dwie linijki i zakomentowac linijke z learner
	    sl_t sl = {data, txm_o_save_load};					//TU JEST MOJA WLASNA STRUKTURA DO ZAPAMIETANIA
	    learner l(data, drive, learn, finish, sl);		//CZYLI TUTAJ ZAMIAST all.l.sl uzywamy po prostu sl
		//learner l(data, drive, learn, finish, all.l.sl);
		
		txm_o* b = (txm_o*)data;
		
		b->current_pass = 0;
		b->level_limit = TXM_O_LEVEL_LIM;
		
		if(all.training)
			init_tree(b);		
		else	
		{			
			b->tree_finished = false;	
			
			#ifdef TXM_O_DEBUG_FILE2
			b->debug2_fp = fopen("atxm_o_debug2.csv", "wt");
			#endif
		}
		
		#ifdef TXM_O_ARBITRARY_ROOT
		int i;
		uint32_t tmp, cnt;
		b->arbitrary_fp = fopen("atxm_o_arbiter.txt", "rt");
		
		fscanf(b->arbitrary_fp, "%d\n", &cnt);
		printf("\ncnt: %d\n", cnt);
		
		for(i = 0; i < cnt; i++)
		{
			fscanf(b->arbitrary_fp, "%d\n", &tmp);
			printf("%d, ", tmp);
			b->arbitrary.push_back_sorted(tmp);
		}
		printf("\n\n");		
		#endif
		
		return l;
	}	
}


