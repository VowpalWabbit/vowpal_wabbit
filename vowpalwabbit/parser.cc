// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "parser.h"

#include <sys/types.h>
#include "io/logger.h"

#ifndef _WIN32
#  include <sys/mman.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  include <netinet/tcp.h>
#endif

#include <csignal>

#include <fstream>

#include "text_utils.h"
#include "crossplat_compat.h"

#ifdef _WIN32
#  define NOMINMAX
#  include <winsock2.h>
#  include <Windows.h>
#  include <io.h>
typedef int socklen_t;
// windows doesn't define SOL_TCP and use an enum for the later, so can't check for its presence with a macro.
#  define SOL_TCP IPPROTO_TCP

int daemon(int /*a*/, int /*b*/) { exit(0); }

// Starting with v142 the fix in the else block no longer works due to mismatching linkage. Going forward we should just
// use the actual isocpp version.
// use VW_getpid instead of getpid to avoid name collisions with process.h
#  if _MSC_VER >= 1920
#    define VW_getpid _getpid
#  else
int VW_getpid() { return (int)::GetCurrentProcessId(); }
#  endif

#else
#  include <netdb.h>
#  define VW_getpid getpid
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
#  include <netinet/in.h>
#endif

#include <cerrno>
#include <cstdio>
#include <cassert>

#include "parse_primitives.h"
#include "parse_example.h"
#include "cache.h"
#include "unique_sort.h"
#include "constant.h"
#include "vw.h"
#include "interactions.h"
#include "vw_exception.h"
#include "parse_example_json.h"
#include "parse_dispatch_loop.h"
#include "parse_args.h"
#include "io/io_adapter.h"
#ifdef BUILD_FLATBUFFERS
#  include "parser/flatbuffer/parse_example_flatbuffer.h"
#endif

#ifdef BUILD_EXTERNAL_PARSER
#  include "parse_example_external.h"
#endif

// OSX doesn't expects you to use IPPROTO_TCP instead of SOL_TCP
#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#  define SOL_TCP IPPROTO_TCP
#endif

using std::endl;

// This should not? matter in a library mode.
bool got_sigterm;

void handle_sigterm(int) { got_sigterm = true; }

namespace VW
{
void parse_example_label(string_view label, const label_parser& lbl_parser, const named_labels* ldict,
    label_parser_reuse_mem& reuse_mem, example& ec)
{
  std::vector<string_view> words;
  tokenize(' ', label, words);
  lbl_parser.parse_label(ec.l, ec._reduction_features, reuse_mem, ldict, words);
}
}  // namespace VW

bool is_test_only(uint32_t counter, uint32_t period, uint32_t after, bool holdout_off,
    uint32_t target_modulus)  // target should be 0 in the normal case, or period-1 in the case that emptylines separate
                              // examples
{
  if (holdout_off) return false;
  if (after == 0)  // hold out by period
    return (counter % period == target_modulus);
  else  // hold out by position
    return (counter > after);
}

uint32_t cache_numbits(VW::io::reader& cache_reader)
{
  size_t version_buffer_length;
  if (static_cast<size_t>(cache_reader.read(reinterpret_cast<char*>(&version_buffer_length),
          sizeof(version_buffer_length))) < sizeof(version_buffer_length))
  { THROW("failed to read: version_buffer_length"); }

  if (version_buffer_length > 61) THROW("cache version too long, cache file is probably invalid");
  if (version_buffer_length == 0) THROW("cache version too short, cache file is probably invalid");

  std::vector<char> version_buffer(version_buffer_length);
  if (static_cast<size_t>(cache_reader.read(version_buffer.data(), version_buffer_length)) < version_buffer_length)
  { THROW("failed to read: version buffer"); }
  VW::version_struct cache_version(version_buffer.data());
  if (cache_version != VW::version)
  {
    auto msg = fmt::format(
        "Cache file version does not match current VW version. Cache files must be produced by the version consuming "
        "them. Cache version: {} VW version: {}",
        cache_version.to_string(), VW::version.to_string());
    THROW(msg);
  }

  char marker;
  if (static_cast<size_t>(cache_reader.read(&marker, sizeof(marker))) < sizeof(marker)) { THROW("failed to read"); }

  if (marker != 'c') THROW("data file is not a cache file");

  uint32_t cache_numbits;
  if (static_cast<size_t>(cache_reader.read(reinterpret_cast<char*>(&cache_numbits), sizeof(cache_numbits))) <
      sizeof(cache_numbits))
  { THROW("failed to read"); }

  return cache_numbits;
}

