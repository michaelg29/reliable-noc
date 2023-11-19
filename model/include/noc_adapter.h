
#include "systemc.h"

#include "noc_if.h"

#ifndef NOC_ADAPTER_H
#define NOC_ADAPTER_H

/** Interface from the tile to the NoC adapter. */
class noc_adapter_if : virtual public sc_interface {

    public:

        /**
         * Write a data packet to the network.
         *
         * @param src  The source address relative to the current tile.
         * @param addr The base address to write to.
         * @param data The array of data to write.
         * @param n    The number of data packets to write.
         * @retval Whether the transmission was successful.
         */
        virtual bool write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n) = 0;

        /**
         * Wait for data reception.
         *
         * @param rel_addr The relative address of the packet destination.
         * @param data     The packet data.
         * @param has_more Whether there is more data in the transmission.
         * @retval Whether the packet was valid.
         */
        virtual bool read_packet(uint32_t& rel_addr, noc_data_t& data, bool& has_more) = 0;

};

/** NoC adapter module. */
class noc_adapter : public sc_module, public noc_if, public noc_adapter_if {

    public:

        /** Corresponding router. */
        sc_port<noc_if> router_if;

        noc_adapter(sc_module_name name, uint32_t x, uint32_t y);

        /** noc_if functions. */
        void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

        /** noc_adapter_if functions. */
        bool read_packet(uint32_t& addr, noc_data_t& data, bool& has_more);
        bool write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n);

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

};

#endif // NOC_ADAPTER_H
