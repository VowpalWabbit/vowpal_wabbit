/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/
/*
This implements the allreduce function using threads.
*/
#include "allreduce.h"
#include <future>

using namespace std;

AllReduceSync::AllReduceSync(const size_t total) : m_total(total), m_count(0)
{
	m_mutex = new mutex;
	m_cv = new condition_variable;
	buffers = new void*[total];
}

AllReduceSync::~AllReduceSync()
{
	delete m_mutex;
	delete m_cv;
	delete buffers;
}

void AllReduceSync::waitForSynchronization()
{
	std::unique_lock<std::mutex> l(*m_mutex);
	m_count++;

	if (m_count >= m_total)
	{
		assert(m_count == m_total);

		m_cv->notify_all();
		m_count = 0;
	}
	else
	{
		m_cv->wait(l);
	}
}

AllReduceThreads::AllReduceThreads(AllReduceThreads* root, const size_t ptotal, const size_t pnode)
	: AllReduce(ptotal, pnode), m_sync(root->m_sync), m_syncOwner(false)
{
}

AllReduceThreads::AllReduceThreads(const size_t ptotal, const size_t pnode)
	: AllReduce(ptotal, pnode), m_sync(new AllReduceSync(ptotal)), m_syncOwner(true)
{
}

AllReduceThreads::~AllReduceThreads()
{
	if (m_syncOwner)
	{
		delete m_sync;
	}
}