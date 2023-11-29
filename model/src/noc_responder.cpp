
#include "systemc.h"

#include "system.h"
#include "noc_tile.h"

noc_responder::noc_responder(sc_module_name name) : noc_tile(name) {
    SC_THREAD(main);
    SC_THREAD(recv_listener);

    // initial state
    _state = NOC_RESPONDER_IDLE;
    _last_packet_loaded = false;
}

void noc_responder::main() {
    uint32_t out_addr;
    noc_data_t out_buf;

    while (true) {
        if (proc_if->read_packet(out_addr, out_buf)) {
            // write output buffer
            adapter_if->write_packet(0, out_addr, &out_buf, sizeof(noc_data_t));
        }

        if (_last_packet_loaded && proc_if->is_done()) {
            // issue acknowledge
            adapter_if->write_packet(0, _cur_cmd.tx_addr, (noc_data_t*)&_cur_ack, sizeof(noc_cmd_t));

            // clear flag
            _last_packet_loaded = false;
        }

        POSEDGE();
    }
}

void noc_responder::recv_listener() {
    uint32_t src_addr;
    uint32_t in_addr;
    noc_data_t cur_buf;
    noc_data_t *data = (noc_data_t*)&_cur_cmd;

    LOG("Started recv_listener");

    _state = NOC_RESPONDER_IDLE;
    while (true) {
        // receive packet while not busy
        if (adapter_if->read_packet(src_addr, in_addr, *data) && !_last_packet_loaded) {
            LOGF("Received request containing %016lx at %08x", *data, in_addr);

            switch (_state) {
            case NOC_RESPONDER_IDLE:
                _cur_ack.status = 0;

                if (_cur_cmd.skey != NOC_CMD_SKEY) _cur_ack.status |= NOC_STAT_ERR_KEY;

                _cur_ack.skey = NOC_CMD_SKEY;
                _cur_ack.cmd = _cur_cmd.cmd;

                // advance state and pointer
                _state = NOC_RESPONDER_WAIT_SIZE;
                data += 1;
                break;

            case NOC_RESPONDER_WAIT_SIZE:
                // latch in acknowledge message
                _cur_ack.size = _cur_cmd.size;
                _cur_ack.tx_addr = _cur_cmd.tx_addr;

                // advance state and pointer
                _state = NOC_RESPONDER_WAIT_TRANS_ID;
                data += 1;
                break;

            case NOC_RESPONDER_WAIT_TRANS_ID:
                // latch in acknowledge message
                _cur_ack.trans_id = _cur_cmd.trans_id;

                // advance state and pointer
                _state = NOC_RESPONDER_WAIT_EKEY;
                data += 1;
                break;

            case NOC_RESPONDER_WAIT_EKEY:
                if (_cur_cmd.ekey != NOC_CMD_EKEY) _cur_ack.status |= NOC_STAT_ERR_KEY;
                if (((uint32_t)_cur_cmd.chksum) != ((uint32_t)CALC_CMD_CHKSUM(_cur_cmd))) _cur_ack.status |= NOC_STAT_ERR_CHKSM;

                // latch in acknowledge message
                _cur_ack.ekey = NOC_CMD_EKEY;
                _cur_ack.chksum = (uint32_t)CALC_CMD_CHKSUM(_cur_ack);
                
                proc_if->configure(_cur_cmd.cmd, _cur_cmd.size, src_addr);

                // advance state and pointer
                _state = NOC_RESPONDER_WAIT_DATA;
                _cur_loaded_size = 0;
                data = &cur_buf;
                break;

            case NOC_RESPONDER_WAIT_DATA:
                // advance state and pointer
                _cur_loaded_size += sizeof(noc_data_t);
                proc_if->write_packet(in_addr, cur_buf);
                if (_cur_loaded_size >= _cur_cmd.size) {
                    _state = NOC_RESPONDER_IDLE;
                    _last_packet_loaded = true;
                    data = (noc_data_t*)&_cur_cmd;
                }
                break;

            default:
                break;
            };
        }
    }
}
