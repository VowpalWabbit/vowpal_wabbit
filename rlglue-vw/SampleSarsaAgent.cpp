/* 
	Copyright (C) 2008, Brian Tanner

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	    http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

	
	*  $Revision: 996 $
	*  $Date: 2009-02-08 20:48:32 -0500 (Sun, 08 Feb 2009) $
	*  $Author: brian@tannerpages.com $
	*  $HeadURL: https://rl-library.googlecode.com/svn/trunk/projects/packages/examples/mines-sarsa-c/SampleSarsaAgent.c $
	
*/

/* 	
	This is a very simple Sarsa agent for discrete-action, discrete-state
	environments.  It uses epsilon-greedy exploration.
	
	We've made a decision to store the previous action and observation in 
	their raw form, as structures.  This code could be simplified and you
	could store them just as ints.
*/


#include <stdio.h>  /* for sprintf */
#include <stdlib.h> /* for strtod */
#include "../vowpalwabbit/simple_label.h"

#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>

#include <boost/lexical_cast.hpp>

#include <rlglue/Agent_common.h> /* agent_ functions and RL-Glue types */
#include <rlglue/utils/C/RLStruct_util.h> /* helpful functions for structs */
#include <rlglue/utils/C/TaskSpec_Parser.h> /* task spec parser */

#include "../vowpalwabbit/vw.h"


using namespace std; 

const float pi = acos(-1);
double sarsa_stepsize = 0.1;
double sarsa_epsilon = 0.1;
double sarsa_gamma = 1.0;
double sarsa_lambda = 0.0;
inline feature vw_feature_from_string(vw& v, string fstr, unsigned long seed, float val)
{
  uint32_t foo = VW::hash_feature(v, fstr, seed);
  feature f = { val, foo};
  return f;
}

vw vw = VW::initialize("--hash all --adaptive --learning_rate 10.0 --oprl 1.0");

int numTerms = 0;
int numVars = 0;
int Order = 0;
vector<vector<double> > multipliers;

/**
 * This method iterates through a coefficient vector
 * up to a given degree. (like counting in a base of
 * that degree).
 * 
 * @param cthe coefficient vector.
 * @param NVariablesthe number of variables in c.
 * @param Degreethe degree up to which to increment.
 */
void Iterate(int *c, int NVariables, int Degree)
{
  c[NVariables - 1] = c[NVariables-1] + 1;
  
  if(c[NVariables - 1] > Degree)
    {
      if(NVariables > 1)
	{
	  c[NVariables - 1]  = 0;
	  Iterate(c, NVariables - 1, Degree);
	}
    }
}

/*
 * Compute the full Fourier Basis coefficient matrix for
 * a given number of variables up to a given order.
 * 
 * @param nvarsthe number of variables.
 * @param orderthe highest coefficient value for any individual variable.
 * @returna two dimensional array of doubles. The first dimension length is
 * the number of basis functions, and the second is the number of state variables.
 */
void computeFourierCoefficients(int nvars, int order) {
  int nterms = (int)pow(order + 1.0, nvars);
  numTerms = nterms;
  numVars = nvars;
  Order = order;

  int pos = 0;
  multipliers.resize(nterms);
  for (int i=0; i<nterms; i++)
    multipliers[i].resize(nvars);

  int *c = new int[nvars];
  for(int j = 0; j < nvars; j++) 
    c[j] = 0;
    
  
  do
    {
      for(int k = 0; k < nvars; k++)
	{
	  multipliers[pos][k] = c[k];
	}
      
      pos++;
      
      // Iterate c
      Iterate(c, nvars, order);
    }
  while(c[0] <= order);
}

vector<vector<double> > obsRanges;

/**
 * Scale a state variable to between 0 and 1.
 * (this is required for the Fourier Basis).
 * 
 * @param valthe state variable.
 * @param posthe state variable number.
 * @returnthe normalized state variable.
 */
double scale(double val, int pos)
{
  return (val - obsRanges[pos][0]) / (obsRanges[pos][1] - obsRanges[pos][0]);
}

/**
 * Compute the feature vector for a given state.
 * This is achieved by evaluating each Fourier Basis function
 * at that state.
 * 
 * @param sthe state in question.
 * @return a vector of doubles representing each basis function evaluated at s.
 */