void set_cache_reader(VW::workspace& all) { all.example_parser->reader = read_cached_features; }

void set_string_reader(VW::workspace& all)
{
  all.example_parser->reader = read_features_string;
  all.print_by_ref = print_result_by_ref;
}

bool is_currently_json_reader(const VW::workspace& all)
{
  return all.example_parser->reader == &read_features_json<true> ||
      all.example_parser->reader == &read_features_json<false>;
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
    all.example_parser->reader = &read_features_json<true>;
    all.example_parser->text_reader = &line_to_examples_json<true>;
    all.example_parser->audit = true;
  }
  else
  {
    all.example_parser->reader = &read_features_json<false>;
    all.example_parser->text_reader = &line_to_examples_json<false>;
    all.example_parser->audit = false;
  }

  all.example_parser->decision_service_json = dsjson;

  if (dsjson && all.options->was_supplied("extra_metrics"))
  { all.example_parser->metrics = VW::make_unique<dsjson_metrics>(); }
}

void set_daemon_reader(VW::workspace& all, bool json = false, bool dsjson = false)
{
  if (all.example_parser->input.isbinary())
  {
    all.example_parser->reader = read_cached_features;
    all.print_by_ref = binary_print_result_by_ref;
  }
  else if (json || dsjson)
  {
    set_json_reader(all, dsjson);
  }
  else
  {
    set_string_reader(all);
  }
}

void reset_source(VW::workspace& all, size_t numbits)
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
        all.example_parser->output_done.wait(lock, [&] {
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
      if (f < 0) THROW("accept: " << VW::strerror_to_string(errno));

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
        if (cache_numbits(*file) < numbits) { THROW("argh, a bug in caching of some sort!") }
      }
    }
  }
}

void make_write_cache(VW::workspace& all, std::string& newname, bool quiet)
{
  io_buf& output = all.example_parser->output;
  if (output.num_files() != 0)
  {
    *(all.trace_message) << "Warning: you tried to make two write caches.  Only the first one will be made." << endl;
    return;
  }

  all.example_parser->currentname = newname + std::string(".writing");
  try
  {
    output.add_file(VW::io::open_file_writer(all.example_parser->currentname));
  }
  catch (const std::exception&)
  {
    *(all.trace_message) << "can't create cache file !" << all.example_parser->currentname << endl;
    return;
  }

  size_t v_length = static_cast<uint64_t>(VW::version.to_string().length()) + 1;

  output.bin_write_fixed(reinterpret_cast<const char*>(&v_length), sizeof(v_length));
  output.bin_write_fixed(VW::version.to_string().c_str(), v_length);
  output.bin_write_fixed("c", 1);
  output.bin_write_fixed(reinterpret_cast<const char*>(&all.num_bits), sizeof(all.num_bits));
  output.flush();

  all.example_parser->finalname = newname;
  all.example_parser->write_cache = true;
  if (!quiet) *(all.trace_message) << "creating cache_file = " << newname << endl;
}

