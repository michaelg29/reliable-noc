/*
 * Library: libcrc
 * File:    src/crc32.c
 * Author:  Lammert Bies
 *
 * This file is licensed under the MIT License as stated below
 *
 * Copyright (c) 1999-2016 Lammert Bies
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Description
 * -----------
 * The source file src/crc32.c contains the routines which are needed to
 * calculate a 32 bit CRC value of a sequence of bytes.
 */

#include <stdbool.h>
#include <stdlib.h>
#include "checksum.h"

static void             init_crc32_tab( void );

static bool             crc_tab32_init          = false;
static uint32_t		crc_tab32[256];

/*
 * uint32_t crc_32( const unsigned char *input_str, size_t num_bytes );
 *
 * The function crc_32() calculates in one pass the common 32 bit CRC value for
 * a byte string that is passed to the function together with a parameter
 * indicating the length.
 */

uint32_t crc_32( const unsigned char *input_str, size_t num_bytes ) {

	uint32_t crc;
	uint32_t tmp;
	uint32_t long_c;
	const unsigned char *ptr;
	size_t a;

	if ( ! crc_tab32_init ) init_crc32_tab();

	crc = CRC_START_32;
	ptr = input_str;

	if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

		long_c = 0x000000FFL & (uint32_t) *ptr;
		tmp    =  crc       ^ long_c;
		crc    = (crc >> 8) ^ crc_tab32[ tmp & 0xff ];

		ptr++;
	}

	crc ^= 0xffffffffL;

	return crc & 0xffffffffL;

}  /* crc_32 */

/*
 * uint32_t update_crc_32( uint32_t crc, unsigned char c );
 *
 * The function update_crc_32() calculates a new CRC-32 value based on the
 * previous value of the CRC and the next byte of the data to be checked.
 */

uint32_t update_crc_32( uint32_t crc, unsigned char c ) {

	uint32_t tmp;
	uint32_t long_c;

	long_c = 0x000000ffL & (uint32_t) c;

	if ( ! crc_tab32_init ) init_crc32_tab();

	tmp = crc ^ long_c;
	crc = (crc >> 8) ^ crc_tab32[ tmp & 0xff ];

	return crc & 0xffffffffL;;

}  /* update_crc_32 */

/*
 * static void init_crc32_tab( void );
 *
 * For optimal speed, the CRC32 calculation uses a table with pre-calculated
 * bit patterns which are used in the XOR operations in the program. This table
 * is generated once, the first time the CRC update routine is called.
 */

static void init_crc32_tab( void ) {

	uint32_t i;
	uint32_t j;
	uint32_t crc;

	for (i=0; i<256; i++) {

		crc = i;

		for (j=0; j<8; j++) {

			if ( crc & 0x00000001L ) crc = ( crc >> 1 ) ^ CRC_POLY_32;
			else                     crc =   crc >> 1;
		}

		crc_tab32[i] = crc;
	}

	crc_tab32_init = true;

}  /* init_crc32_tab */

redundancy_state::redundancy_state(uint32_t checkpoint_size, uint32_t n_checkpoints)
    : _checkpoint_size(checkpoint_size), _n_checkpoints(n_checkpoints), _count_cur_checkpoint(0), _checkpoint_idx(0)
{
    _crc = new uint32_t[n_checkpoints+1];
    _crc[0] = CRC_START_32;
    _checkpoint_buf = new uint64_t[checkpoint_size];
}

