#include "error_context.h";
#include <iostream>;

void error_context::on_error(const reinforcement_learning::api_status& status) {
  message = "ErrorCallbackMessage: " + std::string(status.get_error_msg());
  std::cout << message << std::endl;
}

const char* error_context::get_message() const {
  return message.c_str();
}

void on_error(const reinforcement_learning::api_status& status, error_context* context) {
  if (context != nullptr) {
    context->on_error(status);
  }
}