void parse_cache(VW::workspace& all, std::vector<std::string> cache_files, bool kill_cache, bool quiet)
{
  all.example_parser->write_cache = false;

  for (auto& file : cache_files)
  {
    bool cache_file_opened = false;
    if (!kill_cache) try
      {
        all.example_parser->input.add_file(VW::io::open_file_reader(file));
        cache_file_opened = true;
      }
      catch (const std::exception&)
      {
        cache_file_opened = false;
      }
    if (cache_file_opened == false)
      make_write_cache(all, file, quiet);
    else
    {
      uint64_t c = cache_numbits(*all.example_parser->input.get_input_files().back());
      if (c < all.num_bits)
      {
        if (!quiet)
          *(all.trace_message) << "WARNING: cache file is ignored as it's made with less bit precision than required!"
                               << endl;
        all.example_parser->input.close_file();
        make_write_cache(all, file, quiet);
      }
      else
      {
        if (!quiet) *(all.trace_message) << "using cache_file = " << file.c_str() << endl;
        set_cache_reader(all);
        if (c == all.num_bits)
          all.example_parser->sorted_cache = true;
        else
          all.example_parser->sorted_cache = false;
        all.example_parser->resettable = true;
      }
    }
  }

  all.parse_mask = (static_cast<uint64_t>(1) << all.num_bits) - 1;
  if (cache_files.size() == 0)
  {
    if (!quiet) *(all.trace_message) << "using no cache" << endl;
  }
}

// For macs
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
#endif

