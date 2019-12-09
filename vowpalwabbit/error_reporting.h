// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
typedef void (*trace_message_t)(void* context, const std::string&);

// TODO: change to virtual class

// invoke trace_listener when << endl is encountered.
class vw_ostream : public std::ostream
{
  class vw_streambuf : public std::stringbuf
  {
    vw_ostream& parent;

   public:
    vw_streambuf(vw_ostream& str) : parent(str){};

    virtual int sync();
  };
  vw_streambuf buf;

 public:
  vw_ostream();

  void* trace_context;
  trace_message_t trace_listener;
};
