#ifndef _IO_QUEUE_
#define _IO_QUEUE_

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>   

#include "io_item.h"

//static std::mutex _mutex_io;

//Adds a line of input to the input queue
inline bool add_to_queue(vw& all, char *& line){

  std::lock_guard<std::mutex> lck(all.p->io_queue_lock);
  //std::lock_guard<std::mutex> lck(_mutex_io);
  bool finish = false;

  size_t num_chars_initial = readto(*(all.p->input), line, '\n');

  //std::cout << "line: " << line << std::endl;
  
  if(num_chars_initial < 1 || strlen(line) < 1){
      all.p->_io_state.have_added_io = true;
      finish = true;
  }

  IO_Item line_item(std::string(line), num_chars_initial);

  //std::cout << "push line to io queue" << std::endl;

  all.p->_io_state.io_lines->push(line_item);

  all.p->_io_state.have_added_io = true;

  return finish;

}

//Adds all lines of input to the input queue
inline void io_lines_toqueue(vw& all){

  //std::lock_guard<std::mutex> lck(_mutex_io);
  
  parser *original_p = all.p;
  
  char* line = nullptr;

  bool should_finish = false;

  while(!should_finish)
  {    
    should_finish = add_to_queue(all, line);

  }

  all.p = original_p;

  //std::cout << "done with io" << std::endl;
  all.p->_io_state.done_with_io = true;

}

//Pops a line of input from the input queue
inline IO_Item pop_io_queue(vw *all){
  
  std::lock_guard<std::mutex> lck((*all).p->io_queue_lock);
  //std::lock_guard<std::mutex> lck(_mutex_io);
  IO_Item front;
  
  if((*all).p->_io_state.io_lines->size() > 0)
  {

    front = (*all).p->_io_state.io_lines->front();

    (*all).p->_io_state.io_lines->pop();
   
  }

 /* {
      std::unique_lock<std::mutex> lock((*all).p->input_lock);
      //return 1 or something true??
      //(*all).p->input_done.wait(lock, [&] { return should_pop; });
      (*all).p->input_done.notify_one();
  }*/
  
  return front;

  

}

#endif


