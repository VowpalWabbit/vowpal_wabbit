/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
const int quadratic_constant = 27942141;
const int cubic_constant = 21791;
const int cubic_constant2 = 37663;
const int affix_constant = 13903957;
const uint64_t constant = 11650396;

// FNV-like hash constant for 32bit
// http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
const uint32_t FNV_prime = 16777619;



// moved from example.h
const unsigned char wap_ldf_namespace = 126;
const unsigned char history_namespace = 127;
const unsigned char constant_namespace = 128;
const unsigned char nn_output_namespace = 129;
const unsigned char autolink_namespace = 130;
const unsigned char neighbor_namespace = 131;   // this is \x83 -- to do quadratic, say "-q a`printf "\x83"` on the command line
const unsigned char affix_namespace = 132;   // this is \x84
const unsigned char spelling_namespace = 133;   // this is \x85
const unsigned char conditioning_namespace = 134;// this is \x86
const unsigned char dictionary_namespace = 135; // this is \x87
const unsigned char node_id_namespace = 136; // this is \x88
const unsigned char message_namespace = 137; // this is \x89
