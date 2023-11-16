
#include "systemc.h"

#ifndef NOC_IF_H
#define NOC_IF_H

/** Port direction. */
enum noc_dir_e {
    NOC_DIR_X_PLUS,
    NOC_DIR_X_MINUS,
    NOC_DIR_Y_PLUS,
    NOC_DIR_Y_MINUS,
    NOC_DIR_TILE
};

enum noc_routing_alg {
    NOC_ROUTE_X_Y,
    NOC_ROUTE_Y_X
};

/** Data port type. */
typedef uint32_t noc_data_t;

/** Address type. */
struct noc_addr_t {
    uint32_t rel;   // relative address within the module
    uint32_t x;     // x-coordinate in the network
    uint32_t y;     // y-coordinate in the network
};

/** Link control port type. */
struct noc_link_ctrl_t {
    bool ctrl;                   // whether the port is active
    bool head;                   // whether the packet is the head of the burst
    bool tail;                   // whether the packet is the tail of the burst
    noc_addr_t dst;              // destination address
    noc_addr_t src;              // source address
    noc_routing_alg routing_alg; // routing algorithm to use for the packet
};

/**
 * External interface of NoC elements. Includes data and link control ports.
 */
class noc_if : virtual public sc_interface {

    public:

        /**
         * Read the specific port.
         *
         * @param dir       The port to read.
         * @param data      The data on the port.
         * @param link_ctrl The link control data on the port.
         */
        void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

    protected:

};

#endif // NOC_IF_H
