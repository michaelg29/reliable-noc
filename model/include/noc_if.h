
#include "systemc.h"

#include "system.h"

#ifndef NOC_IF_H
#define NOC_IF_H

/** Port direction. */
enum noc_dir_e {
    NOC_DIR_X_PLUS,
    NOC_DIR_X_MINUS,
    NOC_DIR_Y_PLUS,
    NOC_DIR_Y_MINUS,
#if NOC_MODE == NOC_MODE_BASE
    NOC_DIR_TILE,
#elif NOC_MODE == NOC_MODE_REDUNDANT
    NOC_DIR_TILE0,
    NOC_DIR_TILE1,
    NOC_DIR_TILE2,
#endif
    NOC_DIR_NULL
};

/** Type of port. */
enum noc_port_mode_e {
    NOC_PORT_DUMMY, // = 0b00,
    NOC_PORT_WO,    // = 0b01
    NOC_PORT_RO,    // = 0b10,
    NOC_PORT_RW     // = 0b11,
};
#define IS_PORT_READABLE(port_mode) port_mode & NOC_PORT_RO
#define IS_PORT_WRITEABLE(port_mode) port_mode & NOC_PORT_WO

#if NOC_MODE == NOC_MODE_BASE
    #define NOC_N_WDIR (uint32_t)(NOC_DIR_NULL)
#elif NOC_MODE == NOC_MODE_REDUNDANT
    #define NOC_N_WDIR (uint32_t)(NOC_DIR_TILE1)
#endif
#define NOC_N_RDIR (uint32_t)(NOC_DIR_NULL)

enum noc_routing_alg_e {
    NOC_ROUTE_X_Y,
    NOC_ROUTE_Y_X
};

/** Link control port type. */
struct noc_link_ctrl_t {
    bool ctrl;                     // whether the port is active
    bool head;                     // whether the packet is the head of the burst
    bool tail;                     // whether the packet is the tail of the burst
    noc_addr_t dst;                // destination address
    noc_addr_t src;                // source address
    noc_routing_alg_e routing_alg; // routing algorithm to use for the packet
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
