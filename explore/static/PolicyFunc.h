#pragma once

#include "utility.h"
#include "Interaction.h"

class BaseFunctionWrapper { };

typedef u32 Stateful_Policy_Func(void* policy_params, void* application_context);
typedef u32 Stateless_Policy_Func(void* application_context);
typedef void Stateful_Scorer_Func(void* policy_params, void* application_context, float scores[], u32 size);
typedef void Stateless_Scorer_Func(void* application_context, float scores[], u32 size);

template <class T>
class StatefulFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef u32 Policy_Func(T* policy_params, Context& application_Context);
	typedef void Scorer_Func(T* policy_params, Context& application_Context, float scores[], u32 size);

	Stateful_Policy_Func* m_policy_function;
	Stateful_Scorer_Func* m_scorer_function;
};

class StatelessFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef u32 Policy_Func(Context& application_Context);
	typedef void Scorer_Func(Context& application_Context, float scores[], u32 size);

	Stateless_Policy_Func* m_policy_function;
	Stateless_Scorer_Func* m_scorer_function;
};
