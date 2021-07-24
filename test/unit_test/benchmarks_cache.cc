#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <queue>
#include <chrono>
#include <fstream>

#include "parse_example.cc"
#include "io_to_queue.h"
#include "parse_dispatch_loop.h"

#include <time.h> 

std::string read_string_from_file(const std::string &file_path) {
    const std::ifstream input_stream(file_path, std::ios_base::binary);

    if (input_stream.fail()) {
        throw std::runtime_error("Failed to open file");
    }

    std::stringstream buffer;
    buffer << input_stream.rdbuf();

    return buffer.str();
}

float run_io_and_parser(size_t num_parse_threads)
{
    using namespace std::chrono;

    vw* vw = VW::initialize("--no_stdin --quiet -c", nullptr , false, nullptr, nullptr);

    std::string text = read_string_from_file("../../0002_4million.dat");
    // add input as buffer_view so parses line by line
    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    size_t count = 4000001;
    std::thread mock_learner([&vw, &count]() 
    {
        while(count--)
        {
            auto ev = VW::get_example(vw->example_parser);
            auto ec = (*ev)[0];
            VW::finish_example(*vw, *ec);
            VW::finish_example_vector(*vw, *ev);
        }
        set_done(*vw);
    });
    auto start = high_resolution_clock::now();

    vw->example_parser->num_parse_threads = num_parse_threads;
    VW::start_io_thread(*vw);
    VW::start_parser(*vw);
    VW::end_parser(*vw);
    VW::end_io_thread(*vw);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    mock_learner.join();

    VW::finish(*vw);

    return duration.count()/1000000.0;
}

float run_parser_and_mock_learner(size_t num_parse_threads)
{
    using namespace std::chrono;

    vw* vw = VW::initialize("--no_stdin --quiet -c", nullptr , false, nullptr, nullptr);
    {
        auto start = high_resolution_clock::now();

        std::string text = read_string_from_file("../../0002_4million.dat");
        // add input as buffer_view so parses line by line
        vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

        char* line;
        while(!vw->example_parser->input_file_reader(*vw, line)){}

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        std::cout << "Time taken to read (io_thread): " << duration.count()/1000000.0 << " seconds" << std::endl;
    }
    // BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 1001);
    size_t count = vw->example_parser->io_lines.size();
    std::thread mock_learner([&vw, &count]() 
    {
        while(count--)
        {
            auto ev = VW::get_example(vw->example_parser);
            auto ec = (*ev)[0];
            VW::finish_example(*vw, *ec);
            VW::finish_example_vector(*vw, *ev);
        }
        set_done(*vw);
    });
    auto start = high_resolution_clock::now();

    vw->example_parser->num_parse_threads = num_parse_threads;
    VW::start_parser(*vw);
    VW::end_parser(*vw);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    mock_learner.join();

    VW::finish(*vw);
    return duration.count()/1000000.0;
}

float run_parser_and_learner(size_t num_parse_threads)
{
    using namespace std::chrono;

    vw* vw = VW::initialize("--no_stdin --quiet -c", nullptr , false, nullptr, nullptr);
    {
        auto start = high_resolution_clock::now();

        std::string text = read_string_from_file("../../0002_4million.dat");
        // add input as buffer_view so parses line by line
        vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

        char* line;
        while(!vw->example_parser->input_file_reader(*vw, line)){}

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        std::cout << "Time taken to read (io_thread): " << duration.count()/1000000.0 << " seconds" << std::endl;
    }
    // BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 1001);

    auto start = high_resolution_clock::now();

    vw->example_parser->num_parse_threads = num_parse_threads;
    VW::start_parser(*vw);
    VW::LEARNER::generic_driver(*vw);
    VW::end_parser(*vw);
    
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    VW::finish(*vw);
    return duration.count()/1000000.0;
}

BOOST_AUTO_TEST_CASE(creating_cache)
{
    std::cout<<"creating cache file\n";
    run_parser_and_learner(32);
}

BOOST_AUTO_TEST_CASE(test_io_and_parser_cache)
{
    std::cout<<"IO_and_PARSER\n";
    for(size_t i=1; i<=8; i++)
        std::cout << "Total Time taken: " << run_io_and_parser(i) << " seconds, by parser_threads = " << i << std::endl;
}

BOOST_AUTO_TEST_CASE(test_parser_and_mock_learner_cache)
{
    std::cout<<"PARSER_and_MOCK_LEARNER\n";
    for(size_t i=1; i<=8; i++)
        std::cout << "Total Time taken: " << run_parser_and_mock_learner(i) << " seconds, by parser_threads = " << i << std::endl;
}

BOOST_AUTO_TEST_CASE(test_parser_and_learner_cache)
{
    std::cout<<"PARSER_and_LEARNER\n";
    for(size_t i=1; i<=8; i++)
        std::cout << "Total Time taken: " << run_parser_and_learner(i) << " seconds, by parser_threads = " << i << std::endl;
}