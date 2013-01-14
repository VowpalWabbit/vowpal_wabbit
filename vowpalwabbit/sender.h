/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
namespace SENDER{
void parse_send_args(po::variables_map& vm, std::vector<std::string> pairs);
void drive_send(void*);
 void save_load(void* in, io_buf& model_file, bool read, bool text);
}
