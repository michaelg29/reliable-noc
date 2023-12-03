
#include "systemc.h"

#include "sc_trace.hpp"
#include "noc_if.h"

#ifndef NOC_ADAPTER_H
#define NOC_ADAPTER_H

/** Interface from the tile to the NoC adapter. */
class noc_adapter_if : virtual public sc_interface {

    public:

        /**
         * Wait for data reception.
         *
         * @param src_addr The source address of the packet.
         * @param rel_addr The relative address of the packet destination.
         * @param data     The packet data.
         * @retval Whether the packet was valid.
         */
        bool read_packet(uint32_t& src_addr, uint32_t& rel_addr, noc_data_t& data);

        /**
         * Write a data packet to the network.
         *
         * @param src         The source address relative to the current tile.
         * @param addr        The base address to write to.
         * @param data        The array of data to write.
         * @param n           The size of the payload in bytes.
         * @param op          The operation of the request.
         * @param track_class The class with which to tag the packet, leave as TRACKABLE_CLASS_NONE if no tracking.
         * @retval Whether the transmission was successful.
         */
        bool write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n, trackable_class_e track_class = TRACKABLE_CLASS_NONE);

    private:

        /** Virtual functions to override in subclasses. */
        virtual bool _read_packet(uint32_t& src_addr, uint32_t& rel_addr, noc_data_t& data) = 0;
        virtual bool _write_packet(uint32_t src, uint32_t addr, noc_data_t *data, uint32_t n) = 0;

};

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
