#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <queue>
#include <chrono>

#include "conditional_contextual_bandit.h"
#include "parse_example.cc"
#include "parse_dispatch_loop.h"
#include "io_to_queue.h"

#include <time.h> 


void sleep_random_amt_of_time(){

    srand (time(NULL));

    int duration = rand() % 10;

    std::this_thread::sleep_for (std::chrono::milliseconds(duration));
}

//Mock the io state, by creating an IO_State with 10 identical items in its queue. This IO_State is a mock of the IO_State information that p->_io_state should hold.
void set_mock_iostate(vw *vw, std::queue<IO_Item> *mock_input_lines){

    for(int i=0; i<10; i++){
        std::string added_string = "0 | price:.23 sqft:.25 age:.05 2006";
        IO_Item my_item(added_string, added_string.size());
        mock_input_lines->push(my_item);
    }

    IO_State mock_iostate = IO_State(mock_input_lines);

    BOOST_CHECK_EQUAL(mock_input_lines->size(), 10);
    BOOST_CHECK_EQUAL(mock_iostate.io_lines->size(), 10);

    vw->p->_io_state = mock_iostate;

}

//Mock the input lines, i.e. put filler text as the input of the io_buf in the parser
void mock_io_lines(vw *vw, std::queue<IO_Item> *io_lines){
    
    IO_State io_state(io_lines);

    vw->p->_io_state = io_state;

    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    vw->p->input->set((char *)text.c_str());
}

BOOST_AUTO_TEST_CASE(mock_out_io_to_queue)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::queue<IO_Item> *io_lines = new std::queue<IO_Item>;
    
    mock_io_lines(vw, io_lines);

    //issue: parses char by char, not line by line
    //I think it's an issue with the size of the buffer.
    io_lines_toqueue(*vw);

    //text.size() = 157 chars, adds each char separately
    //this passes :) BUT FIX PARSING SO PARSES EACH LINE
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, io_lines);
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , io_lines->size());

    VW::finish(*vw);


}

BOOST_AUTO_TEST_CASE(mock_out_pop_io)
{
    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::queue<IO_Item> *mock_input_lines = new std::queue<IO_Item>;

    for(int i=0; i<10; i++){
        std::string added_string = "0 | price:.23 sqft:.25 age:.05 2006";
        IO_Item my_item(added_string, added_string.size());
        mock_input_lines->push(my_item);
    }

    IO_State mock_iostate = IO_State(mock_input_lines);

    BOOST_CHECK_EQUAL(mock_input_lines->size(), 10);
    BOOST_CHECK_EQUAL(mock_iostate.io_lines->size(), 10);

    vw->p->_io_state = mock_iostate;

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, mock_input_lines);

    while(vw->p->_io_state.io_lines->size() > 0){
        BOOST_CHECK_EQUAL(pop_io_queue(vw).message, "0 | price:.23 sqft:.25 age:.05 2006");
    }

   BOOST_CHECK_EQUAL(mock_iostate.io_lines->size(), 0);

   VW::finish(*vw);

}

BOOST_AUTO_TEST_CASE(mock_out_parser)
{
    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    std::queue<IO_Item> *mock_input_lines = new std::queue<IO_Item>;

    set_mock_iostate(vw, mock_input_lines);

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, mock_input_lines);

    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    while(vw->p->_io_state.io_lines->size() > 0){

        read_features_string(vw, examples);
    }

   BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0); 

   VW::finish(*vw);

}

//Testing io and parser with 2 threads, sleeping on the parser thread for a random amount of time
BOOST_AUTO_TEST_CASE(sleep_parser)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    std::queue<IO_Item> *io_lines = new std::queue<IO_Item>;

    mock_io_lines(vw, io_lines);

    std::thread io_queue_th([&vw]() 
    {
        io_lines_toqueue(*vw);

    });

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, io_lines);
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , io_lines->size());

    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    while(vw->p->_io_state.io_lines->size() > 0){

        sleep_random_amt_of_time();

        read_features_string(vw, examples);

    }

   BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0); 

   io_queue_th.join();

   VW::finish(*vw);

}

//Testing io and parser with 2 threads, sleeping on the io thread for a random amount of time
BOOST_AUTO_TEST_CASE(sleep_io)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    std::queue<IO_Item> *io_lines = new std::queue<IO_Item>;

    mock_io_lines(vw, io_lines);

    std::thread io_queue_th([&vw]() 
    {
        sleep_random_amt_of_time();
        io_lines_toqueue(*vw);

    });


    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, io_lines);
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , io_lines->size());

    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    while(vw->p->_io_state.io_lines->size() > 0){

        read_features_string(vw, examples);

    }

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0); 

    io_queue_th.join();

    VW::finish(*vw);


}


//Testing io and parser with 2 threads, sleeping on the io and parser threads for a random amount of time
BOOST_AUTO_TEST_CASE(sleep_io_and_parser)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    std::queue<IO_Item> *io_lines = new std::queue<IO_Item>;

    mock_io_lines(vw, io_lines);

    std::thread io_queue_th([&vw]() 
    {
         sleep_random_amt_of_time();
        io_lines_toqueue(*vw);

    });

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, io_lines);
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , io_lines->size());


    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    while(vw->p->_io_state.io_lines->size() > 0){

        sleep_random_amt_of_time();
        read_features_string(vw, examples);

    }

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0); 

    io_queue_th.join();

    VW::finish(*vw);


}

BOOST_AUTO_TEST_CASE(sleep_io_and_parser_twothread)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    std::queue<IO_Item> *io_lines = new std::queue<IO_Item>;

    mock_io_lines(vw, io_lines);

    std::thread io_queue_th([&vw]() 
    {
         sleep_random_amt_of_time();
        io_lines_toqueue(*vw);

    });

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, io_lines);
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , io_lines->size());

    std::thread parse_th([&vw]() 
    {

        //examples is unused in read_features_string, set empty
        auto examples = v_init<example*>();
        examples.push_back(&VW::get_unused_example(vw));

        while(vw->p->_io_state.io_lines->size() > 0){

            sleep_random_amt_of_time();
            read_features_string(vw, examples);

        }


        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0); 

    });

    io_queue_th.join();
    parse_th.join();

    VW::finish(*vw);

}