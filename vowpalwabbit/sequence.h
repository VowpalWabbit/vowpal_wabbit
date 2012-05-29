#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"


#define clog_print_audit_features(ec,reg) { print_audit_features(reg, ec); }

void parse_sequence_args(vw& all, po::variables_map& vm, void (*base_l)(vw&, example*), void (*base_f)(vw&));
void drive_sequence(void*);

#endif
