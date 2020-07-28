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
  
  char* line = nullptr;

  bool should_finish = false;

  while(!should_finish)
  {    
    v_array<example*> example_arr;

    should_finish = all.p->input_file_reader(&all, line, example_arr);
  }

  all.p->_io_state.done_with_io.store(true);

}

inline bool read_input_file_ascii(vw *all, char *&line, v_array<example*>&) {

  size_t num_chars_initial = readto(*(all->p->input), line, '\n');

  bool should_finish = true;
  
  if(num_chars_initial > 0) {
    should_finish = all->p->_io_state.add_to_queue(line, num_chars_initial);
  }

  return should_finish;
}

inline bool read_input_file_binary(vw *all, char *&line, v_array<example*>&) {

  size_t total_num_input_bytes = 0;

  //read_all_data returns the number of bytes successfully read from the input files
  total_num_input_bytes = all->p->input->read_all_data(line, total_num_input_bytes);

  bool should_finish = true;
  
  if(total_num_input_bytes > 0) {
    all->p->_io_state.add_to_queue(line, total_num_input_bytes);
  }

  return should_finish;
 
}

//try to encapsulate start io thread and end io thread, so it can be moved out of parse dispatch loop.
namespace VW {

inline void start_io_thread(vw& all){

  io_state curr_io_state;

  if(!all.early_terminate) {
    all.io_thread = std::thread([&all]() 
    {

      io_lines_toqueue(all);

    });
  }

}

inline void end_io_thread(vw& all){

  all.io_thread.join();

}

}

