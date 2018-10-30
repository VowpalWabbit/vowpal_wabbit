/**
 * @brief RL Simulator example defintion.  Simulates user/world interacting with RL  
 * 
 * @file rl_sim.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include <boost/program_options.hpp>
#include "person.h"
#include "live_model.h"

/**
 * @brief Reinforcement Learning Simulator
 * This class simulates a user interacting with a reinforcement learning loop where
 * a person is chosen in random then an action is chosen and the outcome generated.
 * The generated outcome is reported back to the reinforcement learning loop.
 */
class rl_sim {
public:
  /**
   * @brief Construct a new rl_sim object
   * 
   * @param vm User defined options
   */
  explicit rl_sim(boost::program_options::variables_map vm);

  /**
   * @brief Simulation loop 
   * 
   * @return int Error status
   */
  int loop();

  /**
   * @brief error handler for background errors
   * _on_error free funciton is registered as the background error handler with the api
   * on_error is called by _on_error()
   */
  void on_error(const reinforcement_learning::api_status& status);

  private:
    /**
     * @brief Create a context json string
     * Context json is constructed from user context json and action json
     * @param cntxt User context json
     * @param action Action json
     * @return std::string Constructed context json
     */
    std::string create_context_json(const std::string& cntxt, const std::string& action);

    /**
     * @brief Create a event_id used to match choose_rank and report_outcome() 
     * 
     * @return std::string 
     */
    std::string create_event_id();

    /**
     * @brief Pick a person from the list of people.  Use uniform random.
     * 
     * @return person& 
     */
    person& pick_a_random_person();

    /**
     * @brief Load Inference API configuration from json string.
     * 
     * @param str json string
     * @param config Configuration object used by Inference API
     * @param status api_status object for error feedback
     * @return int Error status
     */
    int load_config_from_json(
      const std::string& str, 
      reinforcement_learning::utility::configuration& config,
      reinforcement_learning::api_status* status);

    /**
     * @brief Load the contents of a file in to a std::string
     * 
     * @param file_name File name
     * @param config_str String to hold the data
     * @return int Error status
     */
    int load_file(const std::string& file_name, std::string& config_str);

    /**
     * @brief Initialize Inference API
     * 
     * @return int Error status
     */
    int init_rl();

    /**
     * @brief Initialize collection of people
     * 
     * @return true If there is no error during init
     * @return false On init error
     */
    bool init_sim_world();

    /**
     * @brief Initialize the simulator
     * 
     * @return true on success
     * @return false on failure
     */
    bool init();

    /**
     * @brief Get the action features as a json string
     * 
     * @return std::string 
     */
    std::string get_action_features();

  
  private:
    boost::program_options::variables_map _options;
    std::unique_ptr<reinforcement_learning::live_model> _rl;
    std::vector<person> _people;
    std::vector<std::string> _topics;
    bool _run_loop = true;
};

