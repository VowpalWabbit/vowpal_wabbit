#include "vw.net.stream_adapter.h"

vw_net_native::stream_io_writer::stream_io_writer(io_writer_vtable vtable) : vtable{vtable} {}

vw_net_native::stream_io_writer::~stream_io_writer() { this->vtable.release(); }

ssize_t vw_net_native::stream_io_writer::write(const char* buffer, size_t num_bytes)
{
  // It really scares me that we do not have a defined way to report
  // success/failure in the io_writer/reader APIs
  return this->vtable.write(buffer, num_bytes);
}

void vw_net_native::stream_io_writer::flush() { this->vtable.flush(); }

vw_net_native::stream_io_reader::stream_io_reader(io_reader_vtable vtable)
    : reader{vtable.is_resettable}, vtable{vtable}
{
}

vw_net_native::stream_io_reader::~stream_io_reader() { this->vtable.release(); }

ssize_t vw_net_native::stream_io_reader::read(char* buffer, size_t num_bytes)
{
  // It really scares me that we do not have a defined way to report
  // success/failure in the io_writer/reader APIs
  return this->vtable.read(buffer, num_bytes);
}

void vw_net_native::stream_io_reader::reset() { this->vtable.reset(); }
