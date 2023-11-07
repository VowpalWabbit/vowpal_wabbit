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
ERROR_CODE_DEFINITION(6, fb_parser_namespace_missing, "Missing Namespace. ")
ERROR_CODE_DEFINITION(7, fb_parser_feature_values_missing, "Missing Feature Values. ")
ERROR_CODE_DEFINITION(8, fb_parser_feature_hashes_names_missing, "Missing Feature Names and Feature Hashes. ")
ERROR_CODE_DEFINITION(9, nothing_to_parse, "No new object to be read from file. ")
ERROR_CODE_DEFINITION(10, fb_parser_unknown_example_type, "Unkown Example type. ")
ERROR_CODE_DEFINITION(11, fb_parser_name_hash_missing, "Missing name and hash field in namespace. ")
ERROR_CODE_DEFINITION(
    12, fb_parser_size_mismatch_ft_hashes_ft_values, "Size of feature hashes and feature values do not match. ")
ERROR_CODE_DEFINITION(
    13, fb_parser_size_mismatch_ft_names_ft_values, "Size of feature names and feature values do not match. ")
ERROR_CODE_DEFINITION(14, unknown_label_type, "Label type in Flatbuffer not understood. ")

// TODO: This is temporary until we switch to the new error handling mechanism.
ERROR_CODE_DEFINITION(10000, vw_exception, "vw_exception: ")

#ifdef ERROR_CODE_DEFINITION_NOOP
#undef ERROR_CODE_DEFINITION
#undef ERROR_CODE_DEFINITION_NOOP
#endif