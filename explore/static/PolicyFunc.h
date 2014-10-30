#pragma once

#include "utility.h"
#include "Interaction.h"

MWT_NAMESPACE {

typedef u32 Stateful_Policy_Func(void* policy_params, void* application_context);
typedef u32 Stateless_Policy_Func(void* application_context);
typedef void Stateful_Scorer_Func(void* policy_params, void* application_context, float scores[], u32 size);
typedef void Stateless_Scorer_Func(void* application_context, float scores[], u32 size);

template <class T>
struct Stateful
{
	typedef u32 Policy(T& policy_params, BaseContext& application_Context);
	typedef void Scorer(T& policy_params, BaseContext& application_Context, float scores[], u32 size);
};

typedef u32 Policy(BaseContext& application_Context);
typedef void Scorer(BaseContext& application_Context, float scores[], u32 size);

}