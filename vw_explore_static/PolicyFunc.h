#pragma once

#include "utility.h"
#include "Interaction.h"

class BaseFunctionWrapper
{ 
public:
	virtual ~BaseFunctionWrapper() { }
};

class MWT_Empty { };

typedef u32 Stateful_Policy_Func(void* state_Context, void* application_context);
typedef u32 Stateless_Policy_Func(void* application_context);
//TODO: I think vectors will cause problem with C# interop
typedef void Stateful_Scorer_Func(void* state_context, void* application_context, float*& scores, size_t& num_scores);
typedef void Stateless_Scorer_Func(void* application_context, float*& scores, size_t& num_scores);


template <class T>
class StatefulFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef u32 Policy_Func(T* state_context, Context* application_context);
	typedef void Scorer_Func(T* state_context, Context* application_context, float*& scores, size_t& num_scores);

	Stateful_Policy_Func* m_policy_function;
	Stateful_Scorer_Func* m_scorer_function;
};

class StatelessFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef u32 Policy_Func(Context* application_context);
	typedef void Scorer_Func(Context* application_context, float*& scores, size_t& num_scores);

	Stateless_Policy_Func* m_policy_function;
	Stateless_Scorer_Func* m_scorer_function;
};