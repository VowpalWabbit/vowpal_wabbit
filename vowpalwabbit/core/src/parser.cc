// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/parser.h"

#include "vw/core/daemon_utils.h"
#include "vw/core/kskip_ngram_transformer.h"
#include "vw/core/numeric_casts.h"
#include "vw/io/errno_handling.h"
#include "vw/io/logger.h"
#include "vw/text_parser/parse_example_text.h"

#ifndef _WIN32
#  include <netinet/tcp.h>
#  include <sys/mman.h>
#  include <sys/wait.h>
#  include <unistd.h>
#endif

#include "vw/core/crossplat_compat.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/text_utils.h"

#include <csignal>
#include <fstream>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif

#  include <WinSock2.h>
#  include <Windows.h>
#  include <io.h>
typedef int socklen_t;
// windows doesn't define SOL_TCP and use an enum for the later, so can't check for its presence with a macro.
#  define SOL_TCP IPPROTO_TCP

int daemon(int /*a*/, int /*b*/) { exit(0); }

// Starting with v142 the fix in the else block no longer works due to mismatching linkage. Going forward we should just
// use the actual isocpp version.
// use VW_GETPID instead of getpid to avoid name collisions with process.h
#  if _MSC_VER >= 1920
#    define VW_GETPID _getpid
#  else
int VW_GETPID() { return (int)::GetCurrentProcessId(); }
#  endif

#else
#  include <netdb.h>
#  define VW_GETPID getpid
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
#  include <netinet/in.h>
#endif

#include "vw/cache_parser/parse_example_cache.h"
#include "vw/common/vw_exception.h"
#include "vw/core/constant.h"
#include "vw/core/interactions.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_dispatch_loop.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/reductions/conditional_contextual_bandit.h"
#include "vw/core/shared_data.h"
#include "vw/core/unique_sort.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/json_parser/parse_example_json.h"
#include "vw/text_parser/parse_example_text.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#ifdef BUILD_FLATBUFFERS
#  include "vw/fb_parser/parse_example_flatbuffer.h"
#endif

#ifdef VW_BUILD_CSV
#  include "vw/csv_parser/parse_example_csv.h"
#endif

// OSX doesn't expects you to use IPPROTO_TCP instead of SOL_TCP
#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#  define SOL_TCP IPPROTO_TCP
#endif

using std::endl;

// This should not? matter in a library mode.
bool got_sigterm;

void handle_sigterm(int) { got_sigterm = true; }

VW::parser::parser(size_t example_queue_limit, bool strict_parse_)
    : example_pool{example_queue_limit}
    , ready_parsed_examples{example_queue_limit}
    , example_queue_limit{example_queue_limit}
    , num_examples_taken_from_pool(0)
    , num_setup_examples(0)
    , num_finished_examples(0)
    , strict_parse{strict_parse_}
{
  this->lbl_parser = VW::simple_label_parser_global;
}

namespace VW
{
void parse_example_label(string_view label, const VW::label_parser& lbl_parser, const named_labels* ldict,
    label_parser_reuse_mem& reuse_mem, VW::example& ec, VW::io::logger& logger)
{
  std::vector<string_view> words;
  VW::tokenize(' ', label, words);
  lbl_parser.parse_label(ec.l, ec.ex_reduction_features, reuse_mem, ldict, words, logger);
}
}  // namespace VW

uint32_t cache_numbits(VW::io::reader& cache_reader)
{
  size_t version_buffer_length;
  if (static_cast<size_t>(cache_reader.read(reinterpret_cast<char*>(&version_buffer_length),
          sizeof(version_buffer_length))) < sizeof(version_buffer_length))
  {
    THROW("failed to read: version_buffer_length");
  }

  if (version_buffer_length > 61) THROW("cache version too long, cache file is probably invalid");
  if (version_buffer_length == 0) THROW("cache version too short, cache file is probably invalid");

  std::vector<char> version_buffer(version_buffer_length);
  if (static_cast<size_t>(cache_reader.read(version_buffer.data(), version_buffer_length)) < version_buffer_length)
  {
    THROW("failed to read: version buffer");
  }
  VW::version_struct cache_version(version_buffer.data());
  if (cache_version != VW::VERSION)
  {
    auto msg = fmt::format(
        "Cache file version does not match current VW version. Cache files must be produced by the version consuming "
        "them. Cache version: {} VW version: {}",
        cache_version.to_string(), VW::VERSION.to_string());
    THROW(msg);
  }

  char marker;
  if (static_cast<size_t>(cache_reader.read(&marker, sizeof(marker))) < sizeof(marker)) { THROW("failed to read"); }

  if (marker != 'c') THROW("data file is not a cache file");

  uint32_t cache_numbits;
  if (static_cast<size_t>(cache_reader.read(reinterpret_cast<char*>(&cache_numbits), sizeof(cache_numbits))) <
      sizeof(cache_numbits))
  {
    THROW("failed to read");
  }

  return cache_numbits;
}

