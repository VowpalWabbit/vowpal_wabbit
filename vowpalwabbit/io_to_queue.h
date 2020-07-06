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
    size_t num_chars_initial = all.p->input_file_reader(&all, line, example_arr);
    should_finish = all.p->_io_state.add_to_queue(line, num_chars_initial);
  }

  all.p->_io_state.done_with_io.store(true);

}

inline size_t read_input_file_ascii(vw *all, char *&line, v_array<example*>&) {
  std::cout << "read input file ascii" << std::endl;
  return readto(*(all->p->input), line, '\n');
}

inline size_t read_input_file_binary(vw *all, char *&line, v_array<example*>& examples) {

  std::cout << "read input file binary" << std::endl;

  //all->p->input is the cache file.

  size_t total_num_bytes = 0;
  std::string cache_file_contents = "";

  char *buffer = nullptr;
    
  size_t num_bytes = 8;

  size_t num_bytes_successfully_read;
  all->p->input->buf_read(buffer, sizeof(buffer));

  num_bytes_successfully_read = strlen(buffer);

  std::cout << "num_bytes_successfully_read: " << num_bytes_successfully_read << std::endl;
  std::cout << "buffer:[" << buffer << "]" << std::endl;

  line = std::move(buffer);

  std::cout << "line: " << line << std::endl;

 //return 0 to indicate that we should finish reading in input (i.e. we are at the end of the cache file)
  return 0;
 
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

