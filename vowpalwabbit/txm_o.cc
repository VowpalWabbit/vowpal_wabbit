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

using namespace std;

namespace TXM_O 
{
	class txm_o_node_pred_type								//w kazdym nodzie mam jedna tablice elementow tego typu, kazde wejscie w tej tablicy odpowiada labelowi ktory wpadl do tego noda podczas trenowania lub predykcji (odpalanie funkcji predict_node)
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
		size_t max_cnt2;                                   //maxymalna wartosc countera 2 w nodzie
		size_t max_cnt2_label;                             //label z maxymalnym counterem w nodzie
		size_t total_cnt2;                                 //laczna ilosc punktow nodzie prxzypisanych przez predyktor do noda
		size_t reg_num;									   //regressor number when oaa starts for the node (I have implementations when one regressor is in a leaf or many, for non-leaf it is empty)	
		
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
		
		v_array<txm_o_node_type> nodes;	//the nodes - czyli nasze drzewo
		
		size_t cn;						//current node
		size_t cl;						//current level: level of the cn
		size_t ex_num;					//index of current example
		size_t ex_total;
		size_t reg_num_max;				//index ostatniego regresora (w drzewie + oaa) + 1
		
		size_t level_limit;             //maxymlane glebokosc drzewa (liscie maja byc na tym levelu, czyli 1 odpowiada rootowi i dwom lisciom)
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
		node.max_cnt2 = 0;
		node.max_cnt2_label = 0;
		node.total_cnt2 = 0;
		node.reg_num = 0;
		
