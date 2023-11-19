
#include "systemc.h"

#include "system.h"
#include "noc_tile.h"

noc_responder::noc_responder(sc_module_name name) : noc_tile(name) {
    SC_THREAD(recv_main);
    SC_THREAD(write_main);
}

void noc_responder::recv_main() {
    uint32_t rel_addr;
    noc_data_t data;
    bool has_more;
    
    LOG("Started recv_main");

    while (true) {
        // receive packet
        if (adapter_if->read_packet(rel_addr, data, has_more)) {
            LOGF("Received %08x at %08x", data, rel_addr);
        }
    }
}

void noc_responder::write_main() {
    LOG("Started write_main");
}
