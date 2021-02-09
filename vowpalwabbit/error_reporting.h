// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

typedef void (*trace_message_t)(void* context, const std::string&);

class noop_output_streambuf : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};

class custom_output_stream_buf : public std::stringbuf
{
public:
  custom_output_stream_buf(void* trace_context, trace_message_t trace_listener)
      : _trace_context(trace_context), _trace_listener(trace_listener)
  {
  }

  virtual int sync()
  {
    auto ret = std::stringbuf::sync();
    if (ret != 0) { return ret; }

    _trace_listener(_trace_context, str());

    // Now that it has been outputted, clear the streambuf.
    str("");

    // Signal success.
    return 0;
  }

private:
  void* _trace_context;
  trace_message_t _trace_listener;
};

class owning_ostream : public std::ostream
{
public:
  owning_ostream(std::unique_ptr<std::streambuf>&& output) : std::ostream(output.get()), _output_buffer(std::move(output))
  {
  }

private:
  std::unique_ptr<std::streambuf> _output_buffer;
};
