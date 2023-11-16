
#include "system.h"
#include "noc_if.h"

/** Default values for the dummy port. */
void noc_if::read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) {
    data = 0;
    link_ctrl.ctrl = false;
    link_ctrl.head = false;
    link_ctrl.tail = false;
    link_ctrl.dst.rel = 0;
    link_ctrl.dst.x = 0;
    link_ctrl.dst.y = 0;
    link_ctrl.src.rel = 0;
    link_ctrl.src.x = 0;
    link_ctrl.src.y = 0;
    link_ctrl.routing_alg = NOC_ROUTE_X_Y;
}
