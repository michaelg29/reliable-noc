
#include <string>

#include "systemc.h"

#include "system.h"
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
    adapter_if->write_packet(0, NOC_BASE_ADDR_RESPONDER2, (noc_data_t *)&_cur_cmd, sizeof(noc_cmd_t));

    // write payload
    adapter_if->write_packet(0, NOC_BASE_ADDR_RESPONDER2, (noc_data_t *)_write_buf, _write_buf_size);
}

void noc_commander::recv_listener() {
    // NoC packets
    uint32_t src_addr;
    uint32_t rel_addr;
    noc_data_t data;

    // cursors
    uint32_t data_i;
    uint32_t out_cursor = 0;
    
    // counters
    uint32_t n_packets_cmp = 0;
    uint32_t n_bytes_cmp = 0;
    uint32_t n_err_bytes = 0;

    while (true) {
        // receive packet
        if (adapter_if->read_packet(src_addr, rel_addr, data)) {
            LOGF("Commander received request containing %016lx at %08x", data, rel_addr);
            n_packets_cmp++;

            data_i = 8;
            while (data_i--) {
                // compare bytes
                n_bytes_cmp++;
                if ((data & 0xff) != _exp_buf[out_cursor]) {
                    LOGF("Unexpected output at byte %d, expected %02x, received %02x", out_cursor, _exp_buf[out_cursor], (uint8_t)data);
                    n_err_bytes++;
                }

                // increment cursor
                out_cursor++;

                // shift out checked byte
                data >>= 8;
            }

            if (out_cursor >= _exp_buf_size) {
                LOG("Setting to IDLE");
                _state == NOC_COMMANDER_IDLE;
                break;
            }
        }
    }
    
    printf("Final report: %d packets compared, %d bytes compared, %d errors\n", n_packets_cmp, n_bytes_cmp, n_err_bytes);
    
    sc_stop();
}
