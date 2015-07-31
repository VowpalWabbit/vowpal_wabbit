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
	: AllReduce(ptotal, pnode) // , child(nullptr), parent(nullptr),
	// reduce_child(new promise<Data*>),
	// broadcast(new promise<void>)
{
}

AllReduceThreads::AllReduceThreads(AllReduceThreads* pparent, size_t ptotal, const size_t pnode)
	: AllReduce(ptotal, pnode), child(nullptr), parent(pparent),
	reduce_child(new promise<Data*>),
	broadcast(new promise<void>)
{
	pparent->child = this;
}

AllReduceThreads::~AllReduceThreads()
{
	delete reduce_child;
	delete broadcast;
}

Data* AllReduceThreads::get_child_data()
{
	auto reduce_future = reduce_child->get_future();
	reduce_future.wait();
	auto left_data = reduce_future.get();
	*reduce_child = promise<Data*>();

	return left_data;
}

void AllReduceThreads::pass_up_and_wait_for_broadcast(void* buffer, size_t n)
{
	Data d = { buffer, n };
	parent->reduce_child->set_value(&d);

	parent->broadcast->get_future().wait();
	*parent->broadcast = promise<void>();
}

void AllReduceThreads::notify_child()
{
	broadcast->set_value();
}
