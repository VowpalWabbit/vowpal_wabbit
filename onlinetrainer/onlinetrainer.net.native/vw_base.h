#pragma once
#include "vw.h"
#include "vw_settings.h"
#include "vw_arguments.h"
#include "vw_io.h"

namespace online_trainer {

  class vw_base {
  private:
    /// <summary>
    /// The settings used for this instance.
    /// </summary>
    vw_settings* m_settings;

    /// <summary>
    /// An optional shared model.
    /// </summary>
    vw_model* m_model;

    /// <summary>
    /// Extracted command line arguments.
    /// </summary>
    vw_arguments* m_arguments;

    /// <summary>
    /// Reference count to native data structure.
    /// </summary>
    int32_t m_instanceCount;

    /// <summary>
    /// The native vowpal wabbit data structure.
    /// </summary>
    vw* m_vw;
  public:
    vw_base(vw_settings* vw_settings);
    virtual ~vw_base();
    /// <summary>
    /// The read/writable model id.
    /// </summary>
    const char* Id;

    /// <summary>
    /// Extracted command line arguments.
    /// </summary>
    vw_arguments* Arguments;
  };
}
