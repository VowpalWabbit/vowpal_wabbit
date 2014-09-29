#pragma once

#include "utility.h"
#include "Interaction.h"

class BaseFunctionWrapper
{ 
public:
	virtual ~BaseFunctionWrapper() { }
};

class MWT_Empty { };

typedef u32 Stateful_Policy_Func(void* state_Context, void* application_Context);
typedef u32 Stateless_Policy_Func(void* application_Context);
typedef void Stateful_Scorer_Func(void* state_Context, void* application_Context, float scores[], u32 size);
typedef void Stateless_Scorer_Func(void* application_Context, float scores[], u32 size);


template <class T>
class StatefulFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef u32 Policy_Func(T* state_Context, Context* application_Context);
	typedef void Scorer_Func(T* state_Context, Context* application_Context, float scores[], u32 size);

	Stateful_Policy_Func* m_policy_function;
	Stateful_Scorer_Func* m_scorer_function;
};

class StatelessFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef u32 Policy_Func(Context* application_Context);
	typedef void Scorer_Func(Context* application_Context, float scores[], u32 size);

	Stateless_Policy_Func* m_policy_function;
	Stateless_Scorer_Func* m_scorer_function;
};