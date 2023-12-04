
/*
 * Validation Tests:
 * https://csrc.nist.gov/csrc/media/projects/cryptographic-algorithm-validation-program/documents/aes/aesavs.pdf
 */

#include "systemc.h"

#include "system.h"

#ifndef APPLICATION_H
#define APPLICATION_H

// ==============================
// ===== AES-256-CBC MACROS =====
// ==============================

// use passed in iv for the first block
// use previous encrypted block as the iv

#define AES_IRREDUCIBLE 0x1B

#define AES_BLOCK_LEN 16
#define AES_BLOCK_SIDE 4

// AES key length in bytes
#define AES_256_KEY_LEN 32

// number of rounds based on the key length
#define AES_256_NR 14

// ============================
// ===== REFERENCE TABLES =====
// ============================

// substitution box
extern uint8_t aes_s_box[256];
extern uint8_t aes_inv_s_box[256];

// constant matrix for mix columns
extern uint8_t aes_mixColMat[AES_BLOCK_SIDE][AES_BLOCK_SIDE];
extern uint8_t aes_inv_mixColMat[AES_BLOCK_SIDE][AES_BLOCK_SIDE];

// ===========================
// ===== UTILITY METHODS =====
// ===========================

// perform Galois Field multiplication of two bytes in GF(2^8)
uint8_t galoisMul(uint8_t g1, uint8_t g2);

// =============================
// ===== APPLICATION CLASS =====
// =============================

#define APPL_FIFO_BUF_N_BITS 6
#define APPL_FIFO_BUF_SIZE (1 << APPL_FIFO_BUF_N_BITS)
#define APPL_FIFO_PTR_MASK (APPL_FIFO_BUF_SIZE - 1)

union aes_block_t {
    uint8_t string[AES_BLOCK_LEN];
    uint8_t square[AES_BLOCK_SIDE][AES_BLOCK_SIDE];
};

struct appl_args_t {
    char *in_file;
    char *iv_file;
    char *key_file;
    char *exp_out_file;
};

class application : virtual public sc_interface, public sc_module {

    public:

        /** Constructor. */
        SC_HAS_PROCESS(application);
        application(sc_module_name name);

        /** Configure. */
        void configure(uint32_t command, uint32_t payload_size, uint32_t out_addr);

        /** Receive packet. */
        void write_packet(uint32_t rel_addr, noc_data_t buffer);
        
        /** Read output packet. */
        bool read_packet(uint32_t& out_addr, noc_data_t& out_buffer);

        /** Determine if whole payload processed. */
        bool is_done();

    private:

        /** Module state. */
        bool _is_busy;
        uint32_t _loaded_size;
        uint32_t _expected_size;
        uint32_t _out_addr;

        /** Internal memory. */
        noc_data_t *_cur_ptr;
        uint8_t _buf[AES_256_KEY_LEN];
        aes_block_t _iv;
        uint8_t _subkeys[AES_256_NR + 1][AES_BLOCK_SIDE][AES_BLOCK_SIDE];

        /** Input FIFO. */
        uint32_t _in_fifo_n[APPL_FIFO_BUF_SIZE];
        aes_block_t _in_fifo[APPL_FIFO_BUF_SIZE];
        uint32_t _in_fifo_head;
        uint32_t _in_fifo_tail;

        /** Output FIFO. */
        noc_data_t _out_fifo[APPL_FIFO_BUF_SIZE];
        uint32_t _out_fifo_head;
        uint32_t _out_fifo_tail;

        /** Main thread functions. */
        void main();
        
        /** AES functions and buffers. */
        aes_block_t _state;
        aes_block_t _tmp;
        void aes_encrypt_block(aes_block_t* in_text, int n,
                       uint8_t subkeys[AES_256_NR + 1][AES_BLOCK_SIDE][AES_BLOCK_SIDE],
                       aes_block_t* iv,
                       aes_block_t* out);

};

// =================================
// ===== AES ENCRYPTION LAYERS =====
// =================================

void aes_addRoundKey(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE], uint8_t subkey[AES_BLOCK_SIDE][AES_BLOCK_SIDE]);
void aes_byteSub(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE]);
void aes_shiftRows(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE]);
void aes_mixCols(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE], uint8_t tmp[AES_BLOCK_SIDE][AES_BLOCK_SIDE]);

// ==========================
// ===== KEY SCHEDULING =====
// ==========================

void aes_generateKeySchedule256(uint8_t in_key[AES_256_KEY_LEN], uint8_t subkeys[AES_256_NR + 1][AES_BLOCK_SIDE][AES_BLOCK_SIDE]);

#endif // APPLICATION_H
