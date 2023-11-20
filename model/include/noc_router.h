
#include "systemc.h"

#include "system.h"
#include "stats_wrapper.h"
#include "noc_if.h"
#include "noc_vc.h"

#ifndef NOC_ROUTER_H
#define NOC_ROUTER_H

#define NOC_N_VC 4

class noc_router_ctrl : public stats_wrapper {

    public:

        /**
         * Constructor.
         *
         * @param x     The x-coordinate of the parent router.
         * @param y     The y-coordinate of the parent router.
         * @param dummy Whether the controller is for a dummy interface.
         */
        noc_router_ctrl(uint32_t dir = 0, uint32_t x = 0, uint32_t y = 0, bool dummy = true);

        /**
         * Determine whether the input channel wants to write to the specified output direction.
         */
        bool read_input(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

        /**
         * Read the packet queued to write.
         *
         * @param data      Data.
         * @param link_ctrl Link control data.
         */
        void read_output(noc_data_t& data, noc_link_ctrl_t& link_ctrl);

        /**
         * Write a packet to the input VCs.
         *
         * @param data      Data.
         * @param link_ctrl Link control data.
         * @retval Whether an open VC was found.
         */
        bool write_input(noc_data_t data, noc_link_ctrl_t link_ctrl);
        
        /**
         * Determine whether there is space in a virtual channel.
         */
        bool can_write_output();

        /**
         * Write a packet to the output VCs.
         *
         * @param data      Data.
         * @param link_ctrl Link control data.
         * @retval Whether an open VC was found.
         */
        bool write_output(noc_data_t data, noc_link_ctrl_t link_ctrl);
        
        /** stats_wrapper functions */
        void reset_stats();
        void print_report(std::ostream& ostream);

    private:

        /** Configuration. */
        uint32_t _x;
        uint32_t _y;

        /** Virtual channels. */
        uint32_t _in_vc_idx;
        noc_vc _in_vc[NOC_N_VC];
        uint32_t _out_vc_idx;
        noc_vc _out_vc[NOC_N_VC];
        
        /** Print report. */
        void print_module_report(std::ostream& ostream);

};

/** NoC router module. */
class noc_router : public sc_module, public noc_if, public stats_wrapper {

    public:

        /** Directional ports and controllers. */
        sc_port<noc_if> ports[NOC_N_DIR];
        noc_router_ctrl dir_ctrls[NOC_N_DIR];

        /** Constructor. */
        SC_HAS_PROCESS(noc_router);
        noc_router(sc_module_name name, uint32_t x, uint32_t y);

        /** To be called after port assignment. */
        void setup_ctrl();

        /** noc_if functions. */
        void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

        /** stats_wrapper functions */
        void reset_stats();
        void print_report(std::ostream& ostream);
        
    private:

        /** Configuration. */
        uint32_t _x;
        uint32_t _y;

        /** Main thread function. */
        void main();
        
        /** Print report. */
        void print_module_report(std::ostream& ostream);

};

#endif // NOC_ROUTER_H
