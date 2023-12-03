
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "systemc.h"

#include "system.h"
#include "sc_fault_inject.hpp"
#include "application.h"

// ============================
// ===== REFERENCE TABLES =====
// ============================

// substitution box
uint8_t aes_s_box[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

uint8_t aes_inv_s_box[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

// constant matrix for mix columns
uint8_t aes_mixColMat[AES_BLOCK_SIDE][AES_BLOCK_SIDE] = {
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}};

uint8_t aes_inv_mixColMat[AES_BLOCK_SIDE][AES_BLOCK_SIDE] = {
    {0x0E, 0x0B, 0x0D, 0x09},
    {0x09, 0x0E, 0x0B, 0x0D},
    {0x0D, 0x09, 0x0E, 0x0B},
    {0x0B, 0x0D, 0x09, 0x0E}};

// ===========================
// ===== UTILITY METHODS =====
// ===========================

// perform Galois Field multiplication of two bytes in GF(2^8)
uint8_t galoisMul(uint8_t g1, uint8_t g2)
{
    // taken and documented from https://en.wikipedia.org/wiki/Rijndael_MixColumns
    uint8_t p = 0;

    for (int i = 0; i < 8; i++)
    {
        if (g2 & 0x01) // if LSB is active (equivalent to a '1' in the polynomial of g2)
        {
            p ^= g1; // p += g1 in GF(2^8)
        }

        bool hiBit = (g1 & 0x80); // g1 >= 128 = 0100 0000
        g1 <<= 1;                 // rotate g1 left (multiply by x in GF(2^8))
        if (hiBit)
        {
            // must reduce
            g1 ^= AES_IRREDUCIBLE; // g1 -= 00011011 == mod(x^8 + x^4 + x^3 + x + 1) = AES irreducible
        }
        g2 >>= 1; // rotate g2 right (divide by x in GF(2^8))
    }

    return p;
}

// =================================
// ===== AES ENCRYPTION LAYERS =====
// =================================

void aes_addRoundKey(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE], uint8_t subkey[AES_BLOCK_SIDE][AES_BLOCK_SIDE])
{
    // add in GF(2^8) corresponding bytes of the subkey and state
    for (int r = 0; r < AES_BLOCK_SIDE; r++)
    {
        for (int c = 0; c < AES_BLOCK_SIDE; c++)
        {
            state[r][c] ^= subkey[r][c];
        }
    }
}

void aes_byteSub(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE])
{
    // substitute each byte using the s-box
    for (int r = 0; r < AES_BLOCK_SIDE; r++)
    {
        for (int c = 0; c < AES_BLOCK_SIDE; c++)
        {
            state[r][c] = aes_s_box[state[r][c]];
        }
    }
}

void aes_shiftRows(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE])
{
    // rotate each row according to its position (first row not shifted)
    uint32_t row;
    for (int r = 8; r < (AES_BLOCK_SIDE << 3); r += 8)
    {
        // treat each row as a uint32
        row = *(uint32_t*)state[r >> 3];
        *(uint32_t*)state[r >> 3] = (row << r) | (row >> (32 - r));
    }
}

void aes_mixCols(uint8_t state[AES_BLOCK_SIDE][AES_BLOCK_SIDE], uint8_t tmp[AES_BLOCK_SIDE][AES_BLOCK_SIDE])
{
    // matrix multiplication in GF(2^8)
    // * => galoisMul, + => ^
    for (int r = 0; r < AES_BLOCK_SIDE; r++)
    {
        for (int c = 0; c < AES_BLOCK_SIDE; c++)
        {
            tmp[r][c] = 0x00;
            // dot product of row r of the mixColMat and the col c of the state
            for (int i = 0; i < AES_BLOCK_SIDE; i++)
            {
                tmp[r][c] ^= galoisMul(aes_mixColMat[r][i], state[i][c]);
            }
        }
    }

    // copy memory to the state
    memcpy(state, tmp, AES_BLOCK_SIDE * AES_BLOCK_SIDE * sizeof(uint8_t));
}

