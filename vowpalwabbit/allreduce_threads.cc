// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

/*
This implements the allreduce function using threads.
*/
#include "allreduce.h"
#include <future>

AllReduceSync::AllReduceSync(const size_t total) : m_total(total), m_count(0), m_run(true)
{
  m_mutex = new std::mutex;
  m_cv = new std::condition_variable;
  buffers = new void*[total];
}

AllReduceSync::~AllReduceSync()
{
  delete m_mutex;
  delete m_cv;
  delete[] buffers;
}

void AllReduceSync::waitForSynchronization()
{
  std::unique_lock<std::mutex> l(*m_mutex);
  m_count++;

  if (m_count >= m_total)
  {
    assert(m_count == m_total);

    m_cv->notify_all();

    // order of m_count before or after notify_all doesn't matter
    // since the lock is still hold at this point in time.
    m_count = 0;

    // flip for the next run
    m_run = !m_run;
  }
  else
  {
    bool current_run = m_run;
    // this predicate cannot depend on m_count, as somebody can race ahead and m_count++
    // FYI just wait can spuriously wake-up
    m_cv->wait(l, [this, current_run] { return m_run != current_run; });
  }
}

AllReduceThreads::AllReduceThreads(AllReduceThreads* root, const size_t ptotal, const size_t pnode, bool pquiet)
    : AllReduce(ptotal, pnode, pquiet), m_sync(root->m_sync), m_syncOwner(false)
{
}

AllReduceThreads::AllReduceThreads(const size_t ptotal, const size_t pnode, bool pquiet)
    : AllReduce(ptotal, pnode, pquiet), m_sync(new AllReduceSync(ptotal)), m_syncOwner(true)
{
}

AllReduceThreads::~AllReduceThreads()
{
  if (m_syncOwner)
  {
    delete m_sync;
  }
}
