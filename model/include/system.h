
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

#define MAX_DATA_SIZE 16
#define MAX_OUT_SIZE MAX_DATA_SIZE + 16

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
// ===== NoC MACROS AND TYPES =====
// ================================

// NoC parameters
#define NOC_X_SIZE 2
#define NOC_Y_SIZE 1
#define NOC_N_TILES (NOC_X_SIZE * NOC_Y_SIZE)

// Base addresses
#define BASE_ADDR_NOC_COMMANDER 0x00000000
#define BASE_ADDR_NOC_RESPONDER 0x01000000
#define BASE_ADDR_NOC_MEMORY    0x02000000

/** Data port type. */
typedef uint64_t noc_data_t;
#define NOC_DSIZE sizeof(uint64_t)

/** Address type. */
struct noc_addr_t {
    uint32_t rel;   // relative address within the module
    uint32_t x;     // x-coordinate in the network
    uint32_t y;     // y-coordinate in the network
};

// Address masks (32b = 4b + 4b + 24b)
#define NOC_ADDR_REL_MASK 0x00ffffff
#define NOC_ADDR_X_MASK   0x0f000000
#define NOC_ADDR_Y_MASK   0xf0000000
#define NOC_GET_REL_ADDR(addr) (addr & NOC_ADDR_REL_MASK)
#define NOC_GET_X_ADDR(addr)   ((addr & NOC_ADDR_X_MASK) >> 24)
#define NOC_GET_Y_ADDR(addr)   ((addr & NOC_ADDR_Y_MASK) >> 28)
#define NOC_RECOVER_RAW_ADDR(addr_struct) ((addr_struct.rel & NOC_ADDR_REL_MASK) | ((addr_struct.x << 24) & NOC_ADDR_X_MASK) | ((addr_struct.y << 28) & NOC_ADDR_Y_MASK))

// ============================
// ===== HELPER FUNCTIONS =====
// ============================

bool parse_cmd_line(int argc, char **argv);
uint32_t read_input_files(uint8_t *out);
uint32_t read_expected_output_file(uint8_t *out);

#endif // SYSTEM_H
