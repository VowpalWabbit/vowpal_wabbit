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

// Initial, const copy of input for comparison
const std::vector<std::string> global_input = {"0 | price:.23 sqft:.25 age:.05 2006", "1 | price:.18 sqft:.15 age:.35 1976", "0 | price:.53 sqft:.32 age:.87 1924", "0 | price:.23 sqft:.25 age:.05 2006"};

std::string read_string_from_file(const std::string &file_path) {
    const std::ifstream input_stream(file_path, std::ios_base::binary);

    if (input_stream.fail()) {
        throw std::runtime_error("Failed to open file");
    }

    std::stringstream buffer;
    buffer << input_stream.rdbuf();

    return buffer.str();
}

void sleep_random_amt_of_time(){

    srand (time(NULL));

    int duration = rand() % 10;

    std::this_thread::sleep_for (std::chrono::milliseconds(duration));
}

// Mock the io state, by creating an io_state with 4 items in its queue. This io_state is a mock of the io_state information that example_parser->_io_state should hold.
void set_mock_iostate(vw *vw){

    std::vector<std::string> input; 

    input.push_back("0 | price:.23 sqft:.25 age:.05 2006"); 
    input.push_back("1 | price:.18 sqft:.15 age:.35 1976");  
    input.push_back("0 | price:.53 sqft:.32 age:.87 1924"); 
    input.push_back("0 | price:.23 sqft:.25 age:.05 2006"); 

    for(int i=0; i<input.size(); i++){
        BOOST_CHECK_EQUAL(input.at(i), global_input.at(i));
    }

    while(!input.empty()){
        std::vector<char> *vec = new std::vector<char>(input.back().begin(), input.back().end());
        vec->push_back('\0');
        vw->example_parser->io_lines.push(vec);
        input.pop_back();
    }

    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 4);

}

//Mock the input lines, i.e. put filler text as the input of the io_buf in the parser
void mock_io_lines(vw *vw){

    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

}

void mock_io_lines_empty(vw *vw){

    std::string text = "";

    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));
}

BOOST_AUTO_TEST_CASE(mock_out_io_to_queue)
{
    vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    // add input as buffer_view so parses line by line
    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    char* line;
    while(!vw->example_parser->input_file_reader(*vw, line)){}

    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 5);

    VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(empty_io_test)
{

    vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::string text = "";

    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    char* line;
    while(!vw->example_parser->input_file_reader(*vw, line)){}

    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 1);

    VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(io_lines_capture_test_single_thread)
{
    vw* vw = VW::initialize("--no_stdin --quiet --ring_size=1005", nullptr , false, nullptr, nullptr);

    std::string text = read_string_from_file("../test/train-sets/0002.dat");

    // add input as buffer_view so parses line by line
    vw->example_parser->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    char* line;
    while(!vw->example_parser->input_file_reader(*vw, line)){}

    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 1001);

    // Capturing the io_lines queue.
    while(vw->example_parser->io_lines.size())
    {
        auto examples = &VW::get_unused_example_vector(vw);
        examples->push_back(&VW::get_unused_example(vw));
        std::vector<char> *io_lines_next_item = nullptr;
        capture_io_lines_items(*vw, io_lines_next_item, examples);
    }

    BOOST_CHECK_EQUAL(vw->example_parser->ready_parsed_examples.size(), 1001);

    while(vw->example_parser->ready_parsed_examples.size())
    {
        auto ev = vw->example_parser->ready_parsed_examples.pop();
        auto ec = (*ev)[0];
        
        VW::finish_example(*vw, *ec);
        VW::finish_example_vector(*vw, *ev);
    }

    VW::finish(*vw);
}


// BOOST_AUTO_TEST_CASE(mock_out_pop_io)
// {
//     vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

//     set_mock_iostate(vw);

//     int curr_num_lines = vw->example_parser->io_lines.size();

//     std::list<std::string> expected_input_lines; 

//     expected_input_lines.push_back("0 | price:.23 sqft:.25 age:.05 2006"); 
//     expected_input_lines.push_back("1 | price:.18 sqft:.15 age:.35 1976");  
//     expected_input_lines.push_back("0 | price:.53 sqft:.32 age:.87 1924"); 
//     expected_input_lines.push_back("0 | price:.23 sqft:.25 age:.05 2006"); 

//     while(vw->example_parser->io_lines.size() > 0){

//         curr_num_lines--;

//         // The reason that we check that the message popped is equal to global_input at 
//         // curr_num_lines is because in set_mock_iostate, the methods push_back and pop_back 
//         // are called on the input lines, so the input lines are pushed in reverse to the io queue in vw's 
//         // io_state's io_lines. This is a trivial component of the checking, and still shows us
//         // that the io queue is being popped as desired.
       
//         std::vector<char> *next_line = vw->example_parser->io_lines.pop();
//         std::string next_expected_input_line = expected_input_lines.front();
//         expected_input_lines.pop_front();

//         // Work-in-progress: check this for fuller testing
//         //BOOST_CHECK_EQUAL(next_line->data(), next_expected_input_line.c_str());

//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), curr_num_lines);

//     }

//     BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 0);

//     VW::finish(*vw);

// }

