
#include "systemc.h"

#ifndef NOC_IF_H
#define NOC_IF_H

/** Macros. */

// Address masks (32b = 4b + 4b + 24b)
#define NOC_ADDR_REL_MASK 0x00ffffff
#define NOC_ADDR_X_MASK   0x0f000000
#define NOC_ADDR_Y_MASK   0xf0000000
#define NOC_GET_REL_ADDR(addr) (addr & NOC_ADDR_REL_MASK)
#define NOC_GET_X_ADDR(addr)   ((addr & NOC_ADDR_X_MASK) >> 24)
#define NOC_GET_Y_ADDR(addr)   ((addr & NOC_ADDR_Y_MASK) >> 28)
#define NOC_RECOVER_RAW_ADDR(addr_struct) ((addr_struct.rel & NOC_ADDR_REL_MASK) | ((addr_struct.x << 24) & NOC_ADDR_X_MASK) | ((addr_struct.y << 28) & NOC_ADDR_Y_MASK))

/** Port direction. */
enum noc_dir_e {
    NOC_DIR_X_PLUS,
    NOC_DIR_X_MINUS,
    NOC_DIR_Y_PLUS,
    NOC_DIR_Y_MINUS,
    NOC_DIR_TILE,
    NOC_DIR_NULL
};
#define NOC_N_DIR (uint32_t)(NOC_DIR_NULL)

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

        /** Constructor. */
        noc_if(bool dummy = false);

        /**
         * Read the specific port.
         *
         * @param dir       The port to read.
         * @param data      The data on the port.
         * @param link_ctrl The link control data on the port.
         */
        virtual void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl) = 0;

        /** Whether the interface is a dummy. */
        bool is_dummy_if();

    private:

        bool _dummy;

};

class noc_dummy_if : public sc_module, public noc_if {

    public:

        /** Constructor. */
        noc_dummy_if(sc_module_name name);
        
        void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

};

#endif // NOC_IF_H
