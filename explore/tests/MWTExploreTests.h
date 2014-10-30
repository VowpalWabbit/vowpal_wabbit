#pragma once

#include "MWTExplorer.h"
#include "MWTRewardReporter.h"
#include "MWTOptimizer.h"
#include "utility.h"

using namespace MultiWorldTesting;

class TestContext : public BaseContext
{
public:
	TestContext()
	{
		m_features.push_back({ 1.5f, 1 });
		m_features.push_back({ .7f, 3 });
		m_features.push_back({ .12f, 6 });
	}

	virtual void Get_Features(size_t& num_features, Feature*& features)
	{
		num_features = m_features.size();
		features = &m_features[0];
	}
private:
	vector<Feature> m_features;
};