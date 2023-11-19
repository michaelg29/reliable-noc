
#include "systemc.h"

#include <string>
#include <stdarg.h>

#ifndef SYSTEM_H
#define SYSTEM_H

// =============================
// ===== HIGH LEVEL MACROS =====
// =============================

// logging functions
#define LOG(a) std::cout << sc_time_stamp() << " - " << a << std::endl;
#define LOGF(a, ...) std::cout << sc_time_stamp() << " - "; printf(a, __VA_ARGS__); printf("\n")

// ===========================
// ===== DELAY FUNCTIONS =====
// ===========================

// clocking parameters
#define FREQ 0.15 // 150MHz = 0.15 op/ns
#define CC_NS (1 / FREQ)

// wait for next CC
#define POSEDGE_NoC() wait(1, SC_NS)
#define YIELD() wait(0, SC_NS)
// wait for next CC, then yield priority with another wait statement
#define POSEDGE() wait(1, SC_NS); wait(0, SC_NS)

// ================================
// ===== CONFIGURATION MACROS =====
// ================================

// NoC parameters
#define NOC_X_SIZE 2
#define NOC_Y_SIZE 1
#define NOC_N_TILES (NOC_X_SIZE * NOC_Y_SIZE)

// Base addresses
#define BASE_ADDR_NOC_COMMANDER 0x00000000
#define BASE_ADDR_NOC_RESPONDER 0x01000000

bool parseCmdLine(int argc, char **argv);

#endif // SYSTEM_H
