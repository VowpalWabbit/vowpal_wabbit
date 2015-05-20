#pragma once

#include "MWTExplorer.h"
#include "utility.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace MultiWorldTesting;

class TestContext
{

};

class TestVarContext : public TestContext, public IVariableActionContext
{
public:
    TestVarContext(u32 num_actions)
    {
        m_num_actions = num_actions;
    }

    u32 Get_Number_Of_Actions()
    {
        return m_num_actions;
    }

private:
    u32 m_num_actions;
};

template <class Ctx>
struct TestInteraction
{
	Ctx& Context;
	u32* Actions;
    u32 Number_Of_Actions;
	float Probability;
	string Unique_Key;
};

template <class TContext>
class TestPolicy : public IPolicy<TContext>
{
public:
	TestPolicy(int params, int num_actions) : m_params(params), m_num_actions(num_actions) { }
    void Choose_Action(TContext& context, u32* actions, u32 num_actions)
	{
        for (u32 i = 0; i < num_actions; i++)
        {
            actions[i] = (m_params + i) % m_num_actions + 1; // action id is one-based
        }
	}
private:
	int m_params;
	int m_num_actions;
};

template <class TContext>
class TestScorer : public IScorer<TContext>
{
public:
	TestScorer(int params, int num_actions, bool uniform = true) : 
		m_params(params), m_num_actions(num_actions), m_uniform(uniform) 
	{ }

    vector<float> Score_Actions(TContext& context)
	{
		vector<float> scores;
		if (m_uniform)
		{
			for (int i = 0; i < m_num_actions; i++)
			{
				scores.push_back((float)m_params);
			}
		}
		else
		{
            for (int i = 0; i < m_num_actions; i++)
			{
                scores.push_back((float)m_params + i);
			}
		}
		return scores;
	}
private:
	int m_params;
	int m_num_actions;
	bool m_uniform;
};

class FixedScorer : public IScorer<TestContext>
{
public:
	FixedScorer(int num_actions, int value) :
		m_num_actions(num_actions), m_value(value)
	{ }

	vector<float> Score_Actions(TestContext& context)
	{
		vector<float> scores;
        for (int i = 0; i < m_num_actions; i++)
		{
			scores.push_back((float)m_value);
		}
		return scores;
	}
private:
	int m_num_actions;
	int m_value;
};

class TestSimpleScorer : public IScorer<SimpleContext>
{
public:
	TestSimpleScorer(int params, int num_actions) : m_params(params), m_num_actions(num_actions) { }
	vector<float> Score_Actions(SimpleContext& context)
	{
		vector<float> scores;
        for (int i = 0; i < m_num_actions; i++)
		{
            scores.push_back((float)m_params);
		}
		return scores;
	}
private:
	int m_params;
	int m_num_actions;
};

class TestSimplePolicy : public IPolicy<SimpleContext>
{
public:
	TestSimplePolicy(int params, int num_actions) : m_params(params), m_num_actions(num_actions) { }
    void Choose_Action(SimpleContext& context, u32* actions, u32 num_actions)
	{
        for (u32 i = 0; i < num_actions; i++)
        {
            actions[i] = (m_params + i) % m_num_actions + 1; // action id is one-based
        }
	}
private:
	int m_params;
	int m_num_actions;
};

class TestSimpleRecorder : public IRecorder<SimpleContext>
{
public:
    virtual void Record(SimpleContext& context, u32* actions, u32 num_actions, float probability, string unique_key)
	{
        m_interactions.push_back({ context, actions, num_actions, probability, unique_key });
	}

	vector<TestInteraction<SimpleContext>> Get_All_Interactions()
	{
		return m_interactions;
	}

private:
	vector<TestInteraction<SimpleContext>> m_interactions;
};

// Return action outside valid range
class TestBadPolicy : public IPolicy<TestContext>
{
public:
    void Choose_Action(TestContext& context, u32* actions, u32 num_actions)
	{
        for (u32 i = 0; i < num_actions; i++)
        {
            actions[0] = num_actions + i + 1;
        }
	}
};

template <class TContext>
class TestRecorder : public IRecorder<TContext>
{
public:
    virtual void Record(TContext& context, u32* actions, u32 num_actions, float probability, string unique_key)
	{
        m_interactions.push_back({ context, actions, num_actions, probability, unique_key });
	}

    vector<TestInteraction<TContext>> Get_All_Interactions()
	{
		return m_interactions;
	}

private:
    vector<TestInteraction<TContext>> m_interactions;
};
