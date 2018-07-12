#pragma once
#include "api_status.h"

class error_context {
public:
  void on_error(const reinforcement_learning::api_status& status);
  const char* get_message() const;
private:
  std::string message;
};

void on_error(const reinforcement_learning::api_status& status, error_context* context);
