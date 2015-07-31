/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_threaded.h"
#include "vw_example.h"
#include "vw_builder.h"
#include "vw_prediction.h"
#include "allreduce.h"

using namespace System;

namespace VW
{
	VowpalWabbitThreaded::VowpalWabbitThreaded(VowpalWabbitSettings^ args, size_t node)
		: VowpalWabbitNative(args), m_exampleCounter(0), m_syncCount(1000), m_node(node)
	{
		if (args->ParallelOptions == nullptr)
		{
			throw gcnew ArgumentNullException("args.ParallelOptions");
		}

		m_vw->all_reduce_type = AllReduceType::Thread;
		m_vw->all_reduce = new AllReduceThreads(args->ParallelOptions->MaxDegreeOfParallelism, node);
	}

	VowpalWabbitThreaded::VowpalWabbitThreaded(VowpalWabbitSettings^ args, VowpalWabbitThreaded^ parent, size_t node)
		: VowpalWabbitNative(args), m_exampleCounter(0), m_syncCount(1000), m_node(node)
	{
		if (args->ParallelOptions == nullptr)
		{
			throw gcnew ArgumentNullException("args.ParallelOptions");
		}

		m_vw->all_reduce_type = AllReduceType::Thread;
		auto parent_all_reduce = (AllReduceThreads*)parent->m_vw->all_reduce;
		m_vw->all_reduce = new AllReduceThreads(parent_all_reduce, args->ParallelOptions->MaxDegreeOfParallelism, node);
	}

	/*
	void VowpalWabbitThreaded::Learn(example* ex)
	{
		VowpalWabbitNative::Learn(ex);

		if (m_exampleCounter++ > m_syncCount)
		{
			try
			{
				m_vw->l->end_pass();
			}
			CATCHRETHROW
		}
	}

	void VowpalWabbitThreaded::LearnAndPredict(example* ex, VowpalWabbitPrediction^ result) 
	{
		VowpalWabbitNative::LearnAndPredict(ex, result);

		if (m_exampleCounter++ > m_syncCount)
		{
			try
			{
				m_vw->l->end_pass();
			}
			CATCHRETHROW
		}
	}
	*/

	void VowpalWabbitThreaded::EndOfPass()
	{
		try 
		{
			m_vw->l->end_pass();
			sync_stats(*m_vw);
		}
		CATCHRETHROW
	}

	size_t VowpalWabbitThreaded::NodeId::get()
	{
		return m_node;
	}
}