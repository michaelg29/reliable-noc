
#include "systemc.h"

#include "system.h"
#include "noc_tile.h"

noc_commander::noc_commander(sc_module_name name) : noc_tile(name) {
    SC_THREAD(main);
}

void noc_commander::main() {
    LOG("Hello, world!");
    
    const char *msg = "Hello, world";
    noc_data_t *data_arr = (noc_data_t*)msg;
    
    POSEDGE();
    
    if (adapter_if->write_packet(0, BASE_ADDR_NOC_RESPONDER, data_arr, 3)) {
        std::cout << "Wrote packet" << std::endl;
    }
    else {
        std::cout << "Could not write packet" << std::endl;
    }
    
    POSEDGE();
    POSEDGE();
    POSEDGE();
    
    sc_stop();
}
