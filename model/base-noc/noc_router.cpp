
#include "systemc.h"

#include "system.h"
#include "noc_if.h"
#include "noc_router.h"

noc_router::noc_router(sc_module_name name, uint32_t x, uint32_t y)
    : sc_module(name), _x(x), _y(y)
{
    SC_THREAD(main);
}

void noc_router::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {

}

void noc_router::main() {
    
    noc_if_t ifs[NOC_N_ROUTER_IF];              // list of interfaces
    noc_dir_e if_port_dirs[NOC_N_ROUTER_IF];    // port directions to read from
    noc_dir_e out_port_dirs[NOC_N_ROUTER_IF];   // requested output direction
    ifs[0] = tile_if;           dirs[0] = NOC_DIR_TILE;     // read tile port
    ifs[1] = xplus_router_if;   dirs[1] = NOC_DIR_X_MINUS;  // read x- port from x+ router
    ifs[2] = xminus_router_if;  dirs[2] = NOC_DIR_X_PLUS;   // read x+ port from x- router
    ifs[3] = yplus_router_if;   dirs[3] = NOC_DIR_Y_MINUS;  // read y- port from y+ router
    ifs[4] = yminus_router_if;  dirs[4] = NOC_DIR_Y_PLUS;   // read y+ port from y- router
    
    // data to parse
    noc_data_t data;
    noc_link_ctrl_t link_ctrl;

    // counters
    int i;
    uint32_t x, y, rel;

    while (true) {
        // write to output ports
        for (i = 0; i < NOC_ROUTER_IF; ++i) {
            if (ifs[i].out_vc.peek(data, link_ctrl)) {
                
            }
        }
        
        // write to input FIFOs
        for (i = 0; i < NOC_ROUTER_IF; ++i) {
            ifs[i].port->read_port(dirs[i], data, link_ctrl);
            if (link_ctrl.ctrl) {
                if (!ifs[i].in_vc.enqueue(data, link_ctrl)) {
                    // send backpressure
                }
            }
        }
        
        // find requested access
        for (i = 0; i < NOC_ROUTER_IF; ++i) {
            if (ifs[i].in_vc.peek(data, link_ctrl)) {
                // find output direction
            }
        }
        
        // arbitrate access
        

        DELAY(1);
    }
}