void set_cache_reader(VW::workspace& all) { all.example_parser->reader = VW::parsers::cache::read_example_from_cache; }

void set_string_reader(VW::workspace& all)
{
  all.example_parser->reader = VW::parsers::text::read_features_string;
  all.print_by_ref = VW::details::print_result_by_ref;
}

bool is_currently_json_reader(const VW::workspace& all)
{
  return all.example_parser->reader == &VW::parsers::json::read_features_json<true> ||
      all.example_parser->reader == &VW::parsers::json::read_features_json<false>;
}

bool is_currently_dsjson_reader(const VW::workspace& all)
{
  return is_currently_json_reader(all) && all.example_parser->decision_service_json;
}

void set_json_reader(VW::workspace& all, bool dsjson = false)
{
  // TODO: change to class with virtual method
  // --invert_hash requires the audit parser version to save the extra information.
  if (all.audit || all.hash_inv)
  {
    all.example_parser->reader = &VW::parsers::json::read_features_json<true>;
    all.example_parser->text_reader = &VW::parsers::json::line_to_examples_json<true>;
    all.example_parser->audit = true;
  }
  else
  {
    all.example_parser->reader = &VW::parsers::json::read_features_json<false>;
    all.example_parser->text_reader = &VW::parsers::json::line_to_examples_json<false>;
    all.example_parser->audit = false;
  }

  all.example_parser->decision_service_json = dsjson;

  if (dsjson && all.global_metrics.are_metrics_enabled())
  {
    all.example_parser->metrics = VW::make_unique<VW::details::dsjson_metrics>();
  }
}

void set_daemon_reader(VW::workspace& all, bool json = false, bool dsjson = false)
{
  if (all.example_parser->input.isbinary())
  {
    all.example_parser->reader = VW::parsers::cache::read_example_from_cache;
    all.print_by_ref = VW::details::binary_print_result_by_ref;
  }
  else if (json || dsjson) { set_json_reader(all, dsjson); }
  else { set_string_reader(all); }
}

void VW::details::reset_source(VW::workspace& all, size_t numbits)
{
  io_buf& input = all.example_parser->input;

  // If in write cache mode then close all of the input files then open the written cache as the new input.
  if (all.example_parser->write_cache)
  {
    all.example_parser->output.flush();
    // Turn off write_cache as we are now reading it instead of writing!
    all.example_parser->write_cache = false;
    all.example_parser->output.close_file();

    // This deletes the file from disk.
    remove(all.example_parser->finalname.c_str());

    // Rename the cache file to the final name.
    if (0 != rename(all.example_parser->currentname.c_str(), all.example_parser->finalname.c_str()))
      THROW("WARN: reset_source(VW::workspace& all, size_t numbits) cannot rename: "
          << all.example_parser->currentname << " to " << all.example_parser->finalname);
    input.close_files();
    // Now open the written cache as the new input file.
    input.add_file(VW::io::open_file_reader(all.example_parser->finalname));
    set_cache_reader(all);
  }

  if (all.example_parser->resettable == true)
  {
    if (all.daemon)
    {
      // wait for all predictions to be sent back to client
      {
        std::unique_lock<std::mutex> lock(all.example_parser->output_lock);
        all.example_parser->output_done.wait(lock,
            [&]
            {
              return all.example_parser->num_finished_examples == all.example_parser->num_setup_examples &&
                  all.example_parser->ready_parsed_examples.size() == 0;
            });
      }

      all.final_prediction_sink.clear();
      all.example_parser->input.close_files();
      all.example_parser->input.reset();
      sockaddr_in client_address;
      socklen_t size = sizeof(client_address);
      int f =
          static_cast<int>(accept(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&client_address), &size));
      if (f < 0) THROW("accept: " << VW::io::strerror_to_string(errno));

      // Disable Nagle delay algorithm due to daemon mode's interactive workload
      int one = 1;
      setsockopt(f, SOL_TCP, TCP_NODELAY, reinterpret_cast<char*>(&one), sizeof(one));

      // note: breaking cluster parallel online learning by dropping support for id

      auto socket = VW::io::wrap_socket_descriptor(f);
      all.final_prediction_sink.push_back(socket->get_writer());
      all.example_parser->input.add_file(socket->get_reader());

      set_daemon_reader(all, is_currently_json_reader(all), is_currently_dsjson_reader(all));
    }
    else
    {
      if (!input.is_resettable()) { THROW("Cannot reset source as it is a non-resettable input type.") }
      input.reset();
      for (auto& file : input.get_input_files())
      {
        const auto num_bits_cachefile = cache_numbits(*file);
        if (num_bits_cachefile < numbits)
        {
          auto message =
              fmt::format("Num bits in the cache file is less than what was expected. Found '{}' but expected >= {}",
                  num_bits_cachefile, numbits);
          THROW(message);
        }
      }
    }
  }
}

