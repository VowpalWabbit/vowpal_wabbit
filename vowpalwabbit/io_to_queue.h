#ifndef _IO_QUEUE_
#define _IO_QUEUE_

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>     

static std::mutex _mutex_io;

#ifndef _IO_Q_COPY_
#define _IO_Q_COPY_
static std::queue<std::string> *input_lines_copy = new std::queue<std::string>;
static bool called_i_l_t = false;
static bool have_added_io = false;

#endif

inline void io_lines_toqueue(vw *all, std::queue<std::string> *input_lines){

  std::lock_guard<std::mutex> lck(_mutex_io);

  parser *original_p = all->p;
  
  char* line = nullptr;
  while(true)
  {

    size_t num_chars_initial = readto(*(all->p->input), line, '\n');
 
    if(num_chars_initial < 1 || strlen(line) < 1){break;}

    input_lines->push(std::string(line));
    input_lines_copy->push(std::string(line));


    called_i_l_t = true;
    have_added_io = true;


  }

  all->p = original_p;

}

inline bool added_io(){
  return have_added_io;
}

inline std::string pop_io_queue(bool should_pop){

    std::cout << "pop_io_queue" << std::endl;
    std::cout << "called_i_l_t: " << called_i_l_t << std::endl;

    if(should_pop && input_lines_copy->size() > 0)
    {

      std::string front = input_lines_copy->front();

      input_lines_copy->pop();

      return front;
      
    }

    return std::string("");

}

#endif