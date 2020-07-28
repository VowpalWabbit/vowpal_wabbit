#ifndef STATIC_LINK_VW
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include <queue>
#include <chrono>

#include "parse_example.cc"
//#include "parse_dispatch_loop.h"
#include "io_to_queue.h"

#include <time.h> 

//initial, const copy of input for comparison
const std::vector<std::string> global_input = {"0 | price:.23 sqft:.25 age:.05 2006", "1 | price:.18 sqft:.15 age:.35 1976", "0 | price:.53 sqft:.32 age:.87 1924", "0 | price:.23 sqft:.25 age:.05 2006"};

void sleep_random_amt_of_time(){

    srand (time(NULL));

    int duration = rand() % 10;

    std::this_thread::sleep_for (std::chrono::milliseconds(duration));
}

//Mock the io state, by creating an io_state with 4 items in its queue. This io_state is a mock of the io_state information that p->_io_state should hold.
void set_mock_iostate(vw *vw, std::queue<io_item> *mock_input_lines){

    std::vector<std::string> input; 

    input.push_back("0 | price:.23 sqft:.25 age:.05 2006"); 
    input.push_back("1 | price:.18 sqft:.15 age:.35 1976");  
    input.push_back("0 | price:.53 sqft:.32 age:.87 1924"); 
    input.push_back("0 | price:.23 sqft:.25 age:.05 2006"); 

    for(int i=0; i<input.size(); i++){
        BOOST_CHECK_EQUAL(input.at(i), global_input.at(i));
    }

    while(!input.empty()){
        //I don't think pushing back is as is should be. want first input to have all lines, second to have all but first, etc.
        std::vector<char> vec(input.back().begin(), input.back().end());
        vec.push_back('\0');
        //io_item my_item(vec, input.back().size() + 1);
        io_item my_item(vec);
        mock_input_lines->push(my_item);
        input.pop_back();
    }

    //io_state mock_iostate;
    //mock_iostate.io_lines = mock_input_lines;

    vw->p->_io_state.io_lines = mock_input_lines;

    BOOST_CHECK_EQUAL(mock_input_lines->size(), 4);
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 4);

}

//Mock the input lines, i.e. put filler text as the input of the io_buf in the parser
//FIX THIS! DON'T WE JUST NEED TO SET INPUT?
//void mock_io_lines(vw *vw, std::queue<io_item> *io_lines){
//remove mock io lines functions...?
void mock_io_lines(vw *vw){
    
    //io_state curr_io_state;

    //(io_lines);
    //curr_io_state.io_lines = io_lines;

    //vw->p->_io_state = curr_io_state;

    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    vw->p->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

}

void mock_io_lines_empty(vw *vw){

    std::string text = "";

    vw->p->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));
}

/* stalls
NOTE: io_lines_toqueue looks for \n, num_chars_init, but right now
it's reading char by char, always one char because of that, so n_c_i never < 1,
test goes on forever, stalls. How to fix to read one line at a time??*/
BOOST_AUTO_TEST_CASE(mock_out_io_to_queue)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::queue<io_item> *io_lines = nullptr;
    
   //mock_io_lines(vw);
   std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    // add input as buffer_view so parses line by line
    vw->p->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    io_lines_toqueue(*vw);

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 4);

    VW::finish(*vw);


}

BOOST_AUTO_TEST_CASE(empty_io_test)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::string text = "";

    vw->p->input->add_file(VW::io::create_buffer_view(text.data(), text.size()));

    io_lines_toqueue(*vw);

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0);

    VW::finish(*vw);

}

//passes
BOOST_AUTO_TEST_CASE(mock_out_pop_io)
{
    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);

    std::queue<io_item> *mock_input_lines = new std::queue<io_item>;

    set_mock_iostate(vw, mock_input_lines);

    BOOST_CHECK_EQUAL(mock_input_lines->size(), 4);

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines, mock_input_lines);

    int curr_num_lines = vw->p->_io_state.io_lines->size();

    while(vw->p->_io_state.io_lines->size() > 0){

        curr_num_lines--;

        //check actual line
        /* The reason that we check that the message popped is equal to gloabl_input at 
        curr_num_lines is because in set_mock_iostate, the methods push_back and pop_back 
        are called on the input lines, so the input lines are pushed in reverse to the io queue in vw's 
        io_state's io_lines. This is a trivial component of the checking, and still shows us
        that the io queue is being popped as desired.*/
       
        vw->p->_io_state.pop_io_queue();
        //BOOST_CHECK_EQUAL(vw->p->_io_state.pop_io_queue().message.data(), global_input.at(curr_num_lines));
        //BOOST_CHECK_EQUAL(pop_io_queue(vw).message, global_input.at(curr_num_lines));
        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), curr_num_lines);

    }

    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0);

    VW::finish(*vw);

}