// // Testing io and parser with 2 threads, sleeping on the parser thread for a random amount of time
// BOOST_AUTO_TEST_CASE(sleep_parser)
// {

//     vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
//     mock_io_lines(vw);

//     // The text used in mock_io_lines
//     std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
//     1 | price:.18 sqft:.15 age:.35 1976
//     0 | price:.53 sqft:.32 age:.87 1924
//     0 | price:.23 sqft:.25 age:.05 2006)";

//     std::thread io_queue_th([&vw]() 
//     {
//         io_lines_toqueue(*vw);

//         //io_lines size in io thread is 4
//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size() , 4);

//     });

//     // io_lines size in parse thread is 0
//     BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size() , 0);

//     //examples is unused in read_features_string, set empty
//     auto examples = v_init<example*>();
//     examples.push_back(&VW::get_unused_example(vw));

//     int size = vw->example_parser->io_lines.size();
//     v_array<VW::string_view> words= v_init<VW::string_view>();
//     v_array<VW::string_view> parse_name = v_init<VW::string_view>();

//     while(vw->example_parser->io_lines.size() > 0){

//         sleep_random_amt_of_time();

//         read_features_string(vw, examples, words, parse_name);

//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), --size);

//     }

//    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 0);

//    io_queue_th.join();

//    VW::finish(*vw);

// }

// // Testing io and parser with 2 threads, sleeping on the io thread for a random amount of time
// BOOST_AUTO_TEST_CASE(sleep_io)
// {

//     vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
//     mock_io_lines(vw);

//     // The text used in mock_io_lines
//     std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
//     1 | price:.18 sqft:.15 age:.35 1976
//     0 | price:.53 sqft:.32 age:.87 1924
//     0 | price:.23 sqft:.25 age:.05 2006)";

//     std::thread io_queue_th([&vw]() 
//     {
//         sleep_random_amt_of_time();
//         io_lines_toqueue(*vw);

//     });

//     //io_lines size in parse thread is 0
//     BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size() , 0);

//     //examples is unused in read_features_string, set empty
//     auto examples = v_init<example*>();
//     examples.push_back(&VW::get_unused_example(vw));

//     int size = vw->example_parser->io_lines.size();
//     v_array<VW::string_view> words= v_init<VW::string_view>();
//     v_array<VW::string_view> parse_name = v_init<VW::string_view>();

//     while(vw->example_parser->io_lines.size() > 0){

//         read_features_string(vw, examples, words, parse_name);

//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), --size);

//     }

//    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 0);

//    io_queue_th.join();

//    VW::finish(*vw);


// }


// //Testing io and parser with 2 threads, sleeping on the io and parser threads for a random amount of time
// BOOST_AUTO_TEST_CASE(sleep_io_and_parser)
// {

//     vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
//     mock_io_lines(vw);

//     // The text used in mock_io_lines
//     std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
//     1 | price:.18 sqft:.15 age:.35 1976
//     0 | price:.53 sqft:.32 age:.87 1924
//     0 | price:.23 sqft:.25 age:.05 2006)";

//     std::thread io_queue_th([&vw]() 
//     {
//         sleep_random_amt_of_time();
//         io_lines_toqueue(*vw);

//     });

//     //io_lines size in parse thread is 0
//     BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size() , 0);

//     //examples is unused in read_features_string, set empty
//     auto examples = v_init<example*>();
//     examples.push_back(&VW::get_unused_example(vw));

//     int size = vw->example_parser->io_lines.size();
//     v_array<VW::string_view> words= v_init<VW::string_view>();
//     v_array<VW::string_view> parse_name = v_init<VW::string_view>();

//     while(vw->example_parser->io_lines.size() > 0){

//         sleep_random_amt_of_time();

//         read_features_string(vw, examples, words, parse_name);

//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), --size);

//     }

//    BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 0);

//    io_queue_th.join();

//    VW::finish(*vw);


// }

// BOOST_AUTO_TEST_CASE(sleep_io_and_parser_twothread)
// {

//     vw* vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
//     mock_io_lines(vw);

//     // The text used in mock_io_lines
//     std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
//     1 | price:.18 sqft:.15 age:.35 1976
//     0 | price:.53 sqft:.32 age:.87 1924
//     0 | price:.23 sqft:.25 age:.05 2006)";

//     std::thread io_queue_th([&vw]() 
//     {
//         sleep_random_amt_of_time();
//         io_lines_toqueue(*vw);

//     });

//     std::thread parse_th([&vw]() 
//     {
//         //io_lines size in parse thread is 0
//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size() , 0);

//         //examples is unused in read_features_string, set empty
//         auto examples = v_init<example*>();
//         examples.push_back(&VW::get_unused_example(vw));

//         int size = vw->example_parser->io_lines.size();
//         v_array<VW::string_view> words= v_init<VW::string_view>();
//         v_array<VW::string_view> parse_name = v_init<VW::string_view>();

//         while(vw->example_parser->io_lines.size() > 0){

//             sleep_random_amt_of_time();

//             read_features_string(vw, examples, words, parse_name);

//             BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), --size);

//         }

//         BOOST_CHECK_EQUAL(vw->example_parser->io_lines.size(), 0);
//     });

//    io_queue_th.join();
//    parse_th.join();

//    VW::finish(*vw);

// }