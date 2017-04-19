/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "io_buf.h"
#include "v_array.h"
#include <vector>
#include <stdio.h>
#include <memory>

#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>

class comp_io_buf : public io_buf
{
	class comp_io_input
	{
	private:
		boost::iostreams::filtering_streambuf<boost::iostreams::input> buf;
		std::istream* input;

	public:
		comp_io_input(std::istream* input, bool own = true);
		~comp_io_input();

		std::istream in;
	};

	class comp_io_output
	{
	private:
		boost::iostreams::filtering_streambuf<boost::iostreams::output> buf;
		std::unique_ptr<std::ostream> output;

	public:
		comp_io_output(std::ostream* output);
		std::ostream out;
	};

	struct comp_spec
	{
		std::string name;

		bool stdin_off;
	};

public:
  std::vector<std::unique_ptr<comp_io_input>> gz_input;
  std::vector<std::unique_ptr<comp_io_output>> gz_output;
  std::vector<comp_spec> gz_spec;

  virtual int open_file(const char* name, bool stdin_off, int flag = READ);

  virtual void reset_file(int f);

  virtual ssize_t read_file(int f, void* buf, size_t nbytes);

  virtual size_t num_files();

  virtual ssize_t write_file(int file, const void* buf, size_t nbytes);

  virtual bool compressed();

  virtual void flush();

  virtual bool close_file();
};
