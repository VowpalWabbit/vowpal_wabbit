#pragma once

namespace online_trainer {

  class i_online_trainer {
  public:
    virtual ~i_online_trainer() = default;
  };

  class vw_online_trainer : public i_online_trainer {

  };
}
