
#include <string>

#include "systemc.h"

#include "system.h"
#include "noc_tile.h"

noc_commander::noc_commander(sc_module_name name) : noc_tile(name) {
    SC_THREAD(main);
    SC_THREAD(recv_listener);

    // read buffers
    _dsize = read_input_files(_write_buf) - AES_BLOCK_LEN - AES_256_KEY_LEN;
    read_expected_output_file(_exp_buf);
    
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
    for (int j = 0; j < _dsize; ++j, ++i) {
        printf("%02x ", _write_buf[i]);
    }
    printf("\nExpected output: ");
    for (int j = 0; j < _dsize; ++j) {
        printf("%02x ", _exp_buf[j]);
    }
    printf("\n");
}

void noc_commander::main() {
    LOG("Hello, world!");
    
    const char *msg = "Hello, world";
    
    _state = NOC_COMMANDER_IDLE;
    POSEDGE();
    
    // write command
    _cur_cmd.skey = NOC_CMD_SKEY;
    _cur_cmd.cmd = 0;
    _cur_cmd.size = AES_256_KEY_LEN + AES_BLOCK_LEN + _dsize;
    _cur_cmd.tx_addr = 0x0;
    _cur_cmd.trans_id = 1;
    _cur_cmd.status = 0;
    _cur_cmd.ekey = NOC_CMD_EKEY;
    _cur_cmd.chksum = CALC_CMD_CHKSUM(_cur_cmd);
    adapter_if->write_packet(0, BASE_ADDR_NOC_RESPONDER, (noc_data_t *)&_cur_cmd, sizeof(noc_cmd_t));
    
    // write payload
    adapter_if->write_packet(0, BASE_ADDR_NOC_RESPONDER, (noc_data_t *)_write_buf, AES_256_KEY_LEN + AES_BLOCK_LEN + _dsize);
    
    // wait for acknowledge
    while (_state != NOC_COMMANDER_IDLE) {
        POSEDGE();
    }
    
    POSEDGE();
    POSEDGE();
    POSEDGE();
    
    sc_stop();
}

void noc_commander::recv_listener() {
    uint32_t src_addr;
    uint32_t rel_addr;
    noc_data_t data;
    _state = NOC_COMMANDER_IDLE;
    
    while (true) {
        // receive packet
        if (adapter_if->read_packet(src_addr, rel_addr, data)) {
            LOGF("Received request containing %016lx at %08x", data, rel_addr);
        }
    }
}
