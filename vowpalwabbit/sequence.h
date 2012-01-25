#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"


#define clog_print_audit_features(ec) { print_audit_features(global.reg, ec); }

void parse_sequence_args(po::variables_map& vm, void (*base_l)(example*), void (*base_f)());
void drive_sequence();

#endif
