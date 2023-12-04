
#include <string>

#include "systemc.h"

#include "system.h"
#include "sc_trace.hpp"
#include "noc_tile.h"

noc_commander::noc_commander(sc_module_name name) : noc_tile(name) {
    SC_THREAD(main);
    SC_THREAD(recv_listener);

    // read buffers
    _write_buf_size = read_input_files(_write_buf);
    _exp_buf_size = read_expected_output_file(_exp_buf);

    // print buffers
    int i = 0;
    printf(  "Encryption key:  ");
    for (int j = 0; j < AES_256_KEY_LEN; ++j, ++i) {
        printf("%02x ", _write_buf[i]);
    }
    printf("\nIV:              ");
    for (int j = 0; j < AES_BLOCK_LEN; ++j, ++i) {
        printf("%02x ", _write_buf[i]);
    }
    printf("\nInput:           ");
    for (int j = 0; j < _write_buf_size - AES_BLOCK_LEN - AES_256_KEY_LEN; ++j, ++i) {
        printf("%02x ", _write_buf[i]);
    }
    printf("\nExpected output: ");
    for (int j = 0; j < _exp_buf_size; ++j) {
        printf("%02x ", _exp_buf[j]);
    }
    printf("\n");

    // initial state
    _state = NOC_COMMANDER_IDLE;
    _in_fifo_head = 0;
    _in_fifo_tail = 0;
}

void noc_commander::transmit_to_responders(noc_data_t *packets, uint32_t n) {
    // determine number of packets
    n = (n / NOC_DSIZE) + ((n % NOC_DSIZE) ? 1 : 0);

    // interleave requests
    uint32_t noc_responder0_addr = NOC_BASE_ADDR_RESPONDER0;
    while (n) {
        adapter_if->write_packet(0, noc_responder0_addr, packets, NOC_DSIZE, REDUNDANT_COMMAND);

        // increment counters
        n--;
        noc_responder0_addr += NOC_DSIZE;
        packets++;
    }
    
    //adapter_if->write_packet(0, NOC_BASE_ADDR_RESPONDER0, packets, n, REDUNDANT_COMMAND);
}

void noc_commander::main() {
    LOG("Hello, world!");

    POSEDGE();

    // write command
    _state = NOC_COMMANDER_WRITE_DATA;
    _cur_cmd.skey = NOC_CMD_SKEY;
    _cur_cmd.cmd = 0;
    _cur_cmd.size = _write_buf_size;
    _cur_cmd.tx_addr = 0x0;
    _cur_cmd.trans_id = 1;
    _cur_cmd.status = 0;
    _cur_cmd.ekey = NOC_CMD_EKEY;
    _cur_cmd.chksum = CALC_CMD_CHKSUM(_cur_cmd);
    transmit_to_responders((noc_data_t *)&_cur_cmd, sizeof(noc_cmd_t));

    // write payload
    transmit_to_responders((noc_data_t *)_write_buf, _write_buf_size);
}

void noc_commander::recv_listener() {
    // NoC packets
    uint32_t src_addr;
    uint32_t rel_addr;
    noc_data_t data;
    
    // comparison counters
    uint32_t n_bytes_cmp = 0;
    uint32_t n_err_bytes = 0;

    while (true) {
        // receive packet
        if (adapter_if->read_packet(src_addr, rel_addr, data)) {
            // capture in logger
            LOGF("[%s]: Received request containing %016lx to %08x from %08x", this->name(), data, rel_addr, src_addr);
            rel_addr += NOC_BASE_ADDR_COMMANDER;
            latency_tracker::capture(&data, &rel_addr);
            rel_addr -= NOC_BASE_ADDR_COMMANDER;
            
            // compare bytes
            for (int i = 0; i < NOC_DSIZE;
                 ++i, data >>= 8, n_bytes_cmp++) {
                if ((uint8_t)data != _exp_buf[n_bytes_cmp]) {
                    printf("Error in byte %08x, expected %02x, got %02x\n", n_bytes_cmp, _exp_buf[n_bytes_cmp], (uint8_t)data);
                    n_err_bytes++;
                }
            }
            
            if (n_bytes_cmp >= _exp_buf_size) {
                LOG("Setting to IDLE");
                _state == NOC_COMMANDER_IDLE;
                break;
            }
        }
    }
    
    // compare received and expected buffer
    LOG("Completed simulation, checking output...");
    printf("Final report: %d bytes compared, %d errors\n", n_bytes_cmp, n_err_bytes);

    sc_stop();
}

void noc_commander::recv_processor() {
    
}
