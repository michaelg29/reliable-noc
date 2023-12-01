/*
 * MODIFIED FROM
 * Library: libcrc
 * File:    include/checksum.h
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
 * The headerfile include/checksum.h contains the definitions and prototypes
 * for routines that can be used to calculate several kinds of checksums.
 */

#ifndef DEF_LIBCRC_CHECKSUM_H
#define DEF_LIBCRC_CHECKSUM_H

#include <stdint.h>

/** Polynomial used in CRC. */
#define     CRC_POLY_32     0xEDB88320L

/* Initialization of the CRC value. */
#define     CRC_START_32    0xFFFFFFFFL

/* Global functions */
uint32_t crc_32(const unsigned char *input_str, size_t num_bytes);
uint32_t update_crc_32(uint32_t crc, unsigned char c);

/** Encapsulation for the state of a redundant module. */
class redundancy_state {

    public:

        /**
         * Constructor.
         *
         * @param checkpoint_size Number of packets to complete a checkpoint.
         * @param n_checkpoints   Number of checkpoints to buffer internally.
         */
        redundancy_state(uint32_t checkpoint_size, uint32_t n_checkpoints);

        /** Update the internal CRC with a new packet. Return the new checkpoint if it achieved one, otherwise return 0. */
        uint32_t update(uint64_t packet, uint32_t& new_crc);

        /** Get the CRC for a specific checkpoint. Return true if the redundant state has reached that checkpoint. */
        bool get_crc(uint32_t checkpoint, uint32_t& crc);

        /** Read the checkpoint buffer. */
        void copy_checkpoint_buf(uint64_t *out_buf);

    private:

        /** Configuration. */
        uint32_t _checkpoint_size;
        uint32_t _n_checkpoints;

        /** CRC and cursors. */
        uint32_t *_crc;
        uint32_t _count_cur_checkpoint;
        uint32_t _checkpoint_idx;

        /** Checkpoint buffer. */
        uint64_t *_checkpoint_buf;

};

/** Status of the incoming packet. */
enum tmr_packet_status_e {
    TMR_STATUS_NONE,
    TMR_STATUS_COMMIT,
    TMR_STATUS_INVALID
};

/** Encapsulation for a collection of redundant modules. */
class tmr_state_collection {

    public:

        /**
         * Constructor.
         *
         * @param checkpoint_size Number of packets to complete a checkpoint.
         * @param n_checkpoints   Number of checkpoints to buffer internally.
         */
        tmr_state_collection(uint32_t checkpoint_size, uint32_t n_checkpoints);

        /** Receive a new packet. Return state of the packet in the redundancy system. */
        tmr_packet_status_e update(int32_t redundant_idx, uint64_t packet, uint64_t *out_packets);

    private:

        /** Internal cursors. */
        uint32_t _confirmed_checkpoint;
        uint32_t _cursors[3];

        /** Collection. */
        redundancy_state *_states[3];

};

#endif  // DEF_LIBCRC_CHECKSUM_H