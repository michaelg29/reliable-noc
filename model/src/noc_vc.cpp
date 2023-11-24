
#include "noc_vc.h"
#include "sc_trace.hpp"

noc_vc::noc_vc(std::string name, bool is_dummy)
    : _name(name), _head(0), _tail(0), _fifo_buf(nullptr), _enqueues(0), _dequeues(0) {
    if (!is_dummy) {
        _fifo_buf = new noc_vc_fifo_t[NOC_VC_BUF_SIZE];
        
        // initialize statistics
        sc_tracer::trace(_enqueues, name, "enqueues");
        sc_tracer::trace(_dequeues, name, "dequeues");
    }
}

bool noc_vc::is_full() {
    return (!_fifo_buf || (_tail - _head) == NOC_VC_BUF_SIZE);
}

bool noc_vc::enqueue(noc_data_t data, noc_link_ctrl_t link_ctrl, noc_dir_e pkt_dir) {
    // reject if full
    if (!_fifo_buf || (_tail - _head) == NOC_VC_BUF_SIZE) return false;

    // add at tail
    _fifo_buf[_tail & NOC_VC_PTR_MASK].data = data;
    _fifo_buf[_tail & NOC_VC_PTR_MASK].link_ctrl = link_ctrl;
    _fifo_buf[_tail & NOC_VC_PTR_MASK].pkt_dir = pkt_dir;

    // increment pointer
    _tail++;
    _enqueues++;
    std::cout << _name << " enqueues " << _enqueues << std::endl;

    return true;
}

bool noc_vc::dequeue(noc_data_t& data, noc_link_ctrl_t& link_ctrl, noc_dir_e& pkt_dir) {
    if (peek(data, link_ctrl, pkt_dir)) {
        // increment pointer
        _head++;
        _dequeues++;
        return true;
    }

    return false;
}

bool noc_vc::peek(noc_data_t& data, noc_link_ctrl_t& link_ctrl, noc_dir_e& pkt_dir) {
    // reject if empty
    if (!_fifo_buf || _tail == _head) return false;

    // read head
    data = _fifo_buf[_head & NOC_VC_PTR_MASK].data;
    link_ctrl = _fifo_buf[_head & NOC_VC_PTR_MASK].link_ctrl;
    pkt_dir = _fifo_buf[_head & NOC_VC_PTR_MASK].pkt_dir;

    return true;
}
