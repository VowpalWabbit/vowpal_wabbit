// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifndef ERROR_CODE_DEFINITION
// If not defined, then define as a noop
#define ERROR_CODE_DEFINITION(code, name, message)
#define ERROR_CODE_DEFINITION_NOOP
#endif

ERROR_CODE_DEFINITION(1, sample_pdf_failed, "Failed to sample from pdf")
ERROR_CODE_DEFINITION(2, num_actions_gt_zero, "Number of leaf nodes must be greater than zero")
ERROR_CODE_DEFINITION(
    3, options_disagree, "Different values specified for two options that are constrained to be the same.")
ERROR_CODE_DEFINITION(4, not_implemented, "Not implemented.")
ERROR_CODE_DEFINITION(5, native_exception, "Native exception: ")

// TODO: This is temporary until we switch to the new error handling mechanism.
ERROR_CODE_DEFINITION(10000, vw_exception, "vw_exception: ")

#ifdef ERROR_CODE_DEFINITION_NOOP
#undef ERROR_CODE_DEFINITION
#undef ERROR_CODE_DEFINITION_NOOP
#endif