void make_write_cache(VW::workspace& all, std::string& newname, bool quiet)
{
  VW::io_buf& output = all.example_parser->output;
  if (output.num_files() != 0)
  {
    all.logger.err_warn("There was an attempt tried to make two write caches. Only the first one will be made.");
    return;
  }

  all.example_parser->currentname = newname + std::string(".writing");
  try
  {
    output.add_file(VW::io::open_file_writer(all.example_parser->currentname));
  }
  catch (const std::exception&)
  {
    all.logger.err_error("Can't create cache file: {}", all.example_parser->currentname);
    return;
  }

  size_t v_length = static_cast<uint64_t>(VW::VERSION.to_string().length()) + 1;

  output.bin_write_fixed(reinterpret_cast<const char*>(&v_length), sizeof(v_length));
  output.bin_write_fixed(VW::VERSION.to_string().c_str(), v_length);
  output.bin_write_fixed("c", 1);
  output.bin_write_fixed(reinterpret_cast<const char*>(&all.num_bits), sizeof(all.num_bits));
  output.flush();

  all.example_parser->finalname = newname;
  all.example_parser->write_cache = true;
  if (!quiet) { *(all.trace_message) << "creating cache_file = " << newname << endl; }
}

void parse_cache(VW::workspace& all, std::vector<std::string> cache_files, bool kill_cache, bool quiet)
{
  all.example_parser->write_cache = false;

  for (auto& file : cache_files)
  {
    bool cache_file_opened = false;
    if (!kill_cache)
    {
      try
      {
        all.example_parser->input.add_file(VW::io::open_file_reader(file));
        cache_file_opened = true;
      }
      catch (const std::exception&)
      {
        cache_file_opened = false;
      }
    }
    if (cache_file_opened == false) { make_write_cache(all, file, quiet); }
    else
    {
      uint64_t c = cache_numbits(*all.example_parser->input.get_input_files().back());
      if (c < all.num_bits)
      {
        if (!quiet)
        {
          all.logger.err_warn("cache file is ignored as it's made with less bit precision than required.");
        }
        all.example_parser->input.close_file();
        make_write_cache(all, file, quiet);
      }
      else
      {
        if (!quiet) { *(all.trace_message) << "using cache_file = " << file.c_str() << endl; }
        set_cache_reader(all);
        all.example_parser->resettable = true;
      }
    }
  }

  all.parse_mask = (static_cast<uint64_t>(1) << all.num_bits) - 1;
  if (cache_files.size() == 0)
  {
    if (!quiet) { *(all.trace_message) << "using no cache" << endl; }
  }
}

// For macs
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
#endif

