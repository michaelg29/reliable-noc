
#include "systemc.h"

#include "system.h"
#include "checksum.h"
#include "application.h"
#include "noc_adapter_if.h"

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

// determine number of checkpoints to store
#define CHECKPOINT_SIZE (2*sizeof(noc_data_t))
#define N_CHECKPOINTS (MAX_OUT_SIZE / CHECKPOINT_SIZE)

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
        uint8_t _exp_buf[MAX_OUT_SIZE];
        uint8_t _rsp_buf[MAX_OUT_SIZE];
        uint32_t _write_buf_size;
        uint32_t _exp_buf_size;

        /** Input FIFO from the adapter. */
        uint32_t _in_fifo_src_addr[RESPONSE_FIFO_BUF_SIZE];
        uint32_t _in_fifo_rel_addr[RESPONSE_FIFO_BUF_SIZE];
        noc_data_t _in_fifo_data[RESPONSE_FIFO_BUF_SIZE];
        uint32_t _in_fifo_head;
        uint32_t _in_fifo_tail;

        /** Transmit a packet to the redundant copies. */
        void transmit_to_responders(noc_data_t *packets, uint32_t n_bytes);

        /** Main thread functions. */
        void main();
        void recv_listener();
        void recv_processor();

};

/** Command responder. */
class noc_responder : public noc_tile {

    public:

        sc_port<application> proc_if;

        /** Constructor. */
        SC_HAS_PROCESS(noc_responder);
        noc_responder(sc_module_name name, uint32_t base_addr);

    private:

        uint32_t _base_addr;

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
