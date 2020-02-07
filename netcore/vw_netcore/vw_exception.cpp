// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_clr.h"

namespace VW
{
VowpalWabbitException::VowpalWabbitException(const vw_exception& ex)
  : Exception(gcnew System::String(ex.what())), m_filename(gcnew System::String(ex.Filename())), m_lineNumber(ex.LineNumber())
{
}

String^ VowpalWabbitException::Filename::get()
{ return m_filename;
}

Int32 VowpalWabbitException::LineNumber::get()
{ return m_lineNumber;
}

VowpalWabbitArgumentDisagreementException::VowpalWabbitArgumentDisagreementException(const vw_argument_disagreement_exception& ex)
	: VowpalWabbitException(ex)
{
}
}