void enable_sources(VW::workspace& all, bool quiet, size_t passes, input_options& input_options)
{
  parse_cache(all, input_options.cache_files, input_options.kill_cache, quiet);

  // default text reader
  all.example_parser->text_reader = VW::read_lines;

  if (!all.no_daemon && (all.daemon || all.active))
  {
#ifdef _WIN32
    WSAData wsaData;
    int lastError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (lastError != 0) THROWERRNO("WSAStartup() returned error:" << lastError);
#endif
    all.example_parser->bound_sock = static_cast<int>(socket(PF_INET, SOCK_STREAM, 0));
    if (all.example_parser->bound_sock < 0)
    {
      std::stringstream msg;
      msg << "socket: " << VW::strerror_to_string(errno);
      *(all.trace_message) << msg.str() << endl;
      THROW(msg.str().c_str());
    }

    int on = 1;
    if (setsockopt(all.example_parser->bound_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&on), sizeof(on)) <
        0)
      *(all.trace_message) << "setsockopt SO_REUSEADDR: " << VW::strerror_to_string(errno) << endl;

    // Enable TCP Keep Alive to prevent socket leaks
    int enableTKA = 1;
    if (setsockopt(all.example_parser->bound_sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&enableTKA),
            sizeof(enableTKA)) < 0)
      *(all.trace_message) << "setsockopt SO_KEEPALIVE: " << VW::strerror_to_string(errno) << endl;

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    short unsigned int port = 26542;
    if (all.options->was_supplied("port")) port = static_cast<uint16_t>(input_options.port);
    address.sin_port = htons(port);

    // attempt to bind to socket
    if (::bind(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
      THROWERRNO("bind");

    // listen on socket
    if (listen(all.example_parser->bound_sock, 1) < 0) THROWERRNO("listen");

    // write port file
    if (all.options->was_supplied("port_file"))
    {
      socklen_t address_size = sizeof(address);
      if (getsockname(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&address), &address_size) < 0)
      { *(all.trace_message) << "getsockname: " << VW::strerror_to_string(errno) << endl; }
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
      if (!all.active && daemon(1, 1)) THROWERRNO("daemon");
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
#ifdef _WIN32
      THROW("not supported on windows");
#else
      fclose(stdin);
      // weights will be shared across processes, accessible to children
      all.weights.share(all.length());

      // learning state to be shared across children
      shared_data* sd = static_cast<shared_data*>(
          mmap(nullptr, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
      new (sd) shared_data(*all.sd);
      free(all.sd);
      all.sd = sd;
      all.example_parser->_shared_data = sd;

      // create children
      size_t num_children = all.num_children;
      v_array<int> children;
      children.resize_but_with_stl_behavior(num_children);
      for (size_t i = 0; i < num_children; i++)
      {
        // fork() returns pid if parent, 0 if child
        // store fork value and run child process if child
        if ((children[i] = fork()) == 0)
        {
          all.logger.quiet |= (i > 0);
          goto child;
        }
      }

      // install signal handler so we can kill children when killed
      {
        struct sigaction sa;
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
          VW::io::logger::errlog_warn(
              "Daemon child process received exited with non-zero exit code: {}. Ignoring.", status);
        }

        if (got_sigterm)
        {
          for (size_t i = 0; i < num_children; i++) kill(children[i], SIGTERM);
          VW::finish(all);
          exit(0);
        }
        if (pid < 0) continue;
        for (size_t i = 0; i < num_children; i++)
          if (pid == children[i])
          {
            if ((children[i] = fork()) == 0)
            {
              all.logger.quiet |= (i > 0);
              goto child;
            }
            break;
          }
      }

#endif
    }

#ifndef _WIN32
  child:
#endif
    sockaddr_in client_address;
    socklen_t size = sizeof(client_address);
    if (!all.logger.quiet) *(all.trace_message) << "calling accept" << endl;
    auto f_a =
        static_cast<int>(accept(all.example_parser->bound_sock, reinterpret_cast<sockaddr*>(&client_address), &size));
    if (f_a < 0) THROWERRNO("accept");

    // Disable Nagle delay algorithm due to daemon mode's interactive workload
    int one = 1;
    setsockopt(f_a, SOL_TCP, TCP_NODELAY, reinterpret_cast<char*>(&one), sizeof(one));

    auto socket = VW::io::wrap_socket_descriptor(f_a);

    all.final_prediction_sink.push_back(socket->get_writer());

    all.example_parser->input.add_file(socket->get_reader());
    if (!all.logger.quiet) *(all.trace_message) << "reading data from port " << port << endl;

    if (all.active) { set_string_reader(all); }
    else
    {
      all.example_parser->sorted_cache = true;
      set_daemon_reader(all, input_options.json, input_options.dsjson);
      all.example_parser->sorted_cache = true;
    }
    all.example_parser->resettable = all.example_parser->write_cache || all.daemon;
  }
  else
  {
    if (all.example_parser->input.num_files() != 0)
    {
      if (!quiet) *(all.trace_message) << "ignoring text input in favor of cache input" << endl;
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
        else if (!all.stdin_off)
        {
          input_name = "stdin";
          // Should try and use stdin
          if (should_use_compressed) { adapter = VW::io::open_compressed_stdin(); }
          else
          {
            adapter = VW::io::open_stdin();
          }
        }
        else
        {
          // Stdin is off and no file was passed.
          input_name = "none";
        }

        if (!quiet) { *(all.trace_message) << "Reading datafile = " << input_name << endl; }

        if (adapter) { all.example_parser->input.add_file(std::move(adapter)); }
      }
      catch (std::exception const&)
      {
        // when trying to fix this exception, consider that an empty filename_to_read is valid if all.stdin_off is false
        if (!filename_to_read.empty())
        { *(all.trace_message) << "can't open '" << filename_to_read << "', sailing on!" << endl; }
        else
        {
          throw;
        }
      }

      if (input_options.json || input_options.dsjson)
      {
        if (!input_options.chain_hash_json)
        {
          *(all.trace_message)
              << "WARNING: Old string feature value behavior is deprecated in JSON/DSJSON and will be removed in a "
                 "future version. Use `--chain_hash` to use new behavior and silence this warning."
              << endl;
        }
        set_json_reader(all, input_options.dsjson);
      }
#ifdef BUILD_FLATBUFFERS
      else if (input_options.flatbuffer)
      {
        all.flat_converter = VW::make_unique<VW::parsers::flatbuffer::parser>();
        all.example_parser->reader = VW::parsers::flatbuffer::flatbuffer_to_examples;
      }
#endif

#ifdef BUILD_EXTERNAL_PARSER
      else if (input_options.ext_opts && input_options.ext_opts->is_enabled())
      {
        all.external_parser = VW::external::parser::get_external_parser(&all, input_options);
        all.example_parser->reader = VW::external::parse_examples;
      }
#endif
      else
      {
        set_string_reader(all);
      }

      all.example_parser->resettable = all.example_parser->write_cache;
      all.chain_hash_json = input_options.chain_hash_json;
    }
  }

  if (passes > 1 && !all.example_parser->resettable)
    THROW("need a cache file for multiple passes : try using  --cache or --cache_file <name>");

  if (!quiet && !all.daemon) *(all.trace_message) << "num sources = " << all.example_parser->input.num_files() << endl;
}

