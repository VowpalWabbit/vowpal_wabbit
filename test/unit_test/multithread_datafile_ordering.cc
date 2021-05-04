#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <queue>
#include <chrono>

#include "io_to_queue.h"

#include <time.h> 

#include <vector>


void mock_io_lines_to_queue(vw *vw){
    
    // Mock out io lines before, since we aren't testing io-parser communication and instead want to set up the io beforehand for the parser
    std::string text = "0 | price:.23 sqft:.25 age:.05 2006\n1 | price:.18 sqft:.15 age:.35 1976\n0 | price:.53 sqft:.32 age:.87 1924\n0 | price:.23 sqft:.25 age:.05 2006\n\n";

    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    io_lines_toqueue(*vw);

}

void run_thread_function(vw*& vw2, VW::ptr_queue<std::string>& input_lines, std::mutex& mut) {

    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw2));

    std::vector<VW::string_view> words;
    std::vector<VW::string_view> parse_name;

    std::atomic<bool> should_continue{true};

    while(should_continue){

        if(input_lines.size() == 0) {
            should_continue = false;
            input_lines.set_done();
            break;
        }

        {
            std::lock_guard<std::mutex> lock(mut);
            
            int size = input_lines.size();

            // Copy, since front returns a reference, which we will remove
            std::string next_line = *input_lines.pop();

            int num_chars_returned = read_features_string(vw2, examples, words, parse_name);

            // causes someitmes failure.
            BOOST_CHECK_EQUAL(num_chars_returned, next_line.size());

            BOOST_CHECK_EQUAL(vw2->example_parser->io_lines.size(), --size);

        }

    }

    BOOST_CHECK_EQUAL(vw2->example_parser->io_lines.size(), 0);

}

BOOST_AUTO_TEST_CASE(parse_text_one_thread)
{

    VW::ptr_queue<std::string> input_lines{5};
    input_lines.push(new std::string("0 | price:.23 sqft:.25 age:.05 2006\n"));
    input_lines.push(new std::string("1 | price:.18 sqft:.15 age:.35 1976\n"));
    input_lines.push(new std::string("0 | price:.53 sqft:.32 age:.87 1924\n"));
    input_lines.push(new std::string("0 | price:.23 sqft:.25 age:.05 2006\n"));
    input_lines.push(new std::string("\n"));

    auto vw2 = VW::initialize("--num_parse_threads=1 --no_stdin --quiet", nullptr, false, nullptr, nullptr);
    // Mock out io lines before, since we aren't testing io-parser communication and instead want to set up the io beforehand for the parser
    mock_io_lines_to_queue(vw2);

    std::mutex mut;

    std::thread parse_thread_one = std::thread([&vw2, &input_lines, &mut]() {
        run_thread_function(vw2, input_lines, mut);
    });

    parse_thread_one.join();

    // TODO: Check that the output corresponds to ready_examples_queue examples.

}


// Note: we sometimes get seg faults. Work-in-progress.
BOOST_AUTO_TEST_CASE(parse_text_two_threads)
{

    // Use std list so we can pop front
    VW::ptr_queue<std::string> input_lines{5};
    input_lines.push(new std::string("0 | price:.23 sqft:.25 age:.05 2006\n"));
    input_lines.push(new std::string("1 | price:.18 sqft:.15 age:.35 1976\n"));
    input_lines.push(new std::string("0 | price:.53 sqft:.32 age:.87 1924\n"));
    input_lines.push(new std::string("0 | price:.23 sqft:.25 age:.05 2006\n"));
    input_lines.push(new std::string("\n"));

    auto vw2 = VW::initialize("--num_parse_threads=1 --no_stdin --quiet", nullptr, false, nullptr, nullptr);
    // Mock out io lines before, since we aren't testing io-parser communication and instead want to set up the io beforehand for the parser
    mock_io_lines_to_queue(vw2);

    std::mutex mut;

    std::thread parse_thread_one =std::thread([&vw2, &input_lines, &mut]() {
        run_thread_function(vw2, input_lines, mut);
    });

    std::thread parse_thread_two =std::thread([&vw2, &input_lines, &mut]() {
        run_thread_function(vw2, input_lines, mut);
    });

    parse_thread_one.join();
    parse_thread_two.join();

    // TODO: Check that the output corresponds to ready_examples_queue examples.

}

BOOST_AUTO_TEST_CASE(parse_text_three_threads)
{

    // Use std list so we can pop front
    VW::ptr_queue<std::string> input_lines{5};
    input_lines.push(new std::string("0 | price:.23 sqft:.25 age:.05 2006\n"));
    input_lines.push(new std::string("1 | price:.18 sqft:.15 age:.35 1976\n"));
    input_lines.push(new std::string("0 | price:.53 sqft:.32 age:.87 1924\n"));
    input_lines.push(new std::string("0 | price:.23 sqft:.25 age:.05 2006\n"));
    input_lines.push(new std::string("\n"));

    auto vw2 = VW::initialize("--num_parse_threads=1 --no_stdin --quiet", nullptr, false, nullptr, nullptr);
    // Mock out io lines before, since we aren't testing io-parser communication and instead want to set up the io beforehand for the parser
    mock_io_lines_to_queue(vw2);

    std::mutex mut;

    std::thread parse_thread_one =std::thread([&vw2, &input_lines, &mut]() {
        run_thread_function(vw2, input_lines, mut);
    });

    std::thread parse_thread_two =std::thread([&vw2, &input_lines, &mut]() {
        run_thread_function(vw2, input_lines, mut);
    });

    std::thread parse_thread_three =std::thread([&vw2, &input_lines, &mut]() {
        run_thread_function(vw2, input_lines, mut);
    });


    parse_thread_one.join();
    parse_thread_two.join();
    parse_thread_three.join();

    // TODO: Check that the output corresponds to ready_examples_queue examples.

}
