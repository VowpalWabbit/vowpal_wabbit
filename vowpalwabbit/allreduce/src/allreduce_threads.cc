// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

/*
This implements the allreduce function using threads.
*/
#include "vw/allreduce/allreduce.h"

#include <future>

VW::AllReduceSync::AllReduceSync(size_t total) : _total(total), _count(0), _run(true) { buffers = new void*[total]; }

VW::AllReduceSync::~AllReduceSync() { delete[] buffers; }

void VW::AllReduceSync::wait_for_synchronization()
{
  std::unique_lock<std::mutex> lock(_mutex);
  _count++;

  if (_count >= _total)
  {
    assert(_count == _total);

    _cv.notify_all();

    // order of _count before or after notify_all doesn't matter
    // since the lock is still hold at this point in time.
    _count = 0;

    // flip for the next run
    _run = !_run;
  }
  else
  {
    bool current_run = _run;
    // this predicate cannot depend on _count, as somebody can race ahead and _count++
    // FYI just wait can spuriously wake-up
    _cv.wait(lock, [this, current_run] { return _run != current_run; });
  }
}

VW::AllReduceThreads::AllReduceThreads(AllReduceThreads* root, size_t ptotal, size_t pnode, bool pquiet)
    : AllReduce(ptotal, pnode, pquiet), _sync(root->_sync), _sync_owner(false)
{
}

VW::AllReduceThreads::AllReduceThreads(size_t ptotal, size_t pnode, bool pquiet)
    : AllReduce(ptotal, pnode, pquiet), _sync(new AllReduceSync(ptotal)), _sync_owner(true)
{
}

VW::AllReduceThreads::~AllReduceThreads()
{
  if (_sync_owner) { delete _sync; }
}
