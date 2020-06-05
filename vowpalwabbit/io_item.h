// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

class io_item {

  public:

      std::vector<char> message;
      int num_chars_init;

      io_item(){
          num_chars_init = 0;
      }

      io_item(std::vector<char> myMsg, int myNumCharsInit){
          message = myMsg;
          num_chars_init = myNumCharsInit;
      }

      ~io_item() {}

};

struct io_state {

    std::queue<io_item> *io_lines = nullptr;
    std::atomic<bool> done_with_io;

    std::mutex cv_mutex;
    std::condition_variable has_input_cv;

    std::mutex io_queue_lock;

    //mutex for io queue access
    std::mutex mut;

    io_state()
      : io_lines{new std::queue<io_item>}
      , done_with_io{false}
    {
    }

    io_state operator=(const io_state &toCopy){
        io_lines = toCopy.io_lines;
        done_with_io.store(toCopy.done_with_io);
        return *this;
    }

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

    inline bool add_to_queue(char *line, io_buf *input){

      std::unique_lock<std::mutex> cv_lock(cv_mutex);

      bool finish = false;

      size_t num_chars_initial = readto(*input, line, '\n');

      std::vector<char> byte_array;
      byte_array.resize(num_chars_initial); // Note: This byte_array is NOT null terminated!

      if(num_chars_initial < 1){
          finish = true;
      }

      memcpy(byte_array.data(), line, num_chars_initial);
      {
        std::lock_guard<std::mutex> lck(io_queue_lock);
        io_lines->emplace(std::move(byte_array), num_chars_initial); // in-place construction and no copying. very efficient!
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

