#ifndef _IO_QUEUE_
#define _IO_QUEUE_

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>   

#include "io_item.h"

//Adds a line of input to the input queue
inline bool add_to_queue(vw& all, char *& line){

  bool finish = false;

  size_t num_chars_initial = readto(*(all.p->input), line, '\n');
  
  if(num_chars_initial < 1 || strlen(line) < 1){
      finish = true;
  }

  IO_Item line_item(std::string(line), num_chars_initial);

  {
    std::lock_guard<std::mutex> lck(all.p->io_queue_lock);
    all.p->_io_state.io_lines->push(line_item);
  }

  return finish;

}

//Adds all lines of input to the input queue
inline void io_lines_toqueue(vw& all){
  
  parser *original_p = all.p;
  
  char* line = nullptr;

  bool should_finish = false;

  while(!should_finish)
  {    
    should_finish = add_to_queue(all, line);

  }

  all.p = original_p;

  all.p->_io_state.done_with_io.store(true);

}

//Pops a line of input from the input queue
inline IO_Item pop_io_queue(vw *all){
  
  std::lock_guard<std::mutex> lck((*all).p->io_queue_lock);

  IO_Item front;
  
  if((*all).p->_io_state.io_lines->size() > 0)
  {

    front = (*all).p->_io_state.io_lines->front();

    (*all).p->_io_state.io_lines->pop();
   
  }

  return front;

}

#endif