void lock_done(parser& p)
{
  p.done = true;
  // in case get_example() is waiting for a fresh example, wake so it can realize there are no more.
  p.ready_parsed_examples.set_done();
}

void set_done(VW::workspace& all)
{
  all.early_terminate = true;
  lock_done(*all.example_parser);
}

void end_pass_example(VW::workspace& all, example* ae)
{
  all.example_parser->lbl_parser.default_label(ae->l);
  ae->end_pass = true;
  all.example_parser->in_pass_counter = 0;
}

void feature_limit(VW::workspace& all, example* ex)
{
  for (namespace_index index : ex->indices)
    if (all.limit[index] < ex->feature_space[index].size())
    {
      features& fs = ex->feature_space[index];
      fs.sort(all.parse_mask);
      unique_features(fs, all.limit[index]);
    }
}

namespace VW
{
example& get_unused_example(VW::workspace* all)
{
  parser* p = all->example_parser;
  auto* ex = p->example_pool.get_object();
  ex->example_counter = static_cast<size_t>(p->num_examples_taken_from_pool.fetch_add(1, std::memory_order_relaxed));
  return *ex;
}

void setup_examples(VW::workspace& all, v_array<example*>& examples)
{
  for (example* ae : examples) setup_example(all, ae);
}

void setup_example(VW::workspace& all, example* ae)
{
  if (all.example_parser->sort_features && ae->sorted == false) unique_sort_features(all.parse_mask, ae);

  if (all.example_parser->write_cache)
  {
    VW::write_example_to_cache(all.example_parser->output, ae, all.example_parser->lbl_parser, all.parse_mask,
        all.example_parser->_cache_temp_buffer);
  }

  // Require all extents to be complete in an example.
#ifndef NDEBUG
  for (auto& fg : *ae) { assert(fg.validate_extents()); }
#endif

  ae->partial_prediction = 0.;
  ae->num_features = 0;
  ae->reset_total_sum_feat_sq();
  ae->loss = 0.;
  ae->_debug_current_reduction_depth = 0;
  ae->use_permutations = all.permutations;

  all.example_parser->num_setup_examples++;
  if (!all.example_parser->emptylines_separate_examples) all.example_parser->in_pass_counter++;

  // Determine if this example is part of the holdout set.
  ae->test_only = is_test_only(all.example_parser->in_pass_counter, all.holdout_period, all.holdout_after,
      all.holdout_set_off, all.example_parser->emptylines_separate_examples ? (all.holdout_period - 1) : 0);
  // If this example has a test only label then it is true regardless.
  ae->test_only |= all.example_parser->lbl_parser.test_label(ae->l);

#ifdef PRIVACY_ACTIVATION
  if (all.privacy_activation)
  { ae->tag_hash = hashall(ae->tag.begin(), ae->tag.size(), all.hash_seed) % all.feature_bitset_size; }
#endif

  if (all.example_parser->emptylines_separate_examples &&
      (example_is_newline(*ae) &&
          (all.example_parser->lbl_parser.label_type != label_type_t::ccb || CCB::ec_is_example_unset(*ae))))
    all.example_parser->in_pass_counter++;

  ae->weight = all.example_parser->lbl_parser.get_weight(ae->l, ae->_reduction_features);

  if (all.ignore_some)
  {
    for (unsigned char* i = ae->indices.begin(); i != ae->indices.end(); i++)
    {
      if (all.ignore[*i])
      {
        // Delete namespace
        ae->feature_space[*i].clear();
        i = ae->indices.erase(i);
        // Offset the increment for this iteration so that is processes this index again which is actually the next
        // item.
        i--;
      }
    }
  }

  if (all.skip_gram_transformer != nullptr) { all.skip_gram_transformer->generate_grams(ae); }

  if (all.add_constant)  // add constant feature
    VW::add_constant_feature(all, ae);

  if (!all.limit_strings.empty()) feature_limit(all, ae);

  uint64_t multiplier = static_cast<uint64_t>(all.wpp) << all.weights.stride_shift();

  if (multiplier != 1)  // make room for per-feature information.
    for (features& fs : *ae)
      for (auto& j : fs.indices) j *= multiplier;
  ae->num_features = 0;
  for (const features& fs : *ae)
  {
    ae->num_features += fs.size();
  }

  // Set the interactions for this example to the global set.
  ae->interactions = &all.interactions;
  ae->extent_interactions = &all.extent_interactions;
}
}  // namespace VW

