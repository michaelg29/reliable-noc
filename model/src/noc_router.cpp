
#include "systemc.h"

#include "system.h"
#include "noc_if.h"
#include "noc_router.h"
#include "sc_trace.hpp"

noc_router::noc_router(sc_module_name name, uint32_t x, uint32_t y)
    : sc_module(name), _x(x), _y(y)
{
    SC_THREAD(main);
}

void noc_router::setup_ctrl() {
    for (int i = 0; i < NOC_N_RDIR; ++i) {
        _rdir_ctrls[i] = noc_router_rctrl(i, _x, _y, rports[i]->is_dummy_if());
    }
    for (int i = 0; i < NOC_N_WDIR; ++i) {
        _wdir_ctrls[i] = noc_router_wctrl(i, _x, _y, false);
    }
}

void noc_router::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    _wdir_ctrls[(uint32_t)(dir)].read_output(data, link_ctrl);
}

void noc_router::main() {

    // port directions to read from
    noc_dir_e if_port_dirs[NOC_N_RDIR];
    if_port_dirs[NOC_DIR_X_PLUS] = NOC_DIR_X_MINUS; // read x- port from x+ router
    if_port_dirs[NOC_DIR_X_MINUS] = NOC_DIR_X_PLUS; // read x+ port from x- router
    if_port_dirs[NOC_DIR_Y_PLUS] = NOC_DIR_Y_MINUS; // read y- port from y+ router
    if_port_dirs[NOC_DIR_Y_MINUS] = NOC_DIR_Y_PLUS; // read y+ port from y- router
#if NOC_MODE == NOC_MODE_BASE
    if_port_dirs[NOC_DIR_TILE] = NOC_DIR_TILE;      // read tile port
#elif NOC_MODE == NOC_MODE_RELIABLE
    if_port_dirs[NOC_DIR_TILE0] = NOC_DIR_TILE0;    // read tile port
    if_port_dirs[NOC_DIR_TILE1] = NOC_DIR_TILE1;    // read tile port
    if_port_dirs[NOC_DIR_TILE2] = NOC_DIR_TILE2;    // read tile port
#endif

    // data to parse
    noc_data_t data;
    noc_link_ctrl_t link_ctrl;

    // counters
    int i, j;
    uint32_t x, y, rel;

    // status bits
    bool out_port_taken[NOC_N_WDIR];

    while (true) {
        // write to input FIFOs
        for (i = 0; i < NOC_N_RDIR; ++i) {
            rports[i]->read_port(if_port_dirs[i], data, link_ctrl);
            if (link_ctrl.ctrl) {
                //LOGF("Packet at rtr %d %d on port %d: %016lx @ %08x", _x, _y, i, data, NOC_RECOVER_RAW_ADDR(link_ctrl.dst));
                if (!_rdir_ctrls[i].write_input(data, link_ctrl)) {
                    // send backpressure
                }
            }
        }

        // allow other routers to grab input data
        YIELD();

        // clear status bits
        for (j = 0; j < NOC_N_WDIR; ++j) {
            out_port_taken[j] = !_wdir_ctrls[j].can_write_output();
        }

        // find access requests for each port
        for (i = NOC_N_RDIR-1; i >= 0; --i) {
            for (j = 0; j < NOC_N_WDIR; ++j) {
                // if current port not taken and packet requested
                if (!out_port_taken[j] && _rdir_ctrls[i].read_input((noc_dir_e)j, data, link_ctrl)) {
                    // write to output FIFO
                    _wdir_ctrls[j].write_output(data, link_ctrl);
                    out_port_taken[j] = true;
                    break;
                }
            }
        }

        POSEDGE_NoC();
    }
}
