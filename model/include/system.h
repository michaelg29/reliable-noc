
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

// ========================================
// ===== NoC PARAMETERS AND ADDRESSES =====
// ========================================

#define NOC_X_SIZE 4
#define NOC_Y_SIZE 3
#define NOC_N_TILES (NOC_X_SIZE * NOC_Y_SIZE)
#define GEN_NOC_BASE_ADDR(x, y) (((x & 0xf) << 24) | ((y & 0xf) << 28))
#define NOC_N_RESPONDERS 3

// location and address of commander
#define NOC_X_COMMANDER 1
#define NOC_Y_COMMANDER 1
#define NOC_BASE_ADDR_COMMANDER GEN_NOC_BASE_ADDR(NOC_X_COMMANDER, NOC_Y_COMMANDER)

// location and address of responder0
#define NOC_X_RESPONDER0 2
#define NOC_Y_RESPONDER0 2
#define NOC_BASE_ADDR_RESPONDER0 GEN_NOC_BASE_ADDR(NOC_X_RESPONDER0, NOC_Y_RESPONDER0)

// location and address of responder0
#define NOC_X_RESPONDER1 3
#define NOC_Y_RESPONDER1 0
#define NOC_BASE_ADDR_RESPONDER1 GEN_NOC_BASE_ADDR(NOC_X_RESPONDER1, NOC_Y_RESPONDER1)

// location and address of responder0
#define NOC_X_RESPONDER2 2
#define NOC_Y_RESPONDER2 1
#define NOC_BASE_ADDR_RESPONDER2 GEN_NOC_BASE_ADDR(NOC_X_RESPONDER2, NOC_Y_RESPONDER2)

// ================================
// ===== NoC MACROS AND TYPES =====
// ================================

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
