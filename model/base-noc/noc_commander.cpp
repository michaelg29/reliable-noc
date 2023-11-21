
#include <string>

#include "systemc.h"

#include "system.h"
#include "noc_tile.h"

noc_commander::noc_commander(sc_module_name name) : noc_tile(name) {
    SC_THREAD(main);
}

void noc_commander::main() {
    LOG("Hello, world!");
    
    const char *msg = "Hello, world";
    
    POSEDGE();
    
    if (adapter_if->write_packet(0, BASE_ADDR_NOC_RESPONDER, (noc_data_t *)msg, strlen(msg))) {
        std::cout << "Wrote string to responder" << std::endl;
    }
    else {
        std::cout << "Could not write string to responder" << std::endl;
    }
    
    POSEDGE();
    POSEDGE();
    POSEDGE();
    
    sc_stop();
}

void noc_commander::dma_main() {
    
}
