#ifndef _IO_QUEUE_
#define _IO_QUEUE_

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>   

#include "io_item.h"

static std::mutex _mutex_io;

inline bool add_to_queue(vw& all, char *& line){

 // std::cout << "add_to_queue" << std::endl;

  std::lock_guard<std::mutex> lck(_mutex_io);

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

//inline void io_lines_toqueue(vw& all, IO_State *iostate){
inline void io_lines_toqueue(vw& all, std::queue<IO_Item> *io_to_set){

 // std::lock_guard<std::mutex> lck(_mutex_io);

  all.p->_io_state.io_lines = io_to_set;
  //std::cout << "io_lines_toqueue" << std::endl;
  
  parser *original_p = all.p;
  
  char* line = nullptr;

  bool should_finish = false;

  while(!should_finish)
  {

    //std::this_thread::sleep_for (std::chrono::seconds(1));
    //std::cout << "readto" << std::endl;
    //size_t num_chars_initial = readto(*(all->p->input), line, '\n');
    
    should_finish = add_to_queue(all, line);

  }

  all.p = original_p;

  //std::cout << "done with io" << std::endl;
  all.p->_io_state.done_with_io = true;
 /* std::cout << "input_lines_copy->size(): " << input_lines_copy->size() << std::endl;
  std::cout << "input_lines->size(): " << input_lines->size() << std::endl;*/

}

inline IO_Item pop_io_queue(vw *all, bool should_pop){
  
  std::lock_guard<std::mutex> lck(_mutex_io);

  IO_Item front;
  
  if(should_pop && (*all).p->_io_state.io_lines->size() > 0)
  {

    front = (*all).p->_io_state.io_lines->front();

    (*all).p->_io_state.io_lines->pop();
   
  }

  return front;

}

#endif