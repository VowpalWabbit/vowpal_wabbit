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
AllReduceThreads::AllReduceThreads(size_t ptotal, const size_t pnode)
	: AllReduce(ptotal, pnode)  // , child(nullptr), parent(nullptr),
	// reduce_child(new promise<Data*>),
	// broadcast(new promise<void>)
{
	root = this;
	m_mutex = new std::mutex;
	m_cv = new std::condition_variable;
	buffers = new void*[total];
}

AllReduceThreads::AllReduceThreads(AllReduceThreads* proot, size_t ptotal, const size_t pnode)
: AllReduce(ptotal, pnode), root(proot), count(0), m_mutex(nullptr), m_cv(nullptr), buffers(nullptr)
{
}

//AllReduceThreads::AllReduceThreads(AllReduceThreads* pparent, size_t ptotal, const size_t pnode)
//	: AllReduce(ptotal, pnode), child(nullptr), parent(pparent),
//	reduce_child(new promise<Data*>),
//	broadcast(new promise<void>)
//{
//	pparent->child = this;
//}

AllReduceThreads::~AllReduceThreads()
{
	delete m_mutex;
	delete m_cv;
	delete buffers;
}

void AllReduceThreads::waitForSynchronization()
{
	std::unique_lock<std::mutex> l(*root->m_mutex);
	count++;
		
	if (count == total)
	{
		root->m_cv->notify_all();
		count = 0;
	}
	else
	{
		m_cv->wait(l);
	}
}
