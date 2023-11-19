
#include "system.h"
#include "noc_if.h"

noc_if::noc_if(bool dummy) : _dummy(dummy) {}

bool noc_if::is_dummy_if() {
    return _dummy;
}

noc_dummy_if::noc_dummy_if(sc_module_name name) : sc_module(name), noc_if(true) {}

void noc_dummy_if::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    link_ctrl.ctrl = false;
}
