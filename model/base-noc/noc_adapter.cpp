
#include "systemc.h"

#include "system.h"
#include "noc_if.h"
#include "noc_adapter.h"

noc_adapter::noc_adapter(sc_module_name name, uint32_t x, uint32_t y)
    : sc_module(name), _x(x), _y(y)
{

}

/** noc_if.read_port */
void noc_adapter::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    link_ctrl.ctrl = false;

    if (dir == NOC_DIR_TILE) {
        data = _w_data;
        link_ctrl = _w_link_ctrl;
    }
}

/** noc_adapter_if.read_packet */
bool noc_adapter::read_packet(uint32_t& rel_addr, noc_data_t& data, bool& has_more) {
    // read from the router
    POSEDGE();
    router_if->read_port(NOC_DIR_TILE, _r_data, _r_link_ctrl);

    // if packet is enabled
    if (_r_link_ctrl.ctrl) {
        // parse packet
        rel_addr = _r_link_ctrl.dst.rel;
        data = _r_data;

        // true if not the tail packet
        has_more = !_r_link_ctrl.tail;

        return true;
    }

    return false;
}

/** noc_adapter_if.write_packet */
bool noc_adapter::write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n) {
    // activate packet
    _w_link_ctrl.ctrl = true;
    _w_link_ctrl.head = true;

    while (n) {
        // construct current packet
        _w_link_ctrl.tail = (n == 1);
        _w_link_ctrl.src.rel = src;
        _w_link_ctrl.src.x = _x;
        _w_link_ctrl.src.y = _y;
        _w_link_ctrl.dst.rel = NOC_GET_REL_ADDR(addr);
        _w_link_ctrl.dst.x = NOC_GET_X_ADDR(addr);
        _w_link_ctrl.dst.y = NOC_GET_Y_ADDR(addr);
        _w_data = *data;

        // increment counters
        n--;
        addr++;
        data++;

        // wait for next CC
        POSEDGE();
    }

    // disable packet
    _w_link_ctrl.ctrl = false;

    return true;
}