void application::aes_encrypt_block(aes_block_t* in_text, int n,
                       uint8_t subkeys[AES_256_NR + 1][AES_BLOCK_SIDE][AES_BLOCK_SIDE],
                       aes_block_t* iv,
                       aes_block_t* out)
{
#ifdef LOG_AES
    printf("\nIV:              ");
    for (int j = 0; j < AES_BLOCK_LEN; ++j) {
        printf("%02x ", iv->string[j]);
    }
    printf("\nInput:           ");
    for (int j = 0; j < n; ++j) {
        printf("%02x ", in_text->string[j]);
    }
#endif

    // represent the state and key as a 4x4 table (read into columns)
    int i = 0;
    for (int c = 0; c < AES_BLOCK_SIDE; c++)
    {
        for (int r = 0; r < AES_BLOCK_SIDE; r++)
        {
            // use PKCS5 padding
            _state.square[r][c] = (i < n) ? in_text->string[i] : AES_BLOCK_LEN - n;

            // XOR with the IV
            _state.square[r][c] ^= iv->string[i];

            i++;
        }
    }

    // ROUND 0
    aes_addRoundKey(_state.square, subkeys[0]);

    // ROUNDS 1 --> NR-1
    for (i = 1; i < AES_256_NR; i++)
    {
        aes_byteSub(_state.square);
        aes_shiftRows(_state.square);
        aes_mixCols(_state.square, _tmp.square);
        aes_addRoundKey(_state.square, subkeys[i]);
    }

    // ROUND NR
    aes_byteSub(_state.square);
    aes_shiftRows(_state.square);
    aes_addRoundKey(_state.square, subkeys[AES_256_NR]);

    // copy bytes of state into the output column by column
    i = 0;
#ifdef LOG_AES
    printf("\nOutput:          ");
#endif
    for (int c = 0; c < AES_BLOCK_SIDE; c++)
    {
        for (int r = 0; r < AES_BLOCK_SIDE; r++)
        {
            out->string[i++] = _state.square[r][c];
#ifdef LOG_AES
            printf("%02x ", _state.square[r][c]);
#endif
        }
    }
#ifdef LOG_AES
    printf("\n");
#endif
}

// ==========================
// ===== KEY SCHEDULING =====
// ==========================

void aes_generateKeySchedule256(uint8_t in_key[AES_256_KEY_LEN], uint8_t subkeys[AES_256_NR + 1][AES_BLOCK_SIDE][AES_BLOCK_SIDE]) {
#ifdef LOG_AES
    printf("Key:              ");
    for (int j = 0; j < AES_256_KEY_LEN; ++j) {
        printf("%02x ", in_key[j]);
    }
    printf("\n");
#endif

    // write original key
    int i;
    for (i = 0; i < 8; i++) {
        // round number is i / 4
        int rd = i >> 2;
        // column is the remainder of a division by 4
        int c = i & 0b11;

        for (int r = 0; r < AES_BLOCK_SIDE; r++) {
            subkeys[rd][r][c] = in_key[i * AES_BLOCK_SIDE + r];
        }
    }

    uint8_t roundCoeff = 0x01;

    int prev_el_rd = 1, prev_el_c = 3;
    for (i = 8; i < 60; i++) {
        int rd = i >> 2;
        int c = i & 0b11;

        int xor_el_rd = rd - 2;
        int xor_el_c = c;

        if (!(i & 0b111)) {
            // function g
            uint8_t g[4] = {
                (uint8_t)(aes_s_box[subkeys[prev_el_rd][1][prev_el_c]] ^ roundCoeff),
                aes_s_box[subkeys[prev_el_rd][2][prev_el_c]],
                aes_s_box[subkeys[prev_el_rd][3][prev_el_c]],
                aes_s_box[subkeys[prev_el_rd][0][prev_el_c]]};

            for (int r = 0; r < AES_BLOCK_SIDE; r++)
            {
                subkeys[rd][r][c] = subkeys[xor_el_rd][r][xor_el_c] ^ g[r];
            }

            roundCoeff = galoisMul(roundCoeff, 0x02);
        }
        else if (!(i & 0b11)) {
            // function h
            for (int r = 0; r < AES_BLOCK_SIDE; r++) {
                subkeys[rd][r][c] = subkeys[xor_el_rd][r][xor_el_c] ^ aes_s_box[subkeys[prev_el_rd][r][prev_el_c]];
            }
        }
        else {
            for (int r = 0; r < AES_BLOCK_SIDE; r++) {
                subkeys[rd][r][c] = subkeys[xor_el_rd][r][xor_el_c] ^ subkeys[prev_el_rd][r][prev_el_c];
            }
        }

        prev_el_rd = rd;
        prev_el_c = c;
    }
}

