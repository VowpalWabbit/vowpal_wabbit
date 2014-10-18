#pragma once

#include "utility.h"
#include "Interaction.h"

typedef u32 Stateful_Policy_Func(void* policy_params, void* application_context);
typedef u32 Stateless_Policy_Func(void* application_context);
typedef void Stateful_Scorer_Func(void* policy_params, void* application_context, float scores[], u32 size);
typedef void Stateless_Scorer_Func(void* application_context, float scores[], u32 size);

template <class T>
class Stateful
{
public:
	typedef u32 Policy(T& policy_params, Context& application_Context);
	typedef void Scorer(T& policy_params, Context& application_Context, float scores[], u32 size);
};

typedef u32 Policy(Context& application_Context);
typedef void Scorer(Context& application_Context, float scores[], u32 size);
