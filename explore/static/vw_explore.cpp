// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

u64	IdGenerator::g_id = 0;
CRITICAL_SECTION IdGenerator::g_id_mutex;