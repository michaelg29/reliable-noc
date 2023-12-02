
#include "checksum.h"

#include <stdlib.h>

#ifndef CHECKSUM_HPP
#define CHECKSUM_HPP

/** Encapsulation for the state of a redundant module. */
template <typename packet_t>
class redundancy_state {

    public:

        /**
         * Constructor.
         *
         * @param checkpoint_size Number of packets to complete a checkpoint.
         * @param n_checkpoints   Number of checkpoints to buffer internally.
         */
        redundancy_state(uint32_t checkpoint_size, uint32_t n_checkpoints)
            : _checkpoint_size(checkpoint_size), _n_checkpoints(n_checkpoints), _count_cur_checkpoint(0), _checkpoint_idx(0)
        {
            _crc = new uint32_t[n_checkpoints+1];
            _crc[0] = CRC_START_32;
            _checkpoint_buf = new packet_t[checkpoint_size];
        }

        /** Update the internal CRC with a new packet. Return the new checkpoint if it achieved one, otherwise return 0. */
        uint32_t update(packet_t packet, uint32_t& new_crc) {
            if (_checkpoint_idx >= _n_checkpoints) return false;

            // buffer incoming packet
            _checkpoint_buf[_count_cur_checkpoint] = packet;
            _count_cur_checkpoint++;

            // update CRC
            for (int byte_i = 0; byte_i < sizeof(packet_t); ++byte_i) {
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

        /** Get the CRC for a specific checkpoint. Return true if the redundant state has reached that checkpoint. */
        bool get_crc(uint32_t checkpoint, uint32_t& crc) {
            // ensure has reached the checkpoint
            if (checkpoint > _checkpoint_idx) {
                return false;
            }

            crc = _crc[checkpoint];
            return true;
        }

        /** Read the checkpoint buffer. */
        void copy_checkpoint_buf(packet_t *out_buf) {
            for (int i = 0; i < _checkpoint_size; ++i) {
                out_buf[i] = _checkpoint_buf[i];
            }
        }

        /** Reset the state. */
        void reset() {
            _count_cur_checkpoint = 0;
            _checkpoint_idx = 0;

            memset(_crc, 0, (_n_checkpoints+1)*sizeof(uint32_t));
            memset(_checkpoint_buf, 0, _checkpoint_size*sizeof(packet_t));
            _crc[0] = CRC_START_32;
        }

    private:

        /** Configuration. */
        uint32_t _checkpoint_size;
        uint32_t _n_checkpoints;

        /** CRC and cursors. */
        uint32_t *_crc;
        uint32_t _count_cur_checkpoint;
        uint32_t _checkpoint_idx;

        /** Checkpoint buffer. */
        packet_t *_checkpoint_buf;

};

/** Status of the incoming packet. */
enum tmr_packet_status_e {
    TMR_STATUS_NONE,
    TMR_STATUS_COMMIT,
    TMR_STATUS_INVALID
};

/** Encapsulation for a collection of three redundant modules. */
template <typename packet_t>
class tmr_state_collection {

    public:

        /**
         * Constructor.
         *
         * @param checkpoint_size Number of packets to complete a checkpoint.
         * @param n_checkpoints   Number of checkpoints to buffer internally.
         */
        tmr_state_collection(uint32_t checkpoint_size, uint32_t n_checkpoints) : _confirmed_checkpoint(0) {
            for (int i = 0; i < 3; ++i) {
                _cursors[i] = 0;
                _states[i] = new redundancy_state<packet_t>(checkpoint_size, n_checkpoints);
            }
        }

        /** Receive a new packet. Return state of the packet in the redundancy system. */
        tmr_packet_status_e update(int32_t redundant_idx, packet_t packet, packet_t *out_packets) {
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

        /** Reset the TMR status. */
        void reset() {
            // reset module states
            for (int i = 0; i < 3; ++i) {
                _states[i]->reset();
                _cursors[i] = 0;
            }

            _confirmed_checkpoint = 0;

        }

    private:

        /** Internal cursors. */
        uint32_t _confirmed_checkpoint;
        uint32_t _cursors[3];

        /** Collection. */
        redundancy_state<packet_t> *_states[3];

};

/** Sample usage of the class. */
void tmr_state_collection_demo() {
    uint32_t n_checkpoints = 4;
    uint32_t checkpoint_size = 2; // in packets
    uint32_t n_packets = n_checkpoints * checkpoint_size;
    tmr_state_collection<uint64_t> states(checkpoint_size, n_checkpoints);

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

#endif // CHECKSUM_HPP