// =============================
// ===== APPLICATION CLASS =====
// =============================

application::application(sc_module_name name) : sc_module(name) {
    // initial state
    _is_busy = false;
    _out_fifo_head = 0;
    _out_fifo_tail = 0;

    SC_THREAD(main);
}

void application::configure(uint32_t command, uint32_t payload_size, uint32_t out_addr) {
    _is_busy = true;
    _expected_size = payload_size;
    _out_addr = out_addr;
    _cur_ptr = (noc_data_t*)_buf;

    LOGF("Configured to receive payload of size %d and write to %08x", payload_size, out_addr);

    sc_fault_injector::set_injectable_ptr(_iv.string, AES_BLOCK_LEN, 0.001f, (char*)this->name());
}

void application::write_packet(uint32_t rel_addr, noc_data_t buffer) {
    // receive data
    *_cur_ptr = buffer;
    _cur_ptr++;
    _loaded_size += sizeof(noc_data_t);

    if (_loaded_size == AES_256_KEY_LEN) {
        // generate key schedule
        aes_generateKeySchedule256(_buf, _subkeys);

        // advance pointer to IV buffer
        _cur_ptr = (noc_data_t*)_iv.string;
    }
    else if (_loaded_size == AES_256_KEY_LEN + AES_BLOCK_LEN) {
        // advance pointer to input buffer
        _cur_ptr = (noc_data_t*)_in_fifo[_in_fifo_tail & APPL_FIFO_PTR_MASK].string;
    }
    else if (_loaded_size > AES_256_KEY_LEN + AES_BLOCK_LEN) {
        if (!(_loaded_size & 0xf) || _loaded_size > _expected_size) {
            // encrypt a complete 16-byte block
            _in_fifo_n[_in_fifo_tail & APPL_FIFO_PTR_MASK] = _loaded_size > _expected_size ? _expected_size & 0x1f : 16;

            // advance pointer to next input FIFO entry
            _in_fifo_tail++;
            _cur_ptr = (noc_data_t*)_in_fifo[_in_fifo_tail & APPL_FIFO_PTR_MASK].string;
        }

        if (_loaded_size == _expected_size) {
            // encrypt a padding block
            _in_fifo_n[_in_fifo_tail & APPL_FIFO_PTR_MASK] = 0;

            // advance pointer to next input FIFO entry
            _in_fifo_tail++;
            _cur_ptr = (noc_data_t*)_in_fifo[_in_fifo_tail & APPL_FIFO_PTR_MASK].string;
        }
    }
}

bool application::read_packet(uint32_t& out_addr, noc_data_t& out_buffer) {
    if (_out_fifo_tail != _out_fifo_head) {
        // dequeue from FIFO
        out_addr = _out_addr;
        out_buffer = _out_fifo[_out_fifo_head & APPL_FIFO_PTR_MASK];
        _out_fifo_head++;

        _out_addr += sizeof(noc_data_t);
        return true;
    }
    else {
        return false;
    }
}

bool application::is_done() {
    return !_is_busy;
}

void application::main() {
    while (true) {
        // read input data
        if (_in_fifo_tail != _in_fifo_head) {
            // encrypt dequeued packet from FIFO
            aes_encrypt_block((aes_block_t*)(_in_fifo +(_in_fifo_head & APPL_FIFO_PTR_MASK)), _in_fifo_n[_in_fifo_head & APPL_FIFO_PTR_MASK],
                              _subkeys,
                              &_iv,
                              &_tmp);
            _in_fifo_head++;

            // copy to output FIFO and IV for next encryption
            memcpy(_iv.string, _tmp.string, AES_BLOCK_LEN);
            memcpy(_out_fifo + (_out_fifo_tail & APPL_FIFO_PTR_MASK), _tmp.string, AES_BLOCK_LEN);
            _out_fifo_tail += 2;
        }

        POSEDGE();
    }
}
