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

inline void io_lines_toqueue(vw *all, std::queue<IO_Item> *input_lines){

  std::lock_guard<std::mutex> lck(_mutex_io);

  parser *original_p = all->p;
  
  char* line = nullptr;

  while(true)
  {

    size_t num_chars_initial = readto(*(all->p->input), line, '\n');
   
    if(num_chars_initial < 1 || strlen(line) < 1){
        called_i_l_t = true;
        have_added_io = true;
        break;
    }

    IO_Item line_item(std::string(line), num_chars_initial);

    input_lines->push(line_item);
    input_lines_copy->push(line_item);

    called_i_l_t = true;
    have_added_io = true;

  }

  all->p = original_p;

}

inline bool added_io(){
  return have_added_io;
}

//will hopefully be called after io_lines_to_queue called.
inline IO_Item pop_io_queue(bool should_pop){

    std::lock_guard<std::mutex> lck(_mutex_io);

    if(should_pop && input_lines_copy->size() > 0)
    {
      IO_Item front = input_lines_copy->front();

      input_lines_copy->pop();

      return front;
      
    }else if(input_lines_copy->size() == 0 && have_added_io){
      return IO_Item(std::string("empty"), 0);
    }

    return IO_Item(std::string("bad"), 0);

}


inline IO_Item * pop_io_queue2(bool should_pop){

    std::lock_guard<std::mutex> lck(_mutex_io);

    if(should_pop && input_lines_copy->size() > 0)
    {

      IO_Item *front = new IO_Item(input_lines_copy->front());

      input_lines_copy->pop();

      return front;
      
    }else if(input_lines_copy->size() == 0 && have_added_io){
      IO_Item *front = new IO_Item(std::string("empty"), 0);

      return front; 
    }

    IO_Item *front = new IO_Item(std::string("bad"), 0);

    return front; 

}

#endif