//Testing io and parser with 2 threads, sleeping on the parser thread for a random amount of time
BOOST_AUTO_TEST_CASE(sleep_parser)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
    mock_io_lines(vw);

    // The text used in mock_io_lines
    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    std::thread io_queue_th([&vw]() 
    {
        //sleep_random_amt_of_time();
        //io lines to queue goes forever...
        io_lines_toqueue(*vw);

        //io_lines size in io thread is 4
        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 4);

        //have condition var... signal at end of while loop with r_feats_string, then check io lines size 0

    });

    //io_lines size in parse thread is 0
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 0);

    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    int size = vw->p->_io_state.io_lines->size();
    v_array<VW::string_view> words= v_init<VW::string_view>();
    v_array<VW::string_view> parse_name = v_init<VW::string_view>();

    while(vw->p->_io_state.io_lines->size() > 0){

        sleep_random_amt_of_time();

        read_features_string(vw, examples, words, parse_name);

        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), --size);

        //check what read etc.

    }

   BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0);

   io_queue_th.join();

   VW::finish(*vw);

}

//Testing io and parser with 2 threads, sleeping on the io thread for a random amount of time
BOOST_AUTO_TEST_CASE(sleep_io)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
    mock_io_lines(vw);

    // The text used in mock_io_lines
    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    std::thread io_queue_th([&vw]() 
    {
        sleep_random_amt_of_time();
        //io lines to queue goes forever...
        io_lines_toqueue(*vw);

        //io_lines size in io thread is 4
        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 4);

        //have condition var... signal at end of while loop with r_feats_string, then check io lines size 0

    });

    //io_lines size in parse thread is 0
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 0);

    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    int size = vw->p->_io_state.io_lines->size();
    v_array<VW::string_view> words= v_init<VW::string_view>();
    v_array<VW::string_view> parse_name = v_init<VW::string_view>();

    while(vw->p->_io_state.io_lines->size() > 0){

        //sleep_random_amt_of_time();

        read_features_string(vw, examples, words, parse_name);

        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), --size);

        //check what read etc.

    }

   BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0);

   io_queue_th.join();

   VW::finish(*vw);


}


//Testing io and parser with 2 threads, sleeping on the io and parser threads for a random amount of time
BOOST_AUTO_TEST_CASE(sleep_io_and_parser)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
    mock_io_lines(vw);

    // The text used in mock_io_lines
    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    std::thread io_queue_th([&vw]() 
    {
        sleep_random_amt_of_time();
        //io lines to queue goes forever...
        io_lines_toqueue(*vw);

        //io_lines size in io thread is 4
        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 4);

        //have condition var... signal at end of while loop with r_feats_string, then check io lines size 0

    });

    //io_lines size in parse thread is 0
    BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 0);

    //examples is unused in read_features_string, set empty
    auto examples = v_init<example*>();
    examples.push_back(&VW::get_unused_example(vw));

    int size = vw->p->_io_state.io_lines->size();
    v_array<VW::string_view> words= v_init<VW::string_view>();
    v_array<VW::string_view> parse_name = v_init<VW::string_view>();

    while(vw->p->_io_state.io_lines->size() > 0){

        sleep_random_amt_of_time();

        read_features_string(vw, examples, words, parse_name);

        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), --size);

        //check what read etc.

    }

   BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0);

   io_queue_th.join();

   VW::finish(*vw);


}

BOOST_AUTO_TEST_CASE(sleep_io_and_parser_twothread)
{

    auto vw = VW::initialize("--no_stdin --quiet", nullptr , false, nullptr, nullptr);
    
    mock_io_lines(vw);

    // The text used in mock_io_lines
    std::string text = R"(0 | price:.23 sqft:.25 age:.05 2006  
    1 | price:.18 sqft:.15 age:.35 1976
    0 | price:.53 sqft:.32 age:.87 1924
    0 | price:.23 sqft:.25 age:.05 2006)";

    std::thread io_queue_th([&vw]() 
    {
        sleep_random_amt_of_time();
        //io lines to queue goes forever...
        io_lines_toqueue(*vw);

        //io_lines size in io thread is 4
        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 4);

        //have condition var... signal at end of while loop with r_feats_string, then check io lines size 0

    });

    std::thread parse_th([&vw]() 
    {
        //io_lines size in parse thread is 0
        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size() , 0);

        //examples is unused in read_features_string, set empty
        auto examples = v_init<example*>();
        examples.push_back(&VW::get_unused_example(vw));

        int size = vw->p->_io_state.io_lines->size();
        v_array<VW::string_view> words= v_init<VW::string_view>();
        v_array<VW::string_view> parse_name = v_init<VW::string_view>();

        while(vw->p->_io_state.io_lines->size() > 0){

            sleep_random_amt_of_time();

            read_features_string(vw, examples, words, parse_name);

            BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), --size);

            //check what read etc.

        }

        BOOST_CHECK_EQUAL(vw->p->_io_state.io_lines->size(), 0);
    });

   io_queue_th.join();
   parse_th.join();

   VW::finish(*vw);

}