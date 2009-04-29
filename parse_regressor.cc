/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <fstream.h>
#include "parse_regressor.h"

void initialize_regressor(regressor &r)
{
  r.length = 1 << r.numbits;
  if (r.seg)
    {
      r.weights = (weight *)malloc(r.length * sizeof(weight));
      weight* end = r.weights+r.length;
      for (weight *v = r.weights; v != end; v++)
	*v = 1.;
      r.other_weights = (weight *)malloc(r.length * sizeof(weight));
      end = r.other_weights + r.length;
      for (weight *v = r.other_weights; v != end; v++)
	*v = 1.;
    }
  else
    r.weights = (weight *)calloc(r.length, sizeof(weight));
}

/* 
   Read in regressors.  If multiple regressors are specified, do a weighted 
   average.  If none are specified, initialize according to global_seg & 
   numbits.
*/
void parse_regressor(vector<string> &regressors, regressor &r)
{
  bool initialized = false;
  
  for (size_t i = 0; i < regressors.size(); i++)
    {
      ifstream regressor(regressors[i].c_str());
      bool seg;
      regressor.read((char *)&seg, sizeof(seg));
      if (!initialized)
	r.seg = seg;
      else 
	if (seg != r.seg)
	  {
	    cout << "can't combine regressors from seg and gd!" << endl;
	    exit (1);
	  }
      size_t local_numbits;
      regressor.read((char *)&local_numbits, sizeof(local_numbits));
      if (!initialized){
	r.numbits = local_numbits;
      }
      else 
	if (local_numbits != r.numbits)
	  {
	    cout << "can't combine regressors with different feature number!" << endl;
	    exit (1);
	  }
      int len;
      regressor.read((char *)&len, sizeof(len));
      
      vector<string> local_pairs;
      for (; len > 0; len--)
	{
	  char pair[2];
	  regressor.read(pair, sizeof(char)*2);
	  string temp(pair, 2);
	  local_pairs.push_back(temp);
	}
      if (!initialized)
	{
	  r.pairs = local_pairs;
	  initialize_regressor(r);
	  initialized = true;
	}
      else
	if (local_pairs != r.pairs)
	  {
	    cout << "can't combine regressors with different features!" << endl;
	    for (size_t i = 0; i < local_pairs.size(); i++)
	      cout << local_pairs[i] << " " << local_pairs[i].size() << " ";
	    cout << endl;
	    for (size_t i = 0; i < r.pairs.size(); i++)
	      cout << r.pairs[i] << " " << r.pairs[i].size() << " ";
	    cout << endl;
	    exit (1);
	  }

      if (!seg)
	while (regressor.good())
	  {
	    size_t hash;
	    regressor.read((char *)&hash, sizeof(hash));
	    weight w = 0.;
	    regressor.read((char *)&w, sizeof(float));
	    if (regressor.good()) 
	      r.weights[hash] = r.weights[hash] + w;
	  }
      else
	{
	  while (regressor.good())
	    {
	      size_t hash;
	      regressor.read((char *)&hash, sizeof(hash));
	      weight first = 0.;
	      regressor.read((char *)&first, sizeof(float));
	      weight second = 0.;
	      regressor.read((char *)&second, sizeof(float));
	      if (regressor.good()) {
		r.weights[hash] = first;
		r.other_weights[hash] = second;
	      }
	    }
	}
      regressor.close();
    }

  if (!initialized)
    initialize_regressor(r);
}

void dump_regressor(ofstream &o, regressor &r)
{
  if (o.is_open()) 
    {
      o.write((char *)&r.seg, sizeof(r.seg));
      o.write((char *)&r.numbits, sizeof(r.numbits));
      int len = r.pairs.size();
      o.write((char *)&len, sizeof(len));
      for (vector<string>::iterator i = r.pairs.begin(); i != r.pairs.end();i++) 
	o << (*i)[0] << (*i)[1];
      
      if (!r.seg) 
	{
	  for(weight* v = r.weights; v != r.weights+r.length; v++)
	    if (*v != 0.)
	      {      
		size_t dist = v - r.weights;
		o.write((char *)&(dist), sizeof (dist));
		o.write((char *)v, sizeof (*v));
	      }
	}
      else 
	{
	  for(weight* v = r.weights; v != r.weights+r.length; v++)
	    if (*v != 1.)
	      {      
		size_t dist = v - r.weights;
		o.write((char *)&(dist), sizeof (dist));
		o.write((char *)v, sizeof (*v));
		o.write((char *)&r.other_weights[dist], sizeof (r.other_weights[dist]));
	      }
	}
    }
  if (r.seg)
    free(r.other_weights);

  free(r.weights);

  o.close();
}

