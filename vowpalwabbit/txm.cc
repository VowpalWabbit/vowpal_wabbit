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
	class txm_node_pred_type								//w kazdym nodzie mam jedna tablice elementow tego typu, kazdej wejscie w tej tablicy odpowiada laelowi ktory wpadl do tego noda podczas trenowania lub predykcji (odpalanie funkcji predict_node)
	{
		public:
		
		float 		Ehk;
		uint32_t 	nk;
		uint32_t	label;									//dla kazdego labela mam Ehk, nk, tu jest suma labeli ktore trafily do noda czy w czasie trenowania czy w czasie predykcji
		uint32_t	label_cnt;								//ilosc przykladow o tym labelu w tym nodzie przypisanych przez expected
		uint32_t	label_cnt2;								//ilosc przykladow o tym labelu w tym nodzie przypisanych przez predictor
		
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
		
		txm_node_pred_type(uint32_t l)						//konstruktor z ustawianiem labela
		{
			label = l;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			label_cnt2 = 0;
		}
		
		txm_node_pred_type()								//konstruktor bez ustawiania labela
		{
			label = 0;
			Ehk = 0.f;
			nk = 0;
			label_cnt = 0;
			label_cnt2 = 0;
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
		
		v_array<size_t> ex_node;		//pozostalosc po starym kodzie - nie korzystam teraz
		v_array<txm_node_type> nodes;	//the nodes - czyli nasze drzewo
		
		size_t cn;						//current node
		size_t ex_num;					//index of current example
		size_t ex_total;
		
		size_t current_pass;			//index of current pass through the data	
		size_t level_limit;             //maxymlane glebokosc drzewa (liscie maja byc na tym levelu, czyli 1 odpowiada rootowi i dwom lisciom)
		
		bool only_leafs;                //only leafs jest wystaione na true gdy mam same liscie (czyste liscie lub drzewo maxymalnej glebokosci i wtedy wszystkie nody na tej glebokosci sa liscmi)
		bool tree_finished;             //zakonczenie drzewa
		
		uint32_t total_wrg;             //w predictie liczone, sluzone do liczenia tego samego bledu ktory liczy VW
		uint32_t total_cor;             //w predictie liczone, sluzone do liczenia tego samego bledu ktory liczy VW
		
		char tree_file_name[256];
	};	

	txm_node_type init_node(size_t id, size_t id_parent, size_t level)        //inicjalizowanie nowego noda drzewa, odpalane przy tworzeniu nowego noda
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
		
		return node;
	}
	
	void init_tree(txm* d)                               //inicjalizacja drzewa
	{
		d->cn = 0;
		d->ex_num = 0;
		d->nodes.push_back(init_node(0, 0, 0));
		d->tree_finished = false;
		d->only_leafs = false;
	}
	
	/*void display_tree(txm* d)                           //wyswietlanie drzewa
	{
		size_t i;
		size_t j;
		size_t level = 0;
		
		for(j = 0; j < d->nodes.size(); j++)			//for every tree node
		{
			if(d->nodes[j].level > level)				//nowa linia gdy jest nowy label
			{
				cout << endl;
				level++;
			}
			
			if(d->nodes[j].removed)						//nie wyswietlamy nodow ktore zostaly oznaczone jako removed
				continue;
			
			cout << j << ":";
			
			if(d->nodes[j].leaf)
				cout << "[";
			else
				cout << "(";
			
			for(i = 0; i < d->nodes[j].node_pred.size(); i++)	//wypisuje wszystkie labele w nodzie ktore trafily na podstawie predyktora
			{
				cout << d->nodes[j].node_pred[i].label;
				
				if(i < d->nodes[j].node_pred.size() - 1)        //po ostatnim labelu w nodzie jest nawiasik a nie przecineczek, wiec stawiamy przecineczek po kazdym z wyjatkiem ostatniego labela w nodzie
					cout << ",";
			}
			
			if(d->nodes[j].leaf)
				cout << "] ";
			else
				cout << ") ";			
		}
		
		cout << endl;	
		cout << endl;		
	}*/
	
	/*void display_node_stats(txm* b)							//wyswietlam statystyke dla poszczegonym nodow
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
				
				printf("max(label:cnt:total): %3d:%6d:%7d",  (int) b->nodes[i].max_cnt2_label, (int) b->nodes[i].max_cnt2, (int) b->nodes[i].total_cnt2);
				//cout << "max:\t" << b->nodes[i].max_cnt2_label << ":" << b->nodes[i].max_cnt2 << "\ttotal:\t" << b->nodes[i].total_cnt2;
				
				cout << endl;
				cout << endl;
			}
		}
	}*/

	/*void save_tree(txm* b)
	{
		FILE *fp;
		size_t i;
		
		fp = fopen(b->tree_file_name, "wt");
		
		if(fp == NULL)
		{
			cout << "Cannot open file: " << b->tree_file_name << endl;
			return;
		}
		
		fprintf(fp, "%d\n", b->nodes.size());
		
		for(i = 0; i < b->nodes.size(); i++)
		{
			fprintf(fp, "%d, %d, %d, %d, %d, \n", b->nodes[i].id, b->nodes[i].id_left, b->nodes[i].id_right, b->nodes[i].max_cnt2_label, b->nodes[i].leaf);
		}	

		fclose(fp);
	}
	
	void load_tree(txm* b)
	{
		FILE *fp;
		size_t i;
		size_t tree_size;
		
		fp = fopen(b->tree_file_name, "rt");
		
		if(fp == NULL)
		{
			cout << "Cannot open file: " << b->tree_file_name << endl;
			return;
		}
		
		fscanf(fp, "%ld\n", &tree_size);
		printf("%ld\n", tree_size);
		
		for(i = 0; i < tree_size; i++)
		{
			b->nodes.push_back(init_node(i, 0, 0));
			fscanf(fp,"%ld, %ld, %ld, %d, %d, \n", &b->nodes[i].id, &b->nodes[i].id_left, &b->nodes[i].id_right, &b->nodes[i].max_cnt2_label, &b->nodes[i].leaf);	
			//printf("%ld, %ld, %ld, %d, %d, \n", b->nodes[i].id, b->nodes[i].id_left, b->nodes[i].id_right, b->nodes[i].max_cnt2_label, b->nodes[i].leaf);		
			//cin.ignore();
		}	

		fclose(fp);
	}*/

	void predict(txm* d, example* ec)
	{
		OAA::mc_label *mc = (OAA::mc_label*)ec->ld;
		vw* all = d->all;
		size_t new_cn;
		//float Eh_norm;
		
		#ifdef TXM_DEBUG_PRED
		int level = 0;
		#endif
		
		if(command_example(all,ec))
		{	
			d->base.learn(ec);
			return;
		}
		
		d->cn = 0;
		
		#ifdef TXM_DEBUG_PRED
		cout << "\nExample: " << d->ex_num << endl;
		#endif
		
		while(!d->nodes[d->cn].leaf)// && d->nodes[d->cn].level < d->level_limit)
		{
			update_example_indicies(all->audit, ec, d->increment * d->cn);	
		
			ec->test_only = true;
			d->base.learn(ec);
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			#ifdef TXM_DEBUG_PRED
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
		}
		
		#ifdef TXM_DEBUG_PRED
		cout << "Prediction finished...\n";
		#endif
		
		#ifdef TXM_DEBUG_PRED
		cout << "Nb of labels in leaf: " << d->nodes[d->cn].node_pred.size() << endl;
		#endif
		
		ec->final_prediction = d->nodes[d->cn].max_cnt2_label;		//PRZYPISYWANIE PREDYKCJI DO PRZYKLADU (necessary for external evaluation)
		
		/*if(d->cn == 0)
		{
			ec->final_prediction = -1;
			cout << "Prediction error: " << err_cnt++ << endl;
 		}*/
		
		#ifdef TXM_DEBUG_PRED
		cout << "labels O:P:\t" << mc->label << ":" << ec->final_prediction << endl;
		#endif
		
		d->ex_num++;					//pozostalosc po starym kodzie
	}	
	
	uint32_t predict_node(txm* d, example* ec, size_t level_limit)		//to samo co predict tylko nie to samo do konca, minimalny argument z jakim to jest odpalane to jest 1 (czyli masz root i dwa liscie)
	{
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
			d->base.learn(ec);
			
			update_example_indicies(all->audit, ec, -d->increment * d->cn);
			
			#ifdef TXM_DEBUG_PRED
			cout << "level: " << level << endl;
			cout << "node: " << d->cn << endl;
			#endif
			
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
			
			level++;
			
			if(d->cn == 0)								//zabezpieczenie, nigdy nie jest wywolywane
			{
				//cout << "Node prediction error!!\n";
				break;
			}
		}		
			
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
		size_t j;
		
		#ifdef TXM_DEBUG
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
			
				if(b->only_leafs)
				{
					b->tree_finished = true;	
				
					for(j = 0; j < b->nodes.size(); j++)
					{
						if(b->nodes[j].level == b->level_limit)
							b->nodes[j].leaf = true;
					}
					
					//save_tree(b);
					
					return;
				}
				else
				{
					b->only_leafs = true;
				}
				
				//display_tree(b);	
				
				b->current_pass++;
				//display_node_stats(b);
				#ifdef TXM_DEBUG_PASS_STOP
				cin.ignore();
				#endif
			}	
			return;
		}		
		
		if(b->current_pass == 0)			//first pass through the data, po przejsciu przez ten pass mam root i dwa liscie
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
		else								//not the first pass through the data, jezeli b->current_pass == 1 to tworze drugi level drzewa, czyli trenuje regresory pierwszego levela
		{
			b->cn = predict_node(b, ec, b->current_pass);		//current node, ktory ja zamierzam trenowac
			
			if(b->nodes[b->cn].leaf || b->cn == 0)   //leaf lub blad, wychodze z learn i dalej bede analizowac kolejny przyklad kolejny przyklad
			{
				b->ex_num++;
				return;
			}			
			
			if(b->nodes[b->cn].id_parent > 0)		//jedno kryterium stopu, warunek mowi ze nie chce z roota robic liscia, parent jest juz wytrenowany a b->cn dopiero jest w trakcie trenowania, trzeba sprawdzic czy czasem parent juz nie jest lisciem
			{										//minimalne drzewo jakie bede miec to root i dwa liscie
				id_parent = b->nodes[b->cn].id_parent;
				
				ftmp = b->nodes[id_parent].max_cnt2;
				ftmp /= b->nodes[id_parent].total_cnt2;
				
				if(ftmp > TXM_LEAF_TH)
				{
					b->nodes[b->cn].leaf = true;	//nie ma znaczenia, mozna to wyrzucic, pozostalosc po starym kodzie
					b->nodes[id_parent].leaf = true;
					
					id_left = b->nodes[id_parent].id_left;
					id_right = b->nodes[id_parent].id_right;
					
					if(id_left)
						b->nodes[id_left].removed = true;
					
					if(id_right)
						b->nodes[id_right].removed = true;
						
					//cout << "\n\nLEAF!!:\t" << id_parent << ":" << b->nodes[id_parent].max_cnt2_label << endl << endl;
					
					return;
				}				
			}
			
			if(b->nodes[b->cn].node_pred.contain_sorted(txm_node_pred_type(oryginal_label), &index))		//jezeli label jest w tablicy node_pred'ow 
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
				
				if(b->nodes[b->cn].node_pred[index].label_cnt2 > b->nodes[b->cn].max_cnt2)
				{
					b->nodes[b->cn].max_cnt2 = b->nodes[b->cn].node_pred[index].label_cnt2;
					b->nodes[b->cn].max_cnt2_label = b->nodes[b->cn].node_pred[index].label;
				}
			}
			
			b->nodes[b->cn].total_cnt2++;
			
			if(b->nodes[b->cn].level >= b->level_limit)	//to jest tylko zwiazane z tymi nodami ktore doszly do poziomu level limit
				return;							//nie ustawiam flagi b->nodes[b->cn].leaf na true, bo chce zeby on przeszedl przez liczenie statystyki dla wszystkich punktow ktore tu moga wpasc
			
			b->only_leafs = false;				//jezeli mam ostatni level drzewa to tu nigdy nie dojde i po ostatnim przykladzie z tego passa po danych wejde w tree_finished
		}
		
		#ifdef TXM_DEBUG
		cout << "Current node: " << b->cn << endl;
		cout << "Example number: " << b->ex_num << endl;
		#endif		
			
		//do the initial prediction to decide if going left or right
		all->sd->min_label = -TXM_PRED_LIM;
		all->sd->max_label = TXM_PRED_LIM;	
		mc->label = 0;	//jezeli jest cos innego tu to on moze zmieniac clipping zakresy
		update_example_indicies(all->audit, ec, b->increment * b->cn);
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
		txm* b = (txm*)d;	
		example* ec = NULL;	
		
		while ( !b->tree_finished )
		{
			if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
			{
				learn((txm*)d, ec);
				
				if(!all->training)
				{
					if (!command_example(all, ec))
						OAA::output_example(*all, ec);
				}
				
				VW::finish_example(*all, ec);				
			}
			else if (parser_done(all->p))
			{				
				return;
			}
		}		
		
		//display_node_stats(b);
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
	
	void txm_save_load_regressor(vw& all, io_buf& model_file, bool read, bool text)
	{
		uint32_t length = 1 << all.num_bits;
		uint32_t stride = all.reg.stride;
		int c = 0;
		uint32_t i = 0;
		size_t brw = 1;
		char ch = 123;

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
		txm* b = (txm*)all.l.sl.sldata;
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
				
				//printf("%ld, %ld, %ld, %d, %d, \n", b->nodes[j].id, b->nodes[j].id_left, b->nodes[j].id_right, b->nodes[j].max_cnt2_label, b->nodes[j].leaf);
			}
		}
		else
		{
			text_len = sprintf(buff, ":%d\n", b->nodes.size());
			v = b->nodes.size();
			brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);

			for(i = 0; i < b->nodes.size(); i++)
			{
				text_len = sprintf(buff, ":%d", b->nodes[i].id);
				v = b->nodes[i].id;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				text_len = sprintf(buff, ":%d", b->nodes[i].id_left);
				v = b->nodes[i].id_left;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				text_len = sprintf(buff, ":%d", b->nodes[i].id_right);
				v = b->nodes[i].id_right;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				text_len = sprintf(buff, ":%d", b->nodes[i].max_cnt2_label);
				v = b->nodes[i].max_cnt2_label;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);
				
				text_len = sprintf(buff, ":%d\n", b->nodes[i].leaf);
				v = b->nodes[i].leaf;
				brw = bin_text_write_fixed(model_file,(char *)&v, sizeof (v), buff, text_len, text);			
			}	
		}		
	}
	
	void txm_save_load(void* data, io_buf& model_file, bool read, bool text)
	{
		txm* g = (txm*)data;			//BARDZO WAZNE JEST ZOSTAWIENIE TEGO g, W PRZECIWNYM WYPADKU ON SIE ODWOLUJE DO TEGO CZEGO NIE MA
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
			txm_save_load_regressor(*all, model_file, read, text);
			save_load_tree(*all, model_file, read, text);
		}
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
		all.weights_per_problem *= TXM_MULTIPLICATIVE_FACTOR*data->k;
		data->base = all.l;
		//TUTAJ ZAMIAST all.l.sl BEDE MIALA WLASNA STRUKTURE DO ZAPAMIETANIA, powinnam odkomentowac te dwie linijki i zakomentowac linijke z learner
	    sl_t sl = {data, txm_save_load};					//TU JEST MOJA WLASNA STRUKTURA DO ZAPAMIETANIA
	    learner l(data, drive, learn, finish, sl);		//CZYLI TUTAJ ZAMIAST all.l.sl uzywamy po prostu sl
		//learner l(data, drive, learn, finish, all.l.sl);
		
		txm* b = (txm*)data;
		
		b->current_pass = 0;
		b->level_limit = TXM_LEVEL_LIM;
		
		if(all.training)
			init_tree(b);		
		else	
		{			
			b->tree_finished = false;	
		}
		
		return l;
	}	
}