namespace VW
{
example* new_unused_example(VW::workspace& all)
{
  example* ec = &get_unused_example(&all);
  all.example_parser->lbl_parser.default_label(ec->l);
  return ec;
}

example* read_example(VW::workspace& all, const char* example_line)
{
  example* ret = &get_unused_example(&all);

  VW::read_line(all, ret, example_line);
  setup_example(all, ret);

  return ret;
}

example* read_example(VW::workspace& all, const std::string& example_line)
{
  return read_example(all, example_line.c_str());
}

void add_constant_feature(VW::workspace& vw, example* ec)
{
  ec->indices.push_back(constant_namespace);
  ec->feature_space[constant_namespace].push_back(1, constant, constant_namespace);
  ec->num_features++;
  if (vw.audit || vw.hash_inv)
    ec->feature_space[constant_namespace].space_names.push_back(audit_strings("", "Constant"));
}

void add_label(example* ec, float label, float weight, float base)
{
  ec->l.simple.label = label;
  auto& simple_red_features = ec->_reduction_features.template get<simple_label_reduction_features>();
  simple_red_features.initial = base;
  ec->weight = weight;
}

example* import_example(VW::workspace& all, const std::string& label, primitive_feature_space* features, size_t len)
{
  example* ret = &get_unused_example(&all);
  all.example_parser->lbl_parser.default_label(ret->l);

  if (label.length() > 0) parse_example_label(all, *ret, label);

  for (size_t i = 0; i < len; i++)
  {
    unsigned char index = features[i].name;
    ret->indices.push_back(index);
    for (size_t j = 0; j < features[i].len; j++)
      ret->feature_space[index].push_back(features[i].fs[j].x, features[i].fs[j].weight_index);
  }

  setup_example(all, ret);
  return ret;
}

primitive_feature_space* export_example(VW::workspace& all, example* ec, size_t& len)
{
  len = ec->indices.size();
  primitive_feature_space* fs_ptr = new primitive_feature_space[len];

  size_t fs_count = 0;

  for (size_t idx = 0; idx < len; ++idx)
  {
    namespace_index i = ec->indices[idx];
    fs_ptr[fs_count].name = i;
    fs_ptr[fs_count].len = ec->feature_space[i].size();
    fs_ptr[fs_count].fs = new feature[fs_ptr[fs_count].len];

    uint32_t stride_shift = all.weights.stride_shift();

    auto& f = ec->feature_space[i];
    for (size_t f_count = 0; f_count < fs_ptr[fs_count].len; f_count++)
    {
      feature t = {f.values[f_count], f.indices[f_count]};
      t.weight_index >>= stride_shift;
      fs_ptr[fs_count].fs[f_count] = t;
    }
    fs_count++;
  }
  return fs_ptr;
}

void releaseFeatureSpace(primitive_feature_space* features, size_t len)
{
  for (size_t i = 0; i < len; i++) delete[] features[i].fs;
  delete (features);
}

void parse_example_label(VW::workspace& all, example& ec, const std::string& label)
{
  std::vector<VW::string_view> words;
  tokenize(' ', label, words);
  all.example_parser->lbl_parser.parse_label(
      ec.l, ec._reduction_features, all.example_parser->parser_memory_to_reuse, all.sd->ldict.get(), words);
}

void empty_example(VW::workspace& /*all*/, example& ec)
{
  for (features& fs : ec) fs.clear();

  ec.indices.clear();
  ec.tag.clear();
#ifdef PRIVACY_ACTIVATION
  ec.tag_hash = 0;
#endif
  ec.sorted = false;
  ec.end_pass = false;
  ec.is_newline = false;
  ec._reduction_features.clear();
  ec.num_features_from_interactions = 0;
}

void clean_example(VW::workspace& all, example& ec)
{
  empty_example(all, ec);
  all.example_parser->example_pool.return_object(&ec);
}

void finish_example(VW::workspace& all, example& ec)
{
  // only return examples to the pool that are from the pool and not externally allocated
  if (!is_ring_example(all, &ec)) return;

  clean_example(all, ec);

  {
    std::lock_guard<std::mutex> lock(all.example_parser->output_lock);
    ++all.example_parser->num_finished_examples;
    all.example_parser->output_done.notify_one();
  }
}
}  // namespace VW

