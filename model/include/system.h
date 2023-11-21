
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

bool parseCmdLine(int argc, char **argv);

// ===========================================
// ===== APPLICATION TYPES AND FUNCTIONS =====
// ===========================================

/** Command structure. */
struct noc_cmd_t {
    uint32_t skey;     // Header start key, must match `NOC_CMD_SKEY`.
    uint32_t cmd;      // Command to execute.
    uint32_t size;     // Size of the payload.
    uint32_t tx_addr;  // Location to write acknowledge packet.
    uint32_t trans_id; // ID of the transaction.
    uint32_t status;   // Response status, only used in acknowledge.
    uint32_t ekey;     // Header end key, must match `NOC_CMD_EKEY`.
    uint32_t chksum;   // Header checksum.
};
#define NOC_CMD_SKEY 0xBEEFCAFE
#define NOC_CMD_EKEY 0x01234567
#define N_PACKETS_IN_CMD (sizeof(noc_cmd_t) / sizeof(noc_data_t))

// calculate the checksum of a command packet
#define CALC_CMD_CHKSUM(cmd) \
    cmd.skey ^ cmd.cmd ^ cmd.size ^ cmd.tx_addr ^ cmd.trans_id ^ cmd.status ^ cmd.ekey

/** State of the commander. */
enum noc_commander_state_e {
    NOC_COMMANDER_IDLE,
    NOC_COMMANDER_WRITE_CMD,
    NOC_COMMANDER_WAIT_ACK,
    NOC_COMMANDER_WRITE_PAYLOAD
};

/** State of the responder. */
enum noc_responder_state_e {
    NOC_RESPONDER_IDLE,
    NOC_RESPONDER_WAIT_SIZE,
    NOC_RESPONDER_WAIT_TRANS_ID,
    NOC_RESPONDER_WAIT_EKEY,
    NOC_RESPONDER_WAIT_DATA
};

#endif // SYSTEM_H
