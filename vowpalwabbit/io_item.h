// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

class io_item {

  public:

      std::vector<char> message;

      io_item(){

      }

      io_item(std::vector<char> myMsg){
          message = myMsg;
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

    io_state operator=(const io_state &) = delete;
    io_state(io_state&&) = delete;
    io_state(const io_state&) = delete;

    inline void set_done_io(bool done_io){
      done_with_io.store(done_io);
    }

    inline bool get_done_io(){
      return done_with_io;
    }


    ~io_state() {

      delete io_lines;
      
    }

    inline bool add_to_queue(char *line, size_t num_chars_initial){

      std::cout << "add_to-queue line: " << line << std::endl;

      bool finish = false;

      std::vector<char> byte_array;
      byte_array.resize(num_chars_initial); // Note: This byte_array is NOT null terminated!

      if(num_chars_initial < 1){
          finish = true;
      }

      memcpy(byte_array.data(), line, num_chars_initial);
      {
        std::lock_guard<std::mutex> lck(io_queue_lock);
        io_lines->emplace(std::move(byte_array));
      }

      has_input_cv.notify_all();

      return finish;

    }

    //Pops a line of input from the input queue
    inline io_item pop_io_queue(){
      
      std::unique_lock<std::mutex> lck(io_queue_lock);

      std::cout << "pop io queue" << std::endl;

      io_item front;

      if(io_lines->size() == 0 && done_with_io) {
        std::cout << "condition L130" << std::endl;

        return io_item();
      }

      while(!done_with_io && io_lines->size() == 0){
    
        std::cout << "wait" << std::endl;
        has_input_cv.wait(lck);
      
      }

      std::cout << "io_lines size (pop io queue): " << io_lines->size() << std::endl;
      
      if(io_lines->size() > 0)
      {

        std::cout << "get front" << std::endl;

        front = io_lines->front();

        io_lines->pop();
      
      }

      return front;

    }

};

