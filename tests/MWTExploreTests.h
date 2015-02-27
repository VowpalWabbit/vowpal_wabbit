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

template <class Ctx>
struct TestInteraction
{
	Ctx& Context;
	u32 Action;
	float Probability;
	string Unique_Key;
};

class TestPolicy : public IPolicy<TestContext>
{
public:
	TestPolicy(int params, int num_actions) : m_params(params), m_num_actions(num_actions) { }
	u32 Choose_Action(TestContext& context)
	{
		return m_params % m_num_actions + 1; // action id is one-based
	}
private:
	int m_params;
	int m_num_actions;
};

class TestScorer : public IScorer<TestContext>
{
public:
	TestScorer(int params, int num_actions, bool uniform = true) : 
		m_params(params), m_num_actions(num_actions), m_uniform(uniform) 
	{ }

	vector<float> Score_Actions(TestContext& context)
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
	u32 Choose_Action(SimpleContext& context)
	{
		return m_params % m_num_actions + 1; // action id is one-based
	}
private:
	int m_params;
	int m_num_actions;
};

class TestSimpleRecorder : public IRecorder<SimpleContext>
{
public:
	virtual void Record(SimpleContext& context, u32 action, float probability, string unique_key)
	{
		m_interactions.push_back({ context, action, probability, unique_key });
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
	u32 Choose_Action(TestContext& context)
	{
		return 100;
	}
};

class TestRecorder : public IRecorder<TestContext>
{
public:
	virtual void Record(TestContext& context, u32 action, float probability, string unique_key)
	{
		m_interactions.push_back({ context, action, probability, unique_key });
	}

	vector<TestInteraction<TestContext>> Get_All_Interactions()
	{
		return m_interactions;
	}

private:
	vector<TestInteraction<TestContext>> m_interactions;
};
