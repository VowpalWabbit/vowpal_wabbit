#include "vld_clr.h"

namespace VW
{
  int VldReportHook(int reportType, wchar_t *message, int *returnValue)
  {
    VisualLeakDetector::Instance->ReportInternal(reportType, message);

    *returnValue = 0; /* don't debug break */
    return 1; /* handled */
  }

  VisualLeakDetector::VisualLeakDetector() : m_messages(gcnew List<Tuple<int, String^>^>)
  {
    if (Instance != nullptr)
    {
      throw gcnew NotSupportedException("Only a single instance is supported.");
    }

    Instance = this;

    VLDSetReportHook(VLD_RPTHOOK_INSTALL, VldReportHook);
  }

  VisualLeakDetector::~VisualLeakDetector()
  {
    this->!VisualLeakDetector();
  }

  VisualLeakDetector::!VisualLeakDetector()
  {
    VLDSetReportHook(VLD_RPTHOOK_REMOVE, VldReportHook);
    Instance = nullptr;
  }

  void VisualLeakDetector::ReportInternal(int reportType, wchar_t *message)
  {
    m_messages->Add(Tuple::Create(reportType, gcnew String(message)));
  }

  void VisualLeakDetector::ReportLeaks()
  {
    VLDReportLeaks();
  }

  List<Tuple<int, String^>^>^ VisualLeakDetector::Messages::get()
  {
    return m_messages;
  }
}
