#pragma once

#include "vw.net.native.h"
#include "vw/io/io_adapter.h"

namespace vw_net_native
{
// using error_fn = void (*)(error_context* error_context, reinforcement_learning::api_status* api_status);
using io_writer_write_fn = ssize_t (*)(const char* buffer, size_t num_bytes);
using io_writer_flush_fn = void (*)();

using io_reader_read_fn = ssize_t (*)(char* buffer, size_t num_bytes);
using io_reader_reset_fn = void (*)();

using io_adapter_release_fn = void (*)();

struct io_writer_vtable
{
  io_adapter_release_fn release;
  io_writer_write_fn write;
  io_writer_flush_fn flush;
};

struct io_reader_vtable
{
  io_adapter_release_fn release;
  io_reader_read_fn read;
  io_reader_reset_fn reset;
  bool is_resettable;
};

class stream_io_writer : public VW::io::writer
{
private:
  io_writer_vtable vtable;

public:
  stream_io_writer(io_writer_vtable vtable);
  ~stream_io_writer();

  ssize_t write(const char* buffer, size_t num_bytes) override;
  void flush() override;
};

class stream_io_reader : public VW::io::reader
{
private:
  io_reader_vtable vtable;

public:
  stream_io_reader(io_reader_vtable vtable);
  ~stream_io_reader();

  ssize_t read(char* buffer, size_t num_bytes) override;
  void reset() override;
};

}  // namespace vw_net_native
