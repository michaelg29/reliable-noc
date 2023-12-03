
#include "systemc.h"

#include "system.h"
#include "noc_if.h"
#include "noc_router.h"
#include "sc_trace.hpp"

noc_router_rctrl::noc_router_rctrl(uint32_t dir, uint32_t x, uint32_t y, bool is_dummy)
    : _x(x), _y(y), _in_vc_idx(0)
{
    if (!is_dummy) {
        // initialize virtual channels
        std::string in_vc_name = "router_" + std::to_string(x) + "_" + std::to_string(y) + ".dir_" + std::to_string(dir) + ".in_vc_";
        for (int i = 0; i < NOC_N_VC; ++i) {
            _in_vc[i] = noc_vc(in_vc_name + std::to_string(i), false);
        }
    }
}

bool noc_router_rctrl::read_input(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    // default value
    link_ctrl.ctrl = false;

    // round-robin VC priority
    noc_dir_e target_dir;
    for (int max_i = _in_vc_idx + NOC_N_VC; _in_vc_idx < max_i; ++_in_vc_idx) {
        // peek packet and ensure packet is active and directed to the desired destination
        if (_in_vc[_in_vc_idx % NOC_N_VC].peek(data, link_ctrl, target_dir)
            && link_ctrl.ctrl && dir == target_dir) {
            _in_vc[_in_vc_idx % NOC_N_VC].dequeue(data, link_ctrl, target_dir);
            _in_vc_idx++;
            return true;
        }
    }

    return false;
}

bool noc_router_rctrl::write_input(noc_data_t data, noc_link_ctrl_t link_ctrl) {
    // find output port
    noc_dir_e target_dir;
    if (link_ctrl.dst.x == _x && link_ctrl.dst.y == _y) {
#if NOC_MODE == NOC_MODE_BASE
        target_dir = NOC_DIR_TILE;
#elif NOC_MODE == NOC_MODE_REDUNDANT
        target_dir = NOC_DIR_TILE0; // always write to initial tile port
#endif
    }
    else if (link_ctrl.routing_alg == NOC_ROUTE_X_Y) {
        if (_x < link_ctrl.dst.x) {
            target_dir = NOC_DIR_X_PLUS;
        }
        else if (_x > link_ctrl.dst.x) {
            target_dir = NOC_DIR_X_MINUS;
        }
        else if (_y < link_ctrl.dst.y) {
            target_dir = NOC_DIR_Y_PLUS;
        }
        else {
            target_dir = NOC_DIR_Y_MINUS;
        }
    }
    else if (link_ctrl.routing_alg == NOC_ROUTE_Y_X) {
        if (_y < link_ctrl.dst.y) {
            target_dir = NOC_DIR_Y_PLUS;
        }
        else if (_y > link_ctrl.dst.y) {
            target_dir = NOC_DIR_Y_MINUS;
        }
        else if (_x < link_ctrl.dst.x) {
            target_dir = NOC_DIR_X_PLUS;
        }
        else {
            target_dir = NOC_DIR_X_MINUS;
        }
    }

    // find VC to write to
    int i = 0;
    _in_vc[i].enqueue(data, link_ctrl, target_dir);
    return true;

    // no VC found
    return false;
}

noc_router_wctrl::noc_router_wctrl(uint32_t dir, uint32_t x, uint32_t y, bool is_dummy)
    : _x(x), _y(y), _out_vc_idx(0)
{
    if (!is_dummy) {
        // initialize virtual channels
        std::string out_vc_name = "router_" + std::to_string(x) + "_" + std::to_string(y) + ".dir_" + std::to_string(dir) + ".out_vc_";
        for (int i = 0; i < NOC_N_VC; ++i) {
            _out_vc[i] = noc_vc(out_vc_name + std::to_string(i), false);
        }
    }
}

void noc_router_wctrl::read_output(noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    // default value
    link_ctrl.ctrl = false;

    // round-robin VC priority
    noc_dir_e dir;
    for (int max_i = _out_vc_idx + NOC_N_VC; _out_vc_idx < max_i; ++_out_vc_idx) {
        if(_out_vc[_out_vc_idx % NOC_N_VC].dequeue(data, link_ctrl, dir)) {
            _out_vc_idx++;
            break;
        }
    }
}

bool noc_router_wctrl::can_write_output() {
    for (int i = 0; i < NOC_N_VC; ++i) {
        if (!_out_vc[i].is_full()) {
            return true;
        }
    }

    return false;
}

bool noc_router_wctrl::write_output(noc_data_t data, noc_link_ctrl_t link_ctrl) {
    // find VC to write to
    int i = 0;
    _out_vc[i].enqueue(data, link_ctrl, NOC_DIR_NULL);
    return true;

    // no VC found
    return false;
}
