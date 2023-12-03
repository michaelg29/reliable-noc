
#include "systemc.h"

#include "system.h"
#include "sc_trace.hpp"
#include "noc_adapter.h"

bool noc_adapter_if::read_packet(uint32_t& src_addr, uint32_t& rel_addr, noc_data_t& data) {
    return _read_packet(src_addr, rel_addr, data);
}

bool noc_adapter_if::write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n, trackable_class_e track_class) {
    if (_write_packet(src, addr, data, n)) {
        // track first packet in the burst
        if (track_class != TRACKABLE_CLASS_NONE) {
            latency_tracker::publish(track_class, data, &addr);
        }

        return true;
    }

    return false;
}
