#ifndef _IO_QUEUE_
#define _IO_QUEUE_

#include <thread>
#include <future>
#include <mutex>  
#include <chrono>     

static std::mutex _mutex_io;


/*namespace io_item {
    class IO_Item;
}*/

#ifndef _IO_ITEM_
#define _IO_ITEM_
class IO_Item {

    public:

        IO_Item(){
            message = "";
            numCharsInit = 0;
        }

        IO_Item(std::string myMsg, int myNumCharsInit){
            message = myMsg;
            numCharsInit = myNumCharsInit;
        }

        //add const string?

        IO_Item operator=(IO_Item toCopy){
            message = toCopy.getString();
            numCharsInit = toCopy.getNumCharsInit();
            return *this;
        }

        ~IO_Item() {}

        inline std::string getString(){
            return std::string(message);
        }

        inline int getNumCharsInit(){
          return numCharsInit;
        }

        inline void setString(std::string newMsg){
            message = newMsg;
        }

        inline void setNumCharsInit(int newNum){
            numCharsInit = newNum;
        }
    
    private:
        std::string message;
        int numCharsInit;

    /*public:

        IO_Item(){
            message = new std::string("");;
            numCharsInit = 0;
        }

        IO_Item(std::string myMsg, int myNumCharsInit){
            message = new std::string(myMsg);
            numCharsInit = myNumCharsInit;
        }

        //add const string?

        IO_Item operator=(IO_Item toCopy){
            message = new std::string(*toCopy.getString());
            numCharsInit = toCopy.getNumCharsInit();
            return *this;
        }

        ~IO_Item() {}

        inline std::string * getString(){
          std::string *copymsg = new std::string(*message);
          return copymsg;
        }

        inline int getNumCharsInit(){
          return numCharsInit;
        }

        inline void setString(std::string newMsg){
            message = new std::string(newMsg);
        }

        inline void setNumCharsInit(int newNum){
            numCharsInit = newNum;
        }
    
    private:
        std::string *message;
        int numCharsInit;*/

};
//}
#endif

/*#ifndef _IO_Q_COPY_
#define _IO_Q_COPY_*/
static std::queue<IO_Item> *input_lines_copy = new std::queue<IO_Item>;
static bool called_i_l_t = false;
static bool have_added_io = false;

//#endif

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

    IO_Item *line_item = new IO_Item(std::string(line), num_chars_initial);

    input_lines->push(*line_item);
    input_lines_copy->push(*line_item);

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

      //IO_Item *front = new IO_Item(input_lines_copy->front());
      IO_Item front = input_lines_copy->front();
      //std::cout << "front string: " << front.getString();

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
      //IO_Item front = input_lines_copy->front();
      //std::cout << "front string: " << front.getString();

      input_lines_copy->pop();

      return front;
      
    }else if(input_lines_copy->size() == 0 && have_added_io){
      IO_Item *front = new IO_Item(std::string("empty"), 0);

      return front; 
    }

    IO_Item *front = new IO_Item(std::string("bad"), 0);

    return front; 

}

//namespace IO {


#endif