vector<double> computeFeatures(double features[])
{
  vector<double> phi(numTerms);
  for(int pos = 0; pos < numTerms; pos++)
    {
      double dsum = 0;
      
      for(int j = 0; j < numVars; j++)
	{
	  double sval = scale(features[j], j);	  
	  dsum += sval*multipliers[pos][j];
	}
      
      phi[pos] = cos((pi) * dsum);
    }
  
  return phi;
}


vector<vector<double> > traces;

string action_to_name(int action) {
  char c = action + 97;
  return string(1, c);
}

double query_vw_value(vector<double> features, int action, int N, double label, double importance){
  //string s = boost::lexical_cast<string>( number );
  vector< VW::feature_space > ec_info;
  vector<feature> s_features;

  // Determin hash space based upon the action
  string hash_name = action_to_name(action);
  uint32_t hash = VW::hash_space(vw, hash_name);
  // Add features to the hash space
  for (int i=0; i<numTerms; i++) {
    string ident = boost::lexical_cast<string>( i );
    s_features.push_back( vw_feature_from_string(vw, ident, hash, features[i]) );
  }

  ec_info.push_back( VW::feature_space(hash_name[0], s_features) );
  example* vec3 = VW::import_example(vw, ec_info);


if (importance > 0.0) {
    //  example *vec2 = VW::read_example(vw, "1.23 |s p^the_man w^the w^man |t p^le_homme w^le w^homme");
    //  label_data* ld = (label_data*)vec2->ld;
    //  cout << "Label: " << ld->label<<endl;
    //  ld->label = 2.123;
    //  cout << "Label: " << ld->label<<endl;
    default_simple_label(vec3->ld);
    ((label_data*)vec3->ld)->label = label;
    ((label_data*)vec3->ld)->weight = importance;
    //    label_data* ld2 = (label_data*)vec3->ld;
    //    cout << "... Label: " << ld2->label<<endl;
    //    cout << label << "|" << hash_name;
    //    for (int i=0; i<numTerms; i++)
    //          cerr << " " << i << ":" << features[i];
    //    cerr << endl;
  }
  
  vw.learn(&vw, vec3);
  double value = vec3->final_prediction;
  VW::finish_example(vw, vec3);
  return value;
}


action_t this_action;
action_t last_action;

observation_t *last_observation=0;

double* value_function=0;
int numActions=0;
int numStates=0;

int policy_frozen=0;
int exploring_frozen=0;



void update_traces(int last_action) {
        vector<double> feats = computeFeatures(last_observation->doubleArray);
	for (int i=0; i<numTerms; i++) {
	  for (int a=0; a< numActions; a++)
	    traces[a][i] *= sarsa_lambda*sarsa_gamma;
	  traces[last_action][i] += feats[i];
	}
}


/* Returns a random integer in [0,max] */
int randInRange(int max);
/* 
 *	Selects a random action with probability 1-sarsa_epsilon, 
 *	and the action with the highest value otherwise.  This is a 
 *	quick'n'dirty implementation, it does not do tie-breaking or
 *	even use a good method of random generation.
*/
int egreedy(double features[]);
int calculateArrayIndex(int theState, int theAction);
void save_value_function(const char *fileName);
void load_value_function(const char *fileName);

