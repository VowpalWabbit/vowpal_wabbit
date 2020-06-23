// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>   

#include "io_item.h"

//Adds all lines of input to the input queue
inline void io_lines_toqueue(vw& all){
  
  parser *original_p = all.p;
  
  char* line = nullptr;

  bool should_finish = false;

  while(!should_finish)
  {    
    should_finish = all.p->_io_state.add_to_queue(line, all.p->input);

  }

  all.p = original_p;

  all.p->_io_state.done_with_io.store(true);

}

//try to encapsulate start io thread and end io thread, so it can be moved out of parse dispatch loop.
namespace VW {

inline void start_io_thread(vw& all){

  io_state curr_io_state;

  all.p->_io_state = curr_io_state ;

  all.io_thread = std::thread([&all]() 
  {

    io_lines_toqueue(all);

  });

}

inline void end_io_thread(vw& all){

  all.io_thread.join();

}

}

