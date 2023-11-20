
#include "systemc.h"

#include "system.h"
#include "noc_if.h"
#include "noc_router.h"

noc_router_ctrl::noc_router_ctrl(uint32_t dir, uint32_t x, uint32_t y, bool dummy)
    : _x(x), _y(y), _in_vc_idx(0), _out_vc_idx(0),
      stats_wrapper("noc_router_ctrl", dummy ? "" : "router_ctrl_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(dir))
{
    if (!dummy) {
        // initialize virtual channels
        std::string in_vc_name = "router_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(dir) + "_in_vc_";
        std::string out_vc_name = "router_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(dir) + "_out_vc_";
        std::cout << in_vc_name << " " << out_vc_name << std::endl;
        for (int i = 0; i < NOC_N_VC; ++i) {
            _in_vc[i] = noc_vc(in_vc_name + std::to_string(i), dummy);
            _out_vc[i] = noc_vc(out_vc_name + std::to_string(i), dummy);
        }
    }
}

bool noc_router_ctrl::read_input(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
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

void noc_router_ctrl::read_output(noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
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

bool noc_router_ctrl::write_input(noc_data_t data, noc_link_ctrl_t link_ctrl) {
    // find output port
    noc_dir_e target_dir;
    if (link_ctrl.dst.x == _x && link_ctrl.dst.y == _y) {
        target_dir = NOC_DIR_TILE;
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

bool noc_router_ctrl::can_write_output() {
    for (int i = 0; i < NOC_N_VC; ++i) {
        if (!_out_vc[i].is_full()) {
            return true;
        }
    }

    return false;
}

bool noc_router_ctrl::write_output(noc_data_t data, noc_link_ctrl_t link_ctrl) {
    // find VC to write to
    int i = 0;
    _out_vc[i].enqueue(data, link_ctrl, NOC_DIR_NULL);
    return true;

    // no VC found
    return false;
}

void noc_router_ctrl::reset_stats() {

}

void noc_router_ctrl::print_report(std::ostream& ostream) {
    stats_wrapper::print_report(ostream);

    // call children
    for (int i = 0; i < NOC_N_VC; ++i) {
        _in_vc[i].print_report(ostream);
        _out_vc[i].print_report(ostream);
    }
}

void noc_router_ctrl::print_module_report(std::ostream& ostream) {

}

noc_router::noc_router(sc_module_name name, uint32_t x, uint32_t y)
    : sc_module(name), _x(x), _y(y), stats_wrapper("noc_router", name)
{
    SC_THREAD(main);
}

void noc_router::setup_ctrl() {
    for (int i = 0; i < NOC_N_DIR; ++i) {
        dir_ctrls[i] = noc_router_ctrl(i, _x, _y, ports[i]->is_dummy_if());
    }
}

void noc_router::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    dir_ctrls[(uint32_t)(dir)].read_output(data, link_ctrl);
}

void noc_router::main() {

    // port directions to read from
    noc_dir_e if_port_dirs[NOC_N_DIR];
    if_port_dirs[NOC_DIR_X_PLUS] = NOC_DIR_X_MINUS;  // read x- port from x+ router
    if_port_dirs[NOC_DIR_X_MINUS] = NOC_DIR_X_PLUS;   // read x+ port from x- router
    if_port_dirs[NOC_DIR_Y_PLUS] = NOC_DIR_Y_MINUS;  // read y- port from y+ router
    if_port_dirs[NOC_DIR_Y_MINUS] = NOC_DIR_Y_PLUS;   // read y+ port from y- router
    if_port_dirs[NOC_DIR_TILE] = NOC_DIR_TILE;     // read tile port

    // data to parse
    noc_data_t data;
    noc_link_ctrl_t link_ctrl;

    // counters
    int i, j;
    uint32_t x, y, rel;

    // status bits
    bool out_port_taken[NOC_N_DIR];

    while (true) {
        // write to input FIFOs
        for (i = 0; i < NOC_N_DIR; ++i) {
            ports[i]->read_port(if_port_dirs[i], data, link_ctrl);
            if (link_ctrl.ctrl) {
                LOGF("Packet at rtr %d %d on port %d: %08x @ %08x", _x, _y, i, data, NOC_RECOVER_RAW_ADDR(link_ctrl.dst));
                if (!dir_ctrls[i].write_input(data, link_ctrl)) {
                    // send backpressure
                }
            }
        }

        // allow other routers to grab input data
        YIELD();

        // clear status bits
        for (j = 0; j < NOC_N_DIR; ++j) {
            out_port_taken[j] = !dir_ctrls[j].can_write_output();
        }

        // find access requests for each port
        for (i = NOC_N_DIR-1; i >= 0; --i) {
            for (j = 0; j < NOC_N_DIR; ++j) {
                // if current port not taken and packet requested
                if (!out_port_taken[j] && dir_ctrls[i].read_input((noc_dir_e)j, data, link_ctrl)) {
                    // write to output FIFO
                    dir_ctrls[j].write_output(data, link_ctrl);
                    out_port_taken[j] = true;
                    break;
                }
            }
        }

        POSEDGE_NoC();
    }
}

void noc_router::reset_stats() {
    // call children
    for (int i = 0; i < NOC_N_DIR; ++i) {
        dir_ctrls[i].reset_stats();
    }
}

void noc_router::print_report(std::ostream& ostream) {
    stats_wrapper::print_report(ostream);

    // call children
    for (int i = 0; i < NOC_N_DIR; ++i) {
        dir_ctrls[i].print_report(ostream);
    }
}

void noc_router::print_module_report(std::ostream& ostream) {

}