void agent_init(const char* task_spec)
{
        srand ( time(NULL) );
	/*Struct to hold the parsed task spec*/
	taskspec_t *ts= new taskspec_t; 
	int decode_result = decode_taskspec( ts, task_spec );
	if(decode_result!=0){
		cerr << "Could not decode task spec, code: " << decode_result
         << "for task spec: " << task_spec << endl; 
		exit(1);
	}
	
	// Lots of assertions to make sure that we can handle this problem.  
	assert(getNumIntObs(ts)==0);
	assert(getNumDoubleObs(ts)>0);
	//	assert(isIntObsMax_special(ts,0)==0);
	//	assert(isIntObsMin_special(ts,0)==0);

	numStates=getNumDoubleObs(ts);//getIntObsMax(ts,0)+1;

	obsRanges.resize(numStates);//(numStates, vector<int>(2, 0));
	for (int i = 0; i<numStates; i++) {
	  obsRanges[i].resize(2);
	  obsRanges[i][0] = getDoubleObsMin(ts,i);
	  obsRanges[i][1] = getDoubleObsMax(ts,i);
	}

	computeFourierCoefficients(numStates, 3);

	assert(getNumIntAct(ts)==1);
	assert(getNumDoubleAct(ts)==0);
	assert(isIntActMax_special(ts,0)==0);
	assert(isIntActMin_special(ts,0)==0);

	numActions=getIntActMax(ts,0)+1;
	traces.resize(numActions);
	for (int i =0 ; i<numActions; i++)
	  traces[i].resize(numTerms);
	// sanity check: vectors of doubles initialize to zero..?

	free_taskspec_struct(ts); // Make the taskspec struct a "blank slate" 

	delete ts; // Free the structure itself 
	//Here is where you might allocate storage for parameters (value function or
  // policy,last action, last observation, etc)
	
  //*Here you would parse the task spec if you felt like it
	
	//Allocate memory for a one-dimensional integer action using utility functions from RLStruct_util
	allocateRLStruct(&this_action,1,0,0);
	allocateRLStruct(&last_action,1,0,0);
	/* That is equivalent to:
			 this_action.numInts     =  1;
			 this_action.intArray    = (int*)calloc(1,sizeof(int));
			 this_action.numDoubles  = 0;
			 this_action.doubleArray = 0;
			 this_action.numChars    = 0;
			 this_action.charArray   = 0;
	*/

	//Allocate memory for a one-dimensional integer observation using utility functions from RLStruct_util
	last_observation=allocateRLStructPointer(1,0,0);
	
	//Later we will parse this from the task spec, but for now
	value_function= new double[numActions*numStates];
  for (int i = 0; i < numActions*numStates; i++)
    value_function[i] = 0; 
}

const action_t *agent_start(const observation_t *this_observation) {
        
	int theIntAction=egreedy(this_observation->doubleArray);
	this_action.intArray[0]=theIntAction;

	replaceRLStruct(&this_action, &last_action);
	replaceRLStruct(this_observation, last_observation);
	
	return &this_action;
}

const action_t *agent_step(double reward, const observation_t *this_observation) {
  //	int newState=this_observation->intArray[0];
  //	int lastState=last_observation->intArray[0];
	int lastAction=last_action.intArray[0];
	// Testing vw
	//	query_vw_value(value_function, 1, numActions*numStates, 1.0, true);

	int newAction=egreedy(this_observation->doubleArray);
	
	//	double Q_sa=query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, 0.0, 0.0);//value_function[calculateArrayIndex(lastState,lastAction)];
	//	double Q_sprime_aprime=query_vw_value(computeFeatures(this_observation->doubleArray), newAction, numStates, 0.0, 0.0);//value_function[calculateArrayIndex(newState,newAction)];
	
	//	double delta = (reward + sarsa_gamma * Q_sprime_aprime - Q_sa);
	/*	Only update the value function if the policy is not frozen */
	//	if(!policy_frozen){
	// If NOT using TRACES
	//	query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, reward + sarsa_gamma * Q_sprime_aprime, 1.0);	  
	// If USING TRACES
	//	double p = query_vw_value(traces[lastAction], lastAction, numStates, 0.0, 0.0);
	//	query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, reward + sarsa_gamma * Q_sprime_aprime, 1.0);	  
	//	query_vw_value(traces[lastAction], lastAction, numStates, p + delta, sarsa_gamma*sarsa_lambda);
	if(!policy_frozen){
	  double Q_sa = query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, reward, 1.0);
	//	update_traces(lastAction);

	    cout << Q_sa;
	    for(int i=0; i<numStates; i++)
	      cout << "," << last_observation->doubleArray[i];
	    cout << endl;
	}
	  //		value_function[calculateArrayIndex(lastState,lastAction)]=new_Q_sa;
	  //	}

	this_action.intArray[0]=newAction;
	replaceRLStruct(&this_action, &last_action);
	replaceRLStruct(this_observation, last_observation);
	
	return &this_action;
}


