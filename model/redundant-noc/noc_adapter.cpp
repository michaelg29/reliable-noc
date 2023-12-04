
#include "systemc.h"

#include "system.h"
#include "checksum.hpp"
#include "noc_if.h"
#include "noc_adapter.h"

noc_adapter::noc_adapter(sc_module_name name, uint32_t x, uint32_t y)
    : sc_module(name), _x(x), _y(y), _is_redundant(false), _out_fifo_head(0), _out_fifo_tail(0) {}

void noc_adapter::configure_redundancy(uint32_t redundant_base_addrs[3], noc_routing_alg_e routing_algs[3]) {
    _is_redundant = true;
    _base_redundant_addr = redundant_base_addrs[0];
    for (int i = 0; i < 3; ++i) {
        _redundant_dst_x[i] = NOC_GET_X_ADDR(redundant_base_addrs[i]);
        _redundant_dst_y[i] = NOC_GET_Y_ADDR(redundant_base_addrs[i]);
        _redundant_routing_alg[i] = routing_algs[i];
    }

    SC_THREAD(main);
}

/** noc_if.read_port */
void noc_adapter::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    link_ctrl = _w_link_ctrl;
    data = _w_data;

    if (dir == NOC_DIR_TILE0) {
        link_ctrl.ctrl = _w_link_ctrl_ctrl[0];
        // x and y destinations set by default in _write_packet
        //link_ctrl.dst.x = _redundant_dst_x[0];
        //link_ctrl.dst.y = _redundant_dst_y[0];
        link_ctrl.routing_alg = _redundant_routing_alg[0];
    }
    else if (_is_redundant && dir == NOC_DIR_TILE1) {
        link_ctrl.ctrl = _w_link_ctrl_ctrl[1];
        link_ctrl.dst.x = _redundant_dst_x[1];
        link_ctrl.dst.y = _redundant_dst_y[1];
        link_ctrl.routing_alg = _redundant_routing_alg[1];
    }
    else if (_is_redundant && dir == NOC_DIR_TILE2) {
        link_ctrl.ctrl = _w_link_ctrl_ctrl[2];
        link_ctrl.dst.x = _redundant_dst_x[2];
        link_ctrl.dst.y = _redundant_dst_y[2];
        link_ctrl.routing_alg = _redundant_routing_alg[2];
    }
}

/** noc_adapter_if.read_packet */
bool noc_adapter::_read_packet(uint32_t& src_addr, uint32_t& rel_addr, noc_data_t& data) {
    if (_is_redundant) {
        // dequeue from FIFO
        if (_out_fifo_head != _out_fifo_tail) {
            src_addr = _out_fifo_src_addr[_out_fifo_head & RESPONSE_FIFO_PTR_MASK];
            rel_addr = _out_fifo_rel_addr[_out_fifo_head & RESPONSE_FIFO_PTR_MASK];
            data = _out_fifo_data[_out_fifo_head & RESPONSE_FIFO_PTR_MASK];
            _out_fifo_head++;
            POSEDGE();

            return true;
        }
        POSEDGE();

        return false;
    }
    else {
        // read from the router
        router_if->read_port(NOC_DIR_TILE0, _r_data, _r_link_ctrl);
        POSEDGE();

        // if packet is enabled
        if (_r_link_ctrl.ctrl) {
            // parse packet
            src_addr = NOC_RECOVER_RAW_ADDR(_r_link_ctrl.src);
            rel_addr = _r_link_ctrl.dst.rel;
            data = _r_data;

            return true;
        }

        return false;
    }
}

/** noc_adapter_if.write_packet */
bool noc_adapter::_write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n) {
    // activate packet
    _w_link_ctrl_ctrl[0] = true;
    _w_link_ctrl.ctrl = false;
    _w_link_ctrl.head = true;

    // determine number of packets
    n = (n / NOC_DSIZE) + ((n % NOC_DSIZE) ? 1 : 0);

    // determine if redundant packet
    if (_is_redundant) {
        uint32_t rel_addr = NOC_GET_REL_ADDR(addr);

        // check if redundant packet (targeted to a redundant module)
        if (NOC_GET_X_ADDR(addr) == _redundant_dst_x[0] && NOC_GET_Y_ADDR(addr) == _redundant_dst_y[0]) {
            _w_link_ctrl_ctrl[1] = true;
            _w_link_ctrl_ctrl[2] = true;
        }
    }

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
        addr += NOC_DSIZE;
        data++;

        // wait for next CC
        POSEDGE();
        _w_link_ctrl.head = false;
    }

    // disable packet
    _w_link_ctrl_ctrl[0] = false;
    _w_link_ctrl_ctrl[1] = false;
    _w_link_ctrl_ctrl[2] = false;
    _w_link_ctrl.ctrl = false;

    return true;
}

void noc_adapter::main() {
    // NoC packets
    uint32_t rel_addr;
    noc_data_t data;

    // keep track of redundant communications
    int32_t redundant_src_idx;
    uint32_t checkpoint_size_bytes = CHECKPOINT_SIZE_PKTS * NOC_DSIZE;
    tmr_packet_status_e status;
    tmr_state_collection<noc_data_t> states(CHECKPOINT_SIZE_PKTS, MAX_OUT_SIZE / checkpoint_size_bytes);

    // buffer
    uint32_t out_addr = 0;
    noc_data_t rsp_buf[CHECKPOINT_SIZE_PKTS];

    while (true) {
        // read from the router
        router_if->read_port(NOC_DIR_TILE0, _r_data, _r_link_ctrl);
        if (_r_link_ctrl.ctrl) {
            // parse packet
            rel_addr = _r_link_ctrl.dst.rel;
            data = _r_data;

            // determine source
            for (redundant_src_idx = 2; redundant_src_idx >= 0; redundant_src_idx--) {
                if (_r_link_ctrl.src.x == _redundant_dst_x[redundant_src_idx] && _r_link_ctrl.src.y == _redundant_dst_y[redundant_src_idx]) break;
            }

            if (redundant_src_idx >= 0) {
                // update CRC
                status = states.update(redundant_src_idx, data, rsp_buf);
                if (status == TMR_STATUS_COMMIT) {
                    // enqueue in output FIFO
                    for (int i = 0; i < CHECKPOINT_SIZE_PKTS; ++i) {
                        _out_fifo_src_addr[_out_fifo_tail & RESPONSE_FIFO_PTR_MASK] = _base_redundant_addr;
                        _out_fifo_rel_addr[_out_fifo_tail & RESPONSE_FIFO_PTR_MASK] = out_addr;
                        _out_fifo_data[_out_fifo_tail &
                        RESPONSE_FIFO_PTR_MASK] = rsp_buf[i];
                        _out_fifo_tail++;

                        out_addr += NOC_DSIZE;
                    }
                }
                else if (status == TMR_STATUS_INVALID) {
                    // TODO: interrupt
                    LOGF("Error detected in checkpoint at byte %08x", out_addr);
                    tile_if->signal(1);
                }
            }
            else {
                // normal packet to enqueue in output FIFO
                _out_fifo_src_addr[_out_fifo_tail & RESPONSE_FIFO_PTR_MASK] = NOC_RECOVER_RAW_ADDR(_r_link_ctrl.src);
                _out_fifo_rel_addr[_out_fifo_tail & RESPONSE_FIFO_PTR_MASK] = rel_addr;
                _out_fifo_data[_out_fifo_tail & RESPONSE_FIFO_PTR_MASK] = data;
                _out_fifo_tail++;
            }
        }
        POSEDGE();
    }
}