		return node;
	}
	
	void init_tree(txm_o* d)                               //inicjalizacja drzewa
	{
		d->cn = 0;
		d->ex_num = 0;
		d->nodes.push_back(init_node(0, 0, 0));
		d->reg_num_max = (2 << TXM_O_LEVEL_LIM) - 1;	  //ilosc regresorow w samym drzewie (jak bede dodawac oaa bede to zwiekszac)
	}	
	
	void predict(txm_o* d, learner& base, example* ec)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		label_data simple_temp;	
		size_t new_cn;
		uint32_t j, k;
		float min_pred, max_oaa;
		uint32_t min_pred_index;
		uint32_t max_oaa_index;	
		v_array<uint32_t> node_list;
		v_array<float> node_list_pred;
		
		v_array<uint32_t> label_list;
		v_array<uint32_t> leaf_list;
		
		#ifdef TXM_O_DEBUG_PRED
		int level = 0;
		#endif
			
		d->cn = 0;		//ustawiamy root
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "\nExample: " << d->ex_num << endl;
		#endif
		
		for(j = 0; j <= TXM_O_LEVEL_LIM; j++)
		{
			while(!d->nodes[d->cn].leaf)
			{
				simple_temp.initial = 0.0;				//zywcem skopiowane z ich kodow
				simple_temp.weight = mc->weight;
				simple_temp.label = FLT_MAX;
				ec->ld = &simple_temp.label;
				base.learn(ec, d->cn);			
				
				#ifdef TXM_O_DEBUG_PRED
				cout << "level: " << level++ << endl;
				cout << "node: " << d->cn << endl;
				#endif
				
				node_list.push_back(d->cn);				
				node_list_pred.push_back(fabs(ec->final_prediction));
	
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
			}
			//jezeli ta petla sie skonczyla to znaczy ze jestesmy w lisciu
			//do ponizszych list dodaje wszystkie leafy do ktorych doszlismy (label leafa i index)
			label_list.push_back(d->nodes[d->cn].max_cnt2_label);
			leaf_list.push_back(d->cn);
			
			min_pred = node_list_pred[0];
			min_pred_index = 0;
			//wybieram node z najmniejsza predykcja (przy pierwszym przejsciu petli for(j...) wyberam z nodow pryncypalnej sciezki)
			for(k = 1; k < node_list.size(); k++)
			{
				if(node_list_pred[k] TXM_O_TRK_COMP min_pred)
				{
					min_pred = node_list_pred[k];
					min_pred_index = k;
				}
			}
			
			//ustawiam d->cn na node z najmniejsza predykcja
			d->cn = node_list[min_pred_index];
			//usuwam d->cn z list node_list i node_list_pred
			node_list[min_pred_index] = node_list.pop();
			node_list_pred[min_pred_index] = node_list_pred.pop();
			
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp.label;
			base.learn(ec, d->cn);			

			if(ec->final_prediction > 0)
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
		}		
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "Prediction finished...\n";
		#endif
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "Nb of labels in leaf: " << d->nodes[d->cn].node_pred.size() << endl;
		#endif
		
		//jeden regresor w leafie, wybieram leaf dla ktorego byla maksymalna predykcja
		max_oaa_index = 0;
		max_oaa = -1.f;
		
		for(j = 0; j < leaf_list.size(); j++)
		{
			d->cn = leaf_list[j];
			
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp.label;
			base.learn(ec, d->nodes[d->cn].reg_num);
			
			if(ec->final_prediction > max_oaa)
			{
				max_oaa = ec->final_prediction;
				max_oaa_index = j;
			}
		}
		
		ec->ld = mc;
		
		d->cn = leaf_list[max_oaa_index];
			
		ec->final_prediction = label_list[max_oaa_index];		//do ewaluacji bledu przez vw
		
		/*cout<<endl;
		for(j = 0; j < label_list.size(); j++)
			cout<<label_list[j]<<"\t";
		cout<<"\t:"<<mc->label<<"\t:"<<ec->final_prediction<<endl;
		for(j = 0; j < label_list.size(); j++)
			cout<<label_list_oaa[j]<<"\t";
		cout<<endl;*/
		
		/*for(j = 0; j < label_list.size(); j++)
		{
			if(label_list[j] == mc->label)
			{
				ec->final_prediction = mc->label;
				break;
			}
		}*/
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "labels O:P:\t" << mc->label << ":" << ec->final_prediction << endl;
		#endif
		
		d->ex_num++;					//pozostalosc po starym kodzie
	}

	void predict_node_list(txm_o* d, learner& base, example* ec, size_t* out_node_list, size_t &out_node_list_index)   //dziala tak samo jak predict tylko nie odpala oaa, uzywana w learnie, zwraca liste lisci ktore beda trenowane w oaa
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		label_data simple_temp;	
		size_t new_cn;
		uint32_t j, k;
		float min_pred;
		uint32_t min_pred_index;
		v_array<uint32_t> node_list;
		v_array<float> node_list_pred;
		
		#ifdef TXM_O_DEBUG_PRED
		int level = 0;
		#endif
		
		out_node_list_index = 0;	
		d->cn = 0;				
		
		#ifdef TXM_O_DEBUG_PRED
		cout << "\nExample: " << d->ex_num << endl;
		#endif
					
		for(j = 0; j <= TXM_O_LEVEL_LIM; j++)
		{
			while(!d->nodes[d->cn].leaf)
			{
				ec->test_only = true;
				simple_temp.initial = 0.0;
				simple_temp.weight = mc->weight;
				simple_temp.label = FLT_MAX;
				ec->ld = &simple_temp.label;
				base.learn(ec, d->cn);			
				
				#ifdef TXM_O_DEBUG_PRED
				cout << "level: " << level++ << endl;
				cout << "node: " << d->cn << endl;
				#endif
				
				node_list.push_back(d->cn);
				node_list_pred.push_back(fabs(ec->final_prediction));
	
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
			}
			
			if(d->nodes[d->cn].leaf)
			{
				out_node_list[out_node_list_index] = d->cn;
				out_node_list_index++;
			}
			
			min_pred = node_list_pred[0];
			min_pred_index = 0;
			
			for(k = 1; k < node_list.size(); k++)
			{
				if(node_list_pred[k] TXM_O_TRK_COMP min_pred)
				{
					min_pred = node_list_pred[k];
					min_pred_index = k;
				}
			}
			
			d->cn = node_list[min_pred_index];
			node_list[min_pred_index] = node_list.pop();
			node_list_pred[min_pred_index] = node_list_pred.pop();
			
			ec->test_only = true;
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp.label;
			base.learn(ec, d->cn);			

			if(ec->final_prediction > 0)
			{
				new_cn = d->nodes[d->cn].id_left;
			}
			else
			{
				new_cn = d->nodes[d->cn].id_right;
			}
		
			if(new_cn != 0)			
				d->cn = new_cn;

			if(new_cn == 0)		
				break;
		}
		
		ec->ld = mc;	
	}	
	
	uint32_t predict_node(txm_o* d, learner& base, example* ec, size_t level_limit)		//dla przykladu wybiera index noda na zadanym poziomie dla danego przykladu do ktorego doszedl
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		size_t level = 0;	
		//float Eh_norm;		
		
		d->cn = 0;
		
		while(!d->nodes[d->cn].leaf && level < level_limit)
		{			
			ec->test_only = true;					
			label_data simple_temp;
			simple_temp.initial = 0.0;
			simple_temp.weight = mc->weight;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp.label;
			base.learn(ec, d->cn);
			
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
	
	void learn(void* d, learner& base, example* ec)//(void* d, example* ec) 
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
		size_t id_current;
		size_t id_left_right;
		size_t j;
		label_data simple_temp;	
		size_t node_list[1000];
		size_t node_list_index;
		
        simple_temp.initial = 0.0;
		simple_temp.weight = mc->weight;
		
		#ifdef TXM_O_DEBUG
		unsigned char* i;
		#endif
		
		vw* all = b->all;	
		
		if(!all->training)
		{
			predict(b, base, ec);
			return;
		}
		
		oryginal_label = mc->label;				

		b->cl = 0;	
		
		while(b->cl <= TXM_O_LEVEL_LIM)
		{
			//ta czesc wybiera b->cn na poziomie b->cl ktory bedziemy trenowac
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
			else								
			{
				b->cn = predict_node(b, base, ec, b->cl);		//current node, ktory ja zamierzam trenowac
				
				if(b->cn == 0)   //zabezpieczenie, nigdy w to nie wchodzimy, blad, wychodze z learn i dalej bede analizowac kolejny przyklad kolejny przyklad
				{
					b->ex_num++;
					return;
				}
				
				if(!b->nodes[b->cn].node_pred.contain_sorted(txm_o_node_pred_type(oryginal_label), &index))		//jezeli label nie jest w tablicy node_pred'ow 
				{
					index = b->nodes[b->cn].node_pred.push_back_sorted(txm_o_node_pred_type(oryginal_label));
				}
				
				b->nodes[b->cn].node_pred[index].label_cnt2++;						
				b->nodes[b->cn].total_cnt2++;
				
				if(b->nodes[b->cn].node_pred[index].label_cnt2 > b->nodes[b->cn].max_cnt2)
				{
					b->nodes[b->cn].max_cnt2 = b->nodes[b->cn].node_pred[index].label_cnt2;
					b->nodes[b->cn].max_cnt2_label = b->nodes[b->cn].node_pred[index].label;
				}
				
				if(b->cl >= TXM_O_LEVEL_LIM)	//leaf level
				{					
					b->nodes[b->cn].leaf = true;										
					id_current = b->cn;
					predict_node_list(b, base, ec, node_list, node_list_index);					
					ec->test_only = false;
						
					//train for OAA					
					for(j = 0; j < node_list_index; j++)	//node list ma indeksy wszystkich lisci od pryncypalnej sciezki i alternatywnych sciezek
					{
						b->cn = node_list[j];
						
						if(b->nodes[b->cn].reg_num == 0)		//lisc nie ma jeszcze przypisanego regresora oaa
						{
							b->nodes[b->cn].reg_num = b->reg_num_max;							
							b->reg_num_max++;
						}
						
						if(b->nodes[b->cn].max_cnt2_label == oryginal_label)
							simple_temp.label = 1.f;
						else
							simple_temp.label = -1.f;
						
						b->cn = b->nodes[b->cn].reg_num;	//biore numer regresora dla liscia i potem go trenuje
						
						ec->ld = &simple_temp;				
						ec->partial_prediction = 0;
						ec->final_prediction = 0;					
						ec->test_only = false;					
						base.learn(ec, b->cn);
						ec->ld = mc;						
					}				
					
					b->cn = id_current;
					ec->final_prediction = b->nodes[b->cn].max_cnt2_label;		//nie uwzgledniam oaa
					break;
				}
			}
			
			//trenujemy wczesniej wybrany node
			#ifdef TXM_O_DEBUG
			cout << "Current node: " << b->cn << endl;
			cout << "Example number: " << b->ex_num << endl;
			#endif		
				
			//do the initial prediction to decide if going left or right
			all->sd->min_label = -TXM_O_PRED_LIM;
			all->sd->max_label = TXM_O_PRED_LIM;	
			//mc->label = 0;	//jezeli jest cos innego tu to on moze zmieniac clipping zakresy
			ec->test_only = true;
			simple_temp.label = FLT_MAX;
			ec->ld = &simple_temp;
			base.learn(ec, b->cn);	
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
			}
			
			b->nodes[id_left_right].node_pred[index].label_cnt++;
			b->nodes[id_left_right].node_pred[index].assigned = true;
			
			//learn!!
			ec->partial_prediction = 0;
			ec->final_prediction = 0;	
			all->sd->min_label = -TXM_O_PRED_LIM;
			all->sd->max_label = TXM_O_PRED_LIM;
			ec->test_only = false;
			base.learn(ec, b->cn);	
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
			
			#ifdef TXM_O_DEBUG
			cout << "Current Pass: " << b->current_pass << endl;
			cout << endl;
			#endif				
			
			b->cl++;
		}	

		b->ex_num++;			
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
		txm_o* b = (txm_o*)all.l->get_save_load_data();
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
				b->nodes.push_back(init_node(j, 0, 0));
				
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].id = v;
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].id_left = v;
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].id_right = v;				
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].max_cnt2_label = v;				
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].leaf = v;
				brw +=bin_read_fixed(model_file, (char*)&v, sizeof(v), "");
				b->nodes[j].reg_num = v;
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
				
				text_len = sprintf(buff, ":%d", (int) b->nodes[i].max_cnt2_label);
				v = b->nodes[i].max_cnt2_label;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);				
				
				text_len = sprintf(buff, ":%d\n", b->nodes[i].leaf);
				v = b->nodes[i].leaf;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);				
				
				text_len = sprintf(buff, ":%d\n", (int) b->nodes[i].reg_num);
				v = b->nodes[i].reg_num;
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
		
		//data->increment = all.reg.stride * all.weights_per_problem;
		//all.weights_per_problem *= TXM_O_MULTIPLICATIVE_FACTOR*data->k;
		//data->base = all.l;
		//TUTAJ ZAMIAST all.l.sl BEDE MIALA WLASNA STRUKTURE DO ZAPAMIETANIA, powinnam odkomentowac te dwie linijki i zakomentowac linijke z learner
	    //sl_t sl = {data, txm_o_save_load};					//TU JEST MOJA WLASNA STRUKTURA DO ZAPAMIETANIA
	    //learner l(data, drive, learn, finish, sl);		//CZYLI TUTAJ ZAMIAST all.l.sl uzywamy po prostu sl
		//learner l(data, drive, learn, finish, all.l.sl);
		
		//learner* l = new learner(data, learn, txm_o_save_load, 2 * data->k);
		learner* l = new learner(data, learn, all.l, txm_o_save_load, (TXM_O_LEVEL_LIM + 4) * data->k);
		l->set_finish_example(OAA::finish_example);
		
		txm_o* b = (txm_o*)data;
		
		b->level_limit = TXM_O_LEVEL_LIM;
		
		if(all.training)
			init_tree(b);		
		
		return l;
	}	
}