uint32_t redundancy_state::update(uint64_t packet, uint32_t& new_crc) {
    if (_checkpoint_idx >= _n_checkpoints) return false;

    // buffer incoming packet
    _checkpoint_buf[_count_cur_checkpoint] = packet;
    _count_cur_checkpoint++;

    // update CRC
    for (int byte_i = 0; byte_i < sizeof(uint64_t); ++byte_i) {
        _crc[_checkpoint_idx] = update_crc_32(_crc[_checkpoint_idx], (uint8_t)packet);
        packet >>= 8;
    }
    new_crc = _crc[_checkpoint_idx];

    if (_count_cur_checkpoint == _checkpoint_size) {
        // advance checkpoint and copy previous CRC
        _count_cur_checkpoint = 0;
        _checkpoint_idx++;
        _crc[_checkpoint_idx] = _crc[_checkpoint_idx-1];

        return _checkpoint_idx;
    }
    else {
        return false;
    }
}

bool redundancy_state::get_crc(uint32_t checkpoint, uint32_t& crc) {
    // ensure has reached the checkpoint
    if (checkpoint > _checkpoint_idx) {
        return false;
    }

    crc = _crc[checkpoint];
    return true;
}

void redundancy_state::copy_checkpoint_buf(uint64_t *out_buf) {
    for (int i = 0; i < _checkpoint_size; ++i) {
        out_buf[i] = _checkpoint_buf[i];
    }
}

tmr_state_collection::tmr_state_collection(uint32_t checkpoint_size, uint32_t n_checkpoints)
    : _confirmed_checkpoint(0)
{
    for (int i = 0; i < 3; ++i) {
        _cursors[i] = 0;
        _states[i] = new redundancy_state(checkpoint_size, n_checkpoints);
    }
}

tmr_packet_status_e tmr_state_collection::update(int32_t redundant_idx, uint64_t packet, uint64_t *out_packets) {
    if (redundant_idx < 0) return TMR_STATUS_NONE;

    // update CRC and cursor
    uint32_t new_crc;
    uint32_t new_checkpoint = _states[redundant_idx]->update(packet, new_crc);
    _cursors[redundant_idx] += sizeof(uint64_t);

    // compare checkpoints if no majority already found
    if (new_checkpoint && new_checkpoint > _confirmed_checkpoint) {
        bool prev_match = true;
        uint32_t other_crc; // store CRC from the other module
        for (int i = 0; i < 3; ++i) {
            // check other module for their CRC copy
            if (i != redundant_idx && _states[i]->get_crc(new_checkpoint, other_crc)) {
                if (other_crc == new_crc) {
                    // found majority vote
                    _confirmed_checkpoint = new_checkpoint;
                    _states[redundant_idx]->copy_checkpoint_buf(out_packets);
                    return TMR_STATUS_COMMIT;
                }
                else {
                    if (!prev_match) {
                        // all three CRC values mismatch
                        return TMR_STATUS_INVALID;
                    }
                    else {
                        // only one mismatch, check next module
                        prev_match = false;
                    }
                }
            }
        }
    }

    return TMR_STATUS_NONE;
}

/**
 * Testing for the CRC fingerprint method.
 */
#ifdef TEST_CRC
#include <stdio.h>
int crc_test_main() {
    uint32_t n_checkpoints = 4;
    uint32_t checkpoint_size = 2; // in packets
    uint32_t n_packets = n_checkpoints * checkpoint_size;
    tmr_state_collection states(checkpoint_size, n_checkpoints);

    // input streams
    uint64_t stream[3][n_packets] = {
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1},
        {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2}
    };

    uint64_t out_packets[checkpoint_size];
    tmr_packet_status_e status;
    for (int i = 0; i < n_packets; ++i) {
        for (int m = 0; m < 3; ++m) {
            status = states.update(m, stream[m][i], out_packets);
            printf("mod %d: %016lx => status %d\n", m, stream[m][i], status);
            if (status == TMR_STATUS_COMMIT) {
                printf("Confirm store of {");
                for (int p = 0; p < checkpoint_size; p++) {
                    printf("%016lx ", out_packets[p]);
                }
                printf("} for packet %d\n", i);
            }
            else if (status == TMR_STATUS_INVALID) {
                printf("No majority on packet %d\n", i);
            }
        }
        printf("\n");
    }
}
#endif
