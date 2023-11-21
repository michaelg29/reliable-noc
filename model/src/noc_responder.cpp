
#include "systemc.h"

#include "system.h"
#include "noc_tile.h"

noc_responder::noc_responder(sc_module_name name) : noc_tile(name) {
    SC_THREAD(recv_listener);
    SC_THREAD(main);
}

void noc_responder::recv_listener() {
    uint32_t src_addr;
    uint32_t rel_addr;
    noc_data_t data;
    
    LOG("Started recv_listener");

    while (true) {
        // receive packet
        if (adapter_if->read_packet(src_addr, rel_addr, data)) {
            LOGF("Received request containing %016lx at %08x", data, rel_addr);
        }
    }
}

void noc_responder::main() {
    while (true) {
        
        
        POSEDGE();
    }
}
