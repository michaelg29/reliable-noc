
#include "systemc.h"

#include "sc_trace.hpp"
#include "noc_if.h"
#include "noc_adapter_if.h"

#ifndef NOC_ADAPTER_H
#define NOC_ADAPTER_H

/** NoC adapter module. */
class noc_adapter : public sc_module, public noc_if, public noc_adapter_if {

    public:

        /** Corresponding router. */
        sc_port<noc_if> router_if;

        noc_adapter(sc_module_name name, uint32_t x, uint32_t y);

        /** noc_if functions. */
        void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

    private:

        /** Configuration. */
        uint32_t _x;
        uint32_t _y;

        /** Incoming packets. */
        noc_data_t _r_data;
        noc_link_ctrl_t _r_link_ctrl;

        /** Outgoing packets. */
        noc_data_t _w_data;
        noc_link_ctrl_t _w_link_ctrl;

        /** noc_adapter_if functions. */
        bool _read_packet(uint32_t& src_addr, uint32_t& addr, noc_data_t& data);
        bool _write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n);

};

#endif // NOC_ADAPTER_H
