#ifndef _IO_QUEUE_
#define _IO_QUEUE_

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>     

static std::mutex _mutex_io;

#ifndef _IO_ITEM_
#define _IO_ITEM_
class IO_Item {

  public:

      std::string message;
      int numCharsInit;

      IO_Item(){
          numCharsInit = 0;
      }

      IO_Item(std::string myMsg, int myNumCharsInit){

          message.assign(myMsg);
          numCharsInit = myNumCharsInit;
      }

      IO_Item operator=(const IO_Item &toCopy){
          message.assign(toCopy.message);
          numCharsInit = toCopy.numCharsInit;
          return *this;
      }

      IO_Item(const IO_Item &toCopy){
          message.assign(toCopy.message);
          numCharsInit = toCopy.numCharsInit;
      }

      ~IO_Item() {}

      inline std::string getString(){
          return std::string(message);
      }

      inline int getNumCharsInit(){
        return numCharsInit;
      }

      inline void setString(std::string newMsg){
          message.assign(newMsg);
      }

      inline void setNumCharsInit(int newNum){
          numCharsInit = newNum;
      }

};

#endif

static std::queue<IO_Item> *input_lines_copy = new std::queue<IO_Item>;
static bool called_i_l_t = false;
static bool have_added_io = false;
static bool done_with_io = false;

inline void io_lines_toqueue(vw& all, std::queue<IO_Item> *input_lines){

  //std::lock_guard<std::mutex> lck(_mutex_io);

  //parser *original_p = all->p;
  parser *original_p = all.p;
  
  char* line = nullptr;

  while(true)
  {

    //std::cout << "readto" << std::endl;
    //size_t num_chars_initial = readto(*(all->p->input), line, '\n');
    size_t num_chars_initial = readto(*(all.p->input), line, '\n');

    //std::cout << "line: " << line << std::endl;
   
    if(num_chars_initial < 1 || strlen(line) < 1){
        called_i_l_t = true;
        have_added_io = true;
        break;
    }

    IO_Item line_item(std::string(line), num_chars_initial);

    //std::cout << "push line to io queue" << std::endl;

    input_lines->push(line_item);
    input_lines_copy->push(line_item);

   // std::this_thread::sleep_for (std::chrono::seconds(1));

    called_i_l_t = true;
    have_added_io = true;

  }

  //all->p = original_p;
  all.p = original_p;

  done_with_io = true;
 /* std::cout << "input_lines_copy->size(): " << input_lines_copy->size() << std::endl;
  std::cout << "input_lines->size(): " << input_lines->size() << std::endl;*/

}

inline bool added_io(){
  return have_added_io;
}

inline bool io_done(){
  return done_with_io;
}

//will hopefully be called after io_lines_to_queue called.
inline IO_Item pop_io_queue(bool should_pop){


  std::lock_guard<std::mutex> lck(_mutex_io);

  if(should_pop && input_lines_copy->size() > 0)
  {
    IO_Item front = input_lines_copy->front();

    //std::cout << "pop line from io queue (when item in)" << std::endl;

    input_lines_copy->pop();

    return front;
    
  }else if(input_lines_copy->size() == 0 && have_added_io && done_with_io){
    //std::cout << "pop line from io queue (before item in)" << std::endl;
    return IO_Item(std::string("empty"), 0);
  }else if(input_lines_copy->size() == 0 && have_added_io && !done_with_io){
    return IO_Item(std::string("bad"), 0);
  }

  return IO_Item(std::string("bad"), 0);

}

#endif