void agent_end(double reward) {
  //	int lastState=last_observation->intArray[0];
	int lastAction=last_action.intArray[0];
	
	//	double Q_sa=query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, 0.0, 0.0);//value_function[calculateArrayIndex(lastState,lastAction)];
	//	double new_Q_sa=Q_sa + sarsa_stepsize * (reward - Q_sa);

	/*	Only update the value function if the policy is not frozen */
	if(!policy_frozen){
	// If NOT using TRACES
	  //	query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, reward,1.0);	  
	// If USING TRACES
	  //	double delta = reward - Q_sa;
	  //	double p = query_vw_value(traces[lastAction], lastAction, numStates, 0.0, 0.0);
	  //	query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, reward,1.0);	  
	  //	query_vw_value(traces[lastAction], lastAction, numStates, p + delta, sarsa_gamma*sarsa_lambda);
	  //	update_traces(lastAction);
	  query_vw_value(computeFeatures(last_observation->doubleArray), lastAction, numStates, reward,1.0);	  
	}

	clearRLStruct(&last_action);
	clearRLStruct(last_observation);
}

void agent_cleanup() {
	clearRLStruct(&this_action);
	clearRLStruct(&last_action);
	freeRLStructPointer(last_observation);
	
	if(value_function!=0){
		delete [] value_function;
		value_function=0;
	}
}

const char* agent_message(const char* _inMessage) {
	string buffer;
  string inMessage = _inMessage;
	
	/*	Message Description
 	 * 'freeze learning'
	 * Action: Set flag to stop updating policy
	 */
	if(inMessage == "freeze learning"){
		policy_frozen=1;
		return "message understood, policy frozen";
	}
	/*	Message Description
 	* unfreeze learning
 	* Action: Set flag to resume updating policy
	*/
	if(inMessage == "unfreeze learning"){
		policy_frozen=0;
		return "message understood, policy unfrozen";
	}
	/*Message Description
 	* freeze exploring
 	* Action: Set flag to stop exploring (greedy actions only)
	*/
	if(inMessage == "freeze exploring"){
		exploring_frozen=1;
		return "message understood, exploring frozen";
	}
	/*Message Description
 	* unfreeze exploring
 	* Action: Set flag to resume exploring (e-greedy actions)
	*/
	if(inMessage == "unfreeze exploring"){
		exploring_frozen=0;
		return "message understood, exploring unfrozen";
	}
	/*Message Description
 	* save_policy FILENAME
 	* Action: Save current value function in binary format to 
	* file called FILENAME
	*/
	if(inMessage.substr(0,11) == "save_policy"){
    buffer = inMessage.substr(12);
		cout << "Saving value function..."; 
		save_value_function(buffer.c_str());
		cout << "Saved." << endl; 
		return "message understood, saving policy";
	}
	/*Message Description
 	* load_policy FILENAME
 	* Action: Load value function in binary format from 
	* file called FILENAME
	*/
	if(inMessage.substr(0,11) == "load_policy"){
    buffer = inMessage.substr(12); 
		cout << "Loading value function...";
		load_value_function(buffer.c_str());
		cout << "Loaded." << endl;
		return "message understood, loading policy";
	}

	
	return "SampleSarsaAgent(C/C++) does not understand your message.";
			
}

void save_value_function(const char *fileName){
  ofstream out(fileName, ios_base::binary);

	out.write(reinterpret_cast<const char *>(value_function),
            sizeof(double)*numStates*numActions);
	out.close();
}

void load_value_function(const char *fileName){
	ifstream in(fileName, ios_base::binary); 
	
	in.read(reinterpret_cast<char *>(value_function),
          sizeof(double)*numStates*numActions);
	in.close(); 
}

int egreedy(double features[]){
	int maxIndex = 0;
	int a = 1;
	int randFrequency=(int)(1.0f/sarsa_epsilon);

	if(!exploring_frozen){
  		if((rand() % randFrequency == 1)) {
    		return randInRange(numActions-1);
  		}
	}
  maxIndex = 0;
  double maxvalue = query_vw_value(computeFeatures(features), 0, numStates, 0.0, 0.0);
  for(a = 1; a < numActions; a++){
    double value_a = query_vw_value(computeFeatures(features), a, numStates, 0.0, 0.0);
    if(value_a > maxvalue)
      maxIndex = a;
  }
  return maxIndex;
}

int randInRange(int max){
	double r, x;
	r = ((double)rand() / ((double)(RAND_MAX)+(double)(1)));
   	x = (r * (max+1));

	return (int)x;
}

int calculateArrayIndex(int theState, int theAction){
	assert(theState<numStates);
	assert(theAction<numActions);
	
	return theState*numActions+theAction;
}
