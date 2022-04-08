// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/experimental/types.h"

#include "vw/io/io_adapter.h"

struct c_reader : public VW::io::reader
{
  c_reader(void* context, VWReadFunc* read);
  ssize_t read(char* buffer, size_t num_bytes) override;

private:
  void* _context = nullptr;
  VWReadFunc* _read_func = nullptr;
};

struct c_writer : public VW::io::writer
{
  c_writer(void* context, VWWriteFunc* write);
  ssize_t write(const char* buffer, size_t num_bytes) override;

private:
  void* _context = nullptr;
  VWWriteFunc* _write_func = nullptr;
};
