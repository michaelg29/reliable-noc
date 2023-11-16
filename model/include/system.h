
#include <string>

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
#define DELAY_CLIENT(cc) wait(cc, SC_NS)
// wait for next CC, then give NoC priority with another wait statement
#define DELAY(cc) wait(cc, SC_NS); wait(0, SC_NS)

// ================================
// ===== CONFIGURATION MACROS =====
// ================================

// NoC parameters
#define N_VC 5

// NoC size: 16x16
// Address masks (32b = 4b + 4b + 24b)
#define NOC_ADDR_REL_MASK 0x00ffffff
#define NOC_ADDR_X_MASK   0x0f000000
#define NOC_ADDR_Y_MASK   0xf0000000
#define NOC_GET_REL_ADDR(addr) (addr & (NOC_ADDR_REL_MASK))
#define NOC_GET_X_ADDR(addr)   (addr & (NOC_ADDR_X_MASK)) >> 24
#define NOC_GET_Y_ADDR(addr)   (addr & (NOC_ADDR_Y_MASK)) >> 28

bool parseCmdLine(int argc, char **argv);

#endif // SYSTEM_H
