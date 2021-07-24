#include "benchmark_functions.cc"

BOOST_AUTO_TEST_CASE(test_io_and_parser)
{
  std::cout << "IO_and_PARSER\n";
  for (size_t i = 1; i <= 8; i++)
    std::cout << "Total Time taken: " << run_io_and_parser(i) << " seconds, by parser_threads = " << i << std::endl;
}

BOOST_AUTO_TEST_CASE(test_parser_and_mock_learner)
{
  std::cout << "PARSER_and_MOCK_LEARNER\n";
  for (size_t i = 1; i <= 8; i++)
    std::cout << "Total Time taken: " << run_parser_and_mock_learner(i) << " seconds, by parser_threads = " << i
              << std::endl;
}

BOOST_AUTO_TEST_CASE(test_parser_and_learner)
{
  std::cout << "PARSER_and_LEARNER\n";
  for (size_t i = 1; i <= 8; i++)
    std::cout << "Total Time taken: " << run_parser_and_learner(i) << " seconds, by parser_threads = " << i
              << std::endl;
}

BOOST_AUTO_TEST_CASE(creating_cache)
{
  std::cout << "creating cache file\n";
  run_parser_and_learner(32);
}

BOOST_AUTO_TEST_CASE(test_io_and_parser_cache)
{
  std::cout << "IO_and_PARSER\n";
  for (size_t i = 1; i <= 8; i++)
    std::cout << "Total Time taken: " << run_io_and_parser(i, "-c") << " seconds, by parser_threads = " << i << std::endl;
}

BOOST_AUTO_TEST_CASE(test_parser_and_mock_learner_cache)
{
  std::cout << "PARSER_and_MOCK_LEARNER\n";
  for (size_t i = 1; i <= 8; i++)
    std::cout << "Total Time taken: " << run_parser_and_mock_learner(i, "-c") << " seconds, by parser_threads = " << i
              << std::endl;
}

BOOST_AUTO_TEST_CASE(test_parser_and_learner_cache)
{
  std::cout << "PARSER_and_LEARNER\n";
  for (size_t i = 1; i <= 8; i++)
    std::cout << "Total Time taken: " << run_parser_and_learner(i, "-c") << " seconds, by parser_threads = " << i
              << std::endl;
}