void VW::details::enable_sources(
    VW::workspace& all, bool quiet, size_t passes, const VW::details::input_options& input_options)
{
  parse_cache(all, input_options.cache_files, input_options.kill_cache, quiet);

  // default text reader
  all.example_parser->text_reader = VW::parsers::text::read_lines;

  if (!input_options.no_daemon && (all.daemon || all.active))
  {
#ifdef _WIN32
    WSAData wsaData;
    int lastError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (lastError != 0) THROWERRNO("WSAStartup() returned error:" << lastError);
#endif
    all.example_parser->bound_sock = static_cast<int>(socket(PF_INET, SOCK_STREAM, 0));
    if (all.example_parser->bound_sock < 0) { THROW(fmt::format("socket: {}", VW::io::strerror_to_string(errno))); }

    int on = 1;
    if (setsockopt(all.example_parser->bound_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&on), sizeof(on)) <
        0)
    {
      *(all.trace_message) << "setsockopt SO_REUSEADDR: " << VW::io::strerror_to_string(errno) << endl;
    }

    // Enable TCP Keep Alive to prevent socket leaks
    int enable_tka = 1;
    if (setsockopt(all.example_parser->bound_sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&enable_tka),
            sizeof(enable_tka)) < 0)
    {
      *(all.trace_message) << "setsockopt SO_KEEPALIVE: " << VW::io::strerror_to_string(errno) << endl;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    short unsigned int port = 26542;
    if (all.options->was_supplied("port")) { port = static_cast<uint16_t>(input_options.port); }
    address.sin_port = htons(port);

    // attempt to bind to socket
    if (::bind(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
    {
      THROWERRNO("bind");
    }

    // listen on socket
    if (listen(all.example_parser->bound_sock, 1) < 0) { THROWERRNO("listen"); }

    // write port file
    if (all.options->was_supplied("port_file"))
    {
      socklen_t address_size = sizeof(address);
      if (getsockname(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&address), &address_size) < 0)
      {
        *(all.trace_message) << "getsockname: " << VW::io::strerror_to_string(errno) << endl;
      }
      std::ofstream port_file;
      port_file.open(input_options.port_file.c_str());
      if (!port_file.is_open()) THROW("error writing port file: " << input_options.port_file);

      port_file << ntohs(address.sin_port) << endl;
      port_file.close();
    }

    // background process (if foreground is not set)
    if (!input_options.foreground)
    {
      // FIXME switch to posix_spawn
      VW_WARNING_STATE_PUSH
      VW_WARNING_DISABLE_DEPRECATED_USAGE
      if (!all.active && daemon(1, 1)) THROWERRNO("daemon");
      VW_WARNING_STATE_POP
    }

    // write pid file
    if (all.options->was_supplied("pid_file"))
    {
      std::ofstream pid_file;
      pid_file.open(input_options.pid_file.c_str());
      if (!pid_file.is_open()) { THROW("error writing pid file"); }
      pid_file << VW::get_pid() << endl;
      pid_file.close();
    }

    if (all.daemon && !all.active)
    {
      // See support notes here: https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Daemon-example
#ifdef __APPLE__
      all.logger.warn("daemon mode is not supported on MacOS.");
#endif

#ifdef _WIN32
      THROW("daemon mode is not supported on Windows");
#else
      fclose(stdin);
      // weights will be shared across processes, accessible to children
      all.weights.share(all.length());

      // learning state to be shared across children
      size_t mmap_length = sizeof(VW::shared_data);
      VW::shared_data* sd = static_cast<VW::shared_data*>(
          mmap(nullptr, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
      // copy construct with placement new
      new (sd) VW::shared_data(*all.sd);
      all.sd = std::shared_ptr<VW::shared_data>(sd, [sd, mmap_length](void*) { munmap(sd, mmap_length); });

      // create children
      const auto num_children = VW::cast_to_smaller_type<size_t>(input_options.num_children);
      VW::v_array<int> children;
      children.resize(num_children);
      for (size_t i = 0; i < num_children; i++)
      {
        // fork() returns pid if parent, 0 if child
        // store fork value and run child process if child
        if ((children[i] = fork()) == 0)
        {
          all.quiet |= (i > 0);
          goto child;
        }
      }

      // install signal handler so we can kill children when killed
      {
        class sigaction sa;
        // specifically don't set SA_RESTART in sa.sa_flags, so that
        // waitid will be interrupted by SIGTERM with handler installed
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handle_sigterm;
        sigaction(SIGTERM, &sa, nullptr);
      }

      while (true)
      {
        // wait for child to change state; if finished, then respawn
        int status;
        pid_t pid = wait(&status);

        // If the child failed we still fork off another one, but log the issue.
        if (status != 0)
        {
          all.logger.err_warn("Daemon child process received exited with non-zero exit code: {}. Ignoring.", status);
        }

        if (got_sigterm)
        {
          for (size_t i = 0; i < num_children; i++) { kill(children[i], SIGTERM); }
          all.finish();
          delete &all;
          std::exit(0);
        }
        if (pid < 0) { continue; }
        for (size_t i = 0; i < num_children; i++)
        {
          if (pid == children[i])
          {
            if ((children[i] = fork()) == 0)
            {
              all.quiet |= (i > 0);
              goto child;
            }
            break;
          }
        }
      }

#endif
    }

#ifndef _WIN32
  child:
#endif
    sockaddr_in client_address;
    socklen_t size = sizeof(client_address);
    if (!all.quiet) { *(all.trace_message) << "calling accept" << endl; }
    auto f_a =
        static_cast<int>(accept(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&client_address), &size));
    if (f_a < 0) THROWERRNO("accept");

    // Disable Nagle delay algorithm due to daemon mode's interactive workload
    int one = 1;
    setsockopt(f_a, SOL_TCP, TCP_NODELAY, reinterpret_cast<char*>(&one), sizeof(one));

    auto socket = VW::io::wrap_socket_descriptor(f_a);

    all.final_prediction_sink.push_back(socket->get_writer());

    all.example_parser->input.add_file(socket->get_reader());
    if (!all.quiet) { *(all.trace_message) << "reading data from port " << port << endl; }

    if (all.active) { set_string_reader(all); }
    else { set_daemon_reader(all, input_options.json, input_options.dsjson); }
    all.example_parser->resettable = all.example_parser->write_cache || all.daemon;
  }
  else
  {
    if (all.example_parser->input.num_files() != 0)
    {
      if (!quiet) { *(all.trace_message) << "ignoring text input in favor of cache input" << endl; }
    }
    else
    {
      std::string filename_to_read = all.data_filename;
      std::string input_name = filename_to_read;
      auto should_use_compressed = input_options.compressed || VW::ends_with(filename_to_read, ".gz");

      try
      {
        std::unique_ptr<VW::io::reader> adapter;
        if (!filename_to_read.empty())
        {
          adapter = should_use_compressed ? VW::io::open_compressed_file_reader(filename_to_read)
                                          : VW::io::open_file_reader(filename_to_read);
        }
        else if (!input_options.stdin_off)
        {
          input_name = "stdin";
          // Should try and use stdin
          if (should_use_compressed) { adapter = VW::io::open_compressed_stdin(); }
          else { adapter = VW::io::open_stdin(); }
        }
        else
        {
          // Stdin is off and no file was passed.
          input_name = "none";
        }

        if (!quiet) { *(all.trace_message) << "Reading datafile = " << input_name << endl; }

        if (adapter) { all.example_parser->input.add_file(std::move(adapter)); }
      }
      catch (std::exception const& ex)
      {
        THROW("Failed to open input data file '" << filename_to_read << "'. Inner error: " << ex.what());
      }

      if (input_options.json || input_options.dsjson) { set_json_reader(all, input_options.dsjson); }
#ifdef BUILD_FLATBUFFERS
      else if (input_options.flatbuffer)
      {
        all.flat_converter = VW::make_unique<VW::parsers::flatbuffer::parser>();
        all.example_parser->reader = VW::parsers::flatbuffer::flatbuffer_to_examples;
      }
#endif
#ifdef VW_BUILD_CSV
      else if (input_options.csv_opts && input_options.csv_opts->enabled)
      {
        all.custom_parser = VW::make_unique<VW::parsers::csv::csv_parser>(*input_options.csv_opts);
        all.example_parser->reader = VW::parsers::csv::parse_csv_examples;
      }
#endif
      else { set_string_reader(all); }

      all.example_parser->resettable = all.example_parser->write_cache;
      all.chain_hash_json = input_options.chain_hash_json;
    }
  }

  if (passes > 1 && !all.example_parser->resettable)
    THROW("need a cache file for multiple passes : try using  --cache or --cache_file <name>");

  if (!quiet && !all.daemon)
  {
    *(all.trace_message) << "num sources = " << all.example_parser->input.num_files() << endl;
  }
}

void VW::details::lock_done(parser& p)
{
  p.done = true;
  // in case get_example() is waiting for a fresh example, wake so it can realize there are no more.
  p.ready_parsed_examples.set_done();
}

void VW::details::set_done(VW::workspace& all)
{
  all.early_terminate = true;
  lock_done(*all.example_parser);
}

void end_pass_example(VW::workspace& all, VW::example* ae)
{
  all.example_parser->lbl_parser.default_label(ae->l);
  ae->end_pass = true;
  all.example_parser->in_pass_counter = 0;
}

namespace VW
{
VW::example& get_unused_example(VW::workspace* all)
{
  auto& p = *all->example_parser;
  auto* ex = p.example_pool.get_object().release();
  ex->example_counter = static_cast<size_t>(p.num_examples_taken_from_pool.fetch_add(1, std::memory_order_relaxed));
  return *ex;
}

}  // namespace VW

void VW::details::free_parser(VW::workspace& all)
{
  // It is possible to exit early when the queue is not yet empty.

  while (all.example_parser->ready_parsed_examples.size() > 0)
  {
    VW::example* current = nullptr;
    all.example_parser->ready_parsed_examples.try_pop(current);
    if (current != nullptr)
    {
      // this function also handles examples that were not from the pool.
      VW::finish_example(all, *current);
    }
  }

  // There should be no examples in flight at this point.
  assert(all.example_parser->ready_parsed_examples.size() == 0);
}
