/* 

Will Dabney

Largely based upon Brain Tanner's SampleSarsaAgent for RL-Glue, and the library.cc example in Vowpel Wabbit.
Differences from the SampleSarsaAgent for RL-Glue is that we completely replace the value function learning with calls 
to the VW library, specifically the off-policy reinforcement learning reduction (oprl). 

The Fourier Basis computation is a port from George Konidaris' Java implementation.

*/


#include <stdio.h>  /* for sprintf */
#include <stdlib.h> /* for strtod */
#include "../vowpalwabbit/simple_label.h"
#include "../vowpalwabbit/v_array.h"

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

// Create VW input features for use with vw library
inline feature vw_feature_from_string(vw& v, string fstr, unsigned long seed, float val)
{
  uint32_t foo = VW::hash_feature(v, fstr, seed);
  feature f = { val, foo};
  return f;
}


// Need the constant PI for computing fourier basis
const double pi = 3.141592653589793;//acos(-1);

// RL constants, 
double sarsa_epsilon = 0.0;

// Create the VW instance and initialize it
//vw vw = VW::initialize("--learning_rate 10.0 --oprl 0.0 --gamma 1.0");
vw vw = VW::initialize("--learning_rate 0.001 --noconstant --oprl 0.9 --gamma 1.0");
example *start_data = VW::read_example(vw, " \'rl_start |a p^the_man w^the w^man");
example *end_data = VW::read_example(vw, " \'rl_end |a p^the_man w^the w^man");

// Variables for the fourier basis functions
int numTerms = 0;
int numVars = 0;
int Order = 0;
vector<vector<double> > multipliers;
vector<vector<double> > obsRanges;


/********** GDK's Fourier Code (Ported from Java) **************/
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

/******************* VW Library Based Calls ********************/

// Convert action int into a character for use as a namespace in VW
string action_to_name(int action) {
  char c = action + 97;
  return string(1, c);
}

// Send features to vw. Used for querying the value function and actual training
double query_vw_value(vector<double> features, int action, int N, double label, double importance, bool start_episode){
  vector< VW::feature_space > ec_info;
  vector<feature> s_features;

  // Determin hash space based upon the action
  string hash_name = action_to_name(action);
  uint32_t hash = VW::hash_space(vw, hash_name);
  // Add features to the hash space
  if(features.size() > 0) {
    for (int i=0; i<numTerms; i++) {
      string ident = boost::lexical_cast<string>( i );
      s_features.push_back( vw_feature_from_string(vw, ident, hash, features[i]) );
    }
  }

  ec_info.push_back( VW::feature_space(hash_name[0], s_features) );
  example* vec3 = VW::import_example(vw, ec_info);
  if (start_episode) {
    copy_array(vec3->tag, start_data->tag);
  } else if(features.size() == 0) {
    copy_array(vec3->tag, end_data->tag);
  }

  // Only train if the importance is non-zero
  /*  if (importance > 0.0) {
    cerr << "Action: " << action << " " << hash_name << endl;
    }*/
    default_simple_label(vec3->ld);
    ((label_data*)vec3->ld)->label = label;
    ((label_data*)vec3->ld)->weight = importance;
    //  }
  vw.learn(&vw, vec3);
  double value = vec3->final_prediction;
  VW::finish_example(vw, vec3);  
  return value;
}


// RL variables
action_t this_action;
action_t last_action;
observation_t *last_observation=0;
int numActions=0;
int numStates=0;
int policy_frozen=0;
int exploring_frozen=0;
bool newEpisode = true;

/************** RL-Glue Code ****************/
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

	numStates=getNumDoubleObs(ts);//getIntObsMax(ts,0)+1;

	// Setup observation range values for use with Fourier basis
	obsRanges.resize(numStates);//(numStates, vector<int>(2, 0));
	for (int i = 0; i<numStates; i++) {
	  obsRanges[i].resize(2);
	  obsRanges[i][0] = getDoubleObsMin(ts,i);
	  obsRanges[i][1] = getDoubleObsMax(ts,i);
	}

	// Compute the fourier coefficients
	computeFourierCoefficients(numStates, 3);

	assert(getNumIntAct(ts)==1);
	assert(getNumDoubleAct(ts)==0);
	assert(isIntActMax_special(ts,0)==0);
	assert(isIntActMin_special(ts,0)==0);

	numActions=getIntActMax(ts,0)+1;

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
	
}

const action_t *agent_start(const observation_t *this_observation) {
        newEpisode = true;        
	int theIntAction=egreedy(this_observation->doubleArray);
	this_action.intArray[0]=theIntAction;
	if(!policy_frozen) {
	  double Q_sa = query_vw_value(computeFeatures(this_observation->doubleArray), theIntAction, numStates, 0.0, 1.0, true);
	  for(int i=0; i<numStates; i++)
	    cout << "," << this_observation->doubleArray[i];
	  cout << endl;
	}
	replaceRLStruct(&this_action, &last_action);
	replaceRLStruct(this_observation, last_observation);
	
	return &this_action;
}

const action_t *agent_step(double reward, const observation_t *this_observation) {
	int lastAction=last_action.intArray[0];
	int newAction=egreedy(this_observation->doubleArray);
	// All of the RL is foisted onto VW
	if(!policy_frozen){
	  double Q_sa = query_vw_value(computeFeatures(this_observation->doubleArray), newAction, numStates, reward, 1.0, false);
	    cout << Q_sa;
	    for(int i=0; i<numStates; i++)
	      cout << "," << this_observation->doubleArray[i];
	    cout << endl;
	}
	newEpisode = false;
	this_action.intArray[0]=newAction;
	replaceRLStruct(&this_action, &last_action);
	replaceRLStruct(this_observation, last_observation);
	
	return &this_action;
}


void agent_end(double reward) {
	int lastAction=last_action.intArray[0];
	
	if(!policy_frozen){
	  query_vw_value(vector<double>(0), 0, numStates, reward,1.0, false);	  
	}
	newEpisode=true;
	clearRLStruct(&last_action);
	clearRLStruct(last_observation);
}

void agent_cleanup() {
	clearRLStruct(&this_action);
	clearRLStruct(&last_action);
	freeRLStructPointer(last_observation);
}

const char* agent_message(const char* _inMessage) {
	string buffer;
  string inMessage = _inMessage;
	
	/*	Message Description
 	 * 'freeze learning'
	 * Action: Set flag to stop updating policy
	 */
	if(inMessage == "freeze learning"){
	  //		policy_frozen=1;
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

}

void load_value_function(const char *fileName){
}

int egreedy(double features[]){
	int maxIndex = 0;
	int a = 1;

	if(!exploring_frozen){
	  if((rand() / double(RAND_MAX) < sarsa_epsilon)) {
	    return randInRange(numActions-1);
  		}
	}
  maxIndex = 0;
  double maxvalue = query_vw_value(computeFeatures(features), 0, numStates, 0.0, 0.0, false);
  for(a = 1; a < numActions; a++){
    double value_a = query_vw_value(computeFeatures(features), a, numStates, 0.0, 0.0, false);
    if(value_a > maxvalue)
      maxIndex = a;
    else if (value_a == maxvalue && (rand() / double(RAND_MAX) < 0.5))
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