void thread_dispatch(VW::workspace& all, const v_array<example*>& examples)
{
  for (auto example : examples) { all.example_parser->ready_parsed_examples.push(example); }
}

void main_parse_loop(VW::workspace* all) { parse_dispatch(*all, thread_dispatch); }

namespace VW
{
example* get_example(parser* p) { return p->ready_parsed_examples.pop(); }

float get_topic_prediction(example* ec, size_t i) { return ec->pred.scalars[i]; }

float get_label(example* ec) { return ec->l.simple.label; }

float get_importance(example* ec) { return ec->weight; }

float get_initial(example* ec)
{
  const auto& simple_red_features = ec->_reduction_features.template get<simple_label_reduction_features>();
  return simple_red_features.initial;
}

float get_prediction(example* ec) { return ec->pred.scalar; }

float get_cost_sensitive_prediction(example* ec) { return static_cast<float>(ec->pred.multiclass); }

v_array<float>& get_cost_sensitive_prediction_confidence_scores(example* ec) { return ec->pred.scalars; }

uint32_t* get_multilabel_predictions(example* ec, size_t& len)
{
  MULTILABEL::labels labels = ec->pred.multilabels;
  len = labels.label_v.size();
  return labels.label_v.begin();
}

float get_action_score(example* ec, size_t i)
{
  ACTION_SCORE::action_scores scores = ec->pred.a_s;

  if (i < scores.size()) { return scores[i].score; }
  else
  {
    return 0.0;
  }
}

size_t get_action_score_length(example* ec) { return ec->pred.a_s.size(); }

size_t get_tag_length(example* ec) { return ec->tag.size(); }

const char* get_tag(example* ec) { return ec->tag.begin(); }

size_t get_feature_number(example* ec) { return ec->get_num_features(); }

float get_confidence(example* ec) { return ec->confidence; }
}  // namespace VW

namespace VW
{
void start_parser(VW::workspace& all) { all.parse_thread = std::thread(main_parse_loop, &all); }
}  // namespace VW

void free_parser(VW::workspace& all)
{
  // It is possible to exit early when the queue is not yet empty.

  while (all.example_parser->ready_parsed_examples.size() > 0)
  {
    auto* current = all.example_parser->ready_parsed_examples.pop();
    // this function also handles examples that were not from the pool.
    VW::finish_example(all, *current);
  }

  // There should be no examples in flight at this point.
  assert(all.example_parser->ready_parsed_examples.size() == 0);
}

namespace VW
{
void end_parser(VW::workspace& all) { all.parse_thread.join(); }

bool is_ring_example(const VW::workspace& all, const example* ae)
{
  return all.example_parser->example_pool.is_from_pool(ae);
}
}  // namespace VW
