// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

ERROR_CODE_DEFINITION(1, sample_pdf_failed, "Failed to sample from pdf")
ERROR_CODE_DEFINITION(2, num_actions_gt_zero, "Number of leaf nodes must be greater than zero")
ERROR_CODE_DEFINITION(
    3, options_disagree, "Different values specified for two options that are constrained to be the same.")
ERROR_CODE_DEFINITION(4, not_implemented, "Not implemented.")
ERROR_CODE_DEFINITION(5, native_exception, "Native exception: ")
ERROR_CODE_DEFINITION(6, fb_parser_namespace_missing, "Missing Namespace")
ERROR_CODE_DEFINITION(7, fb_parser_feature_values_missing, "Missing Feature Values")
ERROR_CODE_DEFINITION(8, fb_parser_feature_hashes_names_missing, "Missing Feature Names and Hashes")
ERROR_CODE_DEFINITION(9, fb_parser_failed_to_parse, "Failed to Parse")
ERROR_CODE_DEFINITION(10, fb_parser_unknown_example_type, "Unkown Example type.")

// TODO: This is temporary until we switch to the new error handling mechanism.
ERROR_CODE_DEFINITION(10000, vw_exception, "vw_exception: ")