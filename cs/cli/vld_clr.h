#include <vld.h>

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace VLD
{
int VldReportHook(int reportType, wchar_t *message, int *returnValue);

public ref class VisualLeakDetector
{
private:
  initonly List<Tuple<int, String^>^>^ m_messages;

  !VisualLeakDetector();

public:
  VisualLeakDetector();

  ~VisualLeakDetector();

  static VisualLeakDetector^ Instance;

  void ReportInternal(int reportType, String^ msg);

  property List<Tuple<int, String^>^>^ Messages
  { List<Tuple<int, String^>^>^ get();
  }

  void ReportLeaks();

  void MarkAllLeaksAsReported();
};
}
