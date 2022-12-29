// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/network.h"

#include "vw/common/string_view.h"
#include "vw/common/vw_exception.h"
#include "vw/core/numeric_casts.h"
#include "vw/core/parse_primitives.h"
#include "vw/io/errno_handling.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"

#ifdef _WIN32
#  define NOMINMAX
#  define _WINSOCK_DEPRECATED_NO_WARNINGS
#  include <WinSock2.h>
#else
#  include <netdb.h>       // gethostbyname
#  include <netinet/in.h>  // sockaddr_in, in_addr
#  include <sys/socket.h>  // socket, sockaddr, PF_INET, SOCK_STREAM
#endif

#include <string>

static constexpr const uint16_t DEFAULT_PORT = 26542;

std::unique_ptr<VW::io::socket> VW::details::open_vw_binary_socket(
    const std::string& host_and_optional_port, VW::io::logger& logger)
{
  auto colon_pos = host_and_optional_port.find(':');
  if (colon_pos == std::string::npos)
  {
    return VW::details::open_vw_binary_socket(host_and_optional_port, DEFAULT_PORT, logger);
  }

  std::string port = host_and_optional_port.substr(colon_pos + 1);
  std::string host = host_and_optional_port.substr(0, colon_pos);
  auto port_num = VW::details::int_of_string(port, logger);

  return open_vw_binary_socket(host, VW::cast_signed_to_unsigned<uint16_t>(port_num), logger);
}

std::unique_ptr<VW::io::socket> NO_SANITIZE_UNDEFINED VW::details::open_vw_binary_socket(
    const std::string& host, uint16_t port, VW::io::logger& /* logger */)
{
  hostent* he = gethostbyname(host.c_str());
  if (he == nullptr) { THROWERRNO("gethostbyname(" << host << ")"); }

  int sd = static_cast<int>(socket(PF_INET, SOCK_STREAM, 0));
  if (sd == -1) { THROWERRNO("socket"); }

  sockaddr_in far_end{};
  far_end.sin_family = AF_INET;
  far_end.sin_port = htons(port);
  // This is UB on MacOS due to a misaligned pointer.
  far_end.sin_addr = *reinterpret_cast<in_addr*>(he->h_addr);
  std::memset(&far_end.sin_zero, '\0', 8);
  if (connect(sd, reinterpret_cast<sockaddr*>(&far_end), sizeof(far_end)) == -1)
  {
    THROWERRNO("connect(" << host << ':' << port << ")");
  }

  auto socket_resource = VW::io::wrap_socket_descriptor(sd);
  auto socket_writer = socket_resource->get_writer();
  char id = '\0';
  if (socket_writer->write(&id, sizeof(id)) < static_cast<ssize_t>(sizeof(id))) { THROW("Failed to handshake socket.") }
  return socket_resource;
}
