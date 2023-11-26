
#include "systemc.h"

#include "system.h"
#include "application.h"
#include "noc_adapter.h"

#ifndef NOC_TILE_H
#define NOC_TILE_H

// =================================
// ===== TILE TYPES AND STATES =====
// =================================

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

// calculate the checksum of a command packet
#define CALC_CMD_CHKSUM(_cmd) \
    _cmd.skey ^ _cmd.cmd ^ _cmd.size ^ _cmd.tx_addr ^ _cmd.trans_id ^ _cmd.status ^ _cmd.ekey

// statuses
#define NOC_STAT_OKAY      0x00000000
#define NOC_STAT_ERR_KEY   0x00000001
#define NOC_STAT_ERR_CHKSM 0x00000002

/** State of the commander. */
enum noc_commander_state_e {
    NOC_COMMANDER_IDLE,
    NOC_COMMANDER_WRITE_DATA
};

/** State of the responder. */
enum noc_responder_state_e {
    NOC_RESPONDER_IDLE,
    NOC_RESPONDER_WAIT_SIZE,
    NOC_RESPONDER_WAIT_TRANS_ID,
    NOC_RESPONDER_WAIT_EKEY,
    NOC_RESPONDER_WAIT_DATA
};

// ========================
// ===== TILE MODULES =====
// ========================

/** NoC tile module. */
class noc_tile : public sc_module {

    public:

        /** Corresponding adapter. */
        sc_port<noc_adapter_if> adapter_if;

        /** Constructor. */
        noc_tile(sc_module_name name);

};

/** Command issuer. */
class noc_commander : public noc_tile {

    public:

        /** Constructor. */
        SC_HAS_PROCESS(noc_commander);
        noc_commander(sc_module_name name);

    private:
    
        /** Current state. */
        noc_cmd_t _cur_cmd;
        noc_cmd_t _cur_ack;
        noc_commander_state_e _state;
        
        /** Buffers. */
        uint8_t _write_buf[AES_256_KEY_LEN + AES_BLOCK_LEN + MAX_DATA_SIZE];
        uint8_t _exp_buf[MAX_DATA_SIZE];
        uint32_t _dsize;

        /** Main thread functions. */
        void main();
        void recv_listener();

};

/** Command responder. */
class noc_responder : public noc_tile {

    public:
    
        sc_port<application> proc_if;

        /** Constructor. */
        SC_HAS_PROCESS(noc_responder);
        noc_responder(sc_module_name name);

    private:
    
        /** Current state. */
        noc_cmd_t _cur_cmd;
        noc_cmd_t _cur_ack;
        noc_responder_state_e _state;
        bool _last_packet_loaded;
        uint32_t _cur_loaded_size;

        /** Main thread functions. */
        void main();
        void recv_listener();

};

#endif // NOC_TILE_H
