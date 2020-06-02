// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

class io_item {

  public:

      std::string message;
      int num_chars_init;

      io_item(){
          num_chars_init = 0;
      }

      io_item(std::string myMsg, int myNumCharsInit){
          message.assign(myMsg);
          num_chars_init = myNumCharsInit;
      }

     /* io_item operator=(const io_item &toCopy){
          message.assign(toCopy.message);
          num_chars_init = toCopy.num_chars_init;
          return *this;
      }

      io_item(const io_item &toCopy){
          message.assign(toCopy.message);
          num_chars_init = toCopy.num_chars_init;
      }*/

      ~io_item() {}

      /*inline const std::string& getString(){
          //return std::string(message);
          return message;
      }*/

      inline std::string getString(){
          return std::string(message);
      }

      inline int getNumCharsInit(){
        return num_chars_init;
      }

      inline void setString(std::string newMsg){
          message.assign(newMsg);
      }

      inline void setNumCharsInit(int newNum){
          num_chars_init = newNum;
      }

};

struct io_state {

    std::queue<io_item> *io_lines = nullptr;
    std::atomic<bool> done_with_io;

    std::mutex cv_mutex;
    std::condition_variable has_input_cv;

    std::mutex io_queue_lock;

    //mutex for io queue access
    std::mutex mut;
    //question: replace io_lines queue with ptr_queue to make it thread-safe?

    io_state()
      : io_lines{new std::queue<io_item>}
      , done_with_io{false}
    {
      /*io_lines = new std::queue<io_item>;
      done_with_io.store(false);*/
    }

    /*io_state(std::queue<io_item> *new_input_lines){
        //input_lines_copy now points to new_input_lines
        io_lines = new_input_lines;
        done_with_io.store(false);
    }*/

    io_state operator=(const io_state &toCopy){
        io_lines = toCopy.io_lines;
        done_with_io.store(toCopy.done_with_io);
        return *this;
    }

    //move constructor -- implement or delete? (ask mentor)
    //how about if reassign? leak memory?
    io_state(io_state&& iostate_to_move)
    {
      //if io_lines is not the null pointer, delete the memory it pointed to before reassigning
      if(io_lines){
        delete io_lines;
      }
      io_lines = iostate_to_move.io_lines;
      done_with_io.store(iostate_to_move.done_with_io);
    }

    io_state(const io_state &toCopy){
        io_lines = toCopy.io_lines;
        done_with_io.store(toCopy.done_with_io);
    }

    inline void set_done_io(bool done_io){
      done_with_io.store(done_io);
    }

    inline bool get_done_io(){
      return done_with_io;
    }


    ~io_state() {}

    inline bool add_to_queue(char *& line, io_buf *input){

      std::unique_lock<std::mutex> cv_lock(cv_mutex);

      bool finish = false;

      size_t num_chars_initial = readto(*input, line, '\n');
      
      if(num_chars_initial < 1 || strlen(line) < 1){
          finish = true;
      }

      io_item line_item(std::string(line), num_chars_initial);

      {
        std::lock_guard<std::mutex> lck(io_queue_lock);

        io_lines->push(line_item);
        
      }

      has_input_cv.notify_all();

      return finish;

    }

    //Pops a line of input from the input queue
    inline io_item pop_io_queue(){
      
      std::lock_guard<std::mutex> lck(io_queue_lock);

      io_item front;
      
      if(io_lines->size() > 0)
      {

        front = io_lines->front();

        io_lines->pop();
      
      }

      return front;

    }

};

