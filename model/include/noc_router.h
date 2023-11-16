
#include "systemc.h"

#include "noc_if.h"
#include "noc_vc.h"

#ifndef NOC_ROUTER_H
#define NOC_ROUTER_H

#define NOC_N_ROUTER_IF 5
#define NOC_N_VC 3

struct noc_if_t {
    sc_port<noc_if> port;
    noc_vc in_vc[NOC_N_VC];
    noc_vc out_vc[NOC_N_VC];
};

/** NoC router module. */
class noc_router : public sc_module, public noc_if {

    public:

        /** Corresponding tile. */
        noc_if_t tile_if;

        /** Neighboring routers. */
        noc_if_t xplus_router_if;
        noc_if_t xminus_router_if;
        noc_if_t yplus_router_if;
        noc_if_t yminus_router_if;

        // write to x- port of xplus router
        // xplus_router_if->write_link(NOC_DIR_X_MINUS, data, link_ctrl)

        /** Constructor. */
        SC_HAS_PROCESS(noc_router);
        noc_router(sc_module_name name, uint32_t x, uint32_t y);

        /** noc_if functions. */
        void read_port(noc_dir_e dir, noc_data_t& data, noc_link_ctrl_t& link_ctrl);

    private:

        /** Configuration. */
        uint32_t _x;
        uint32_t _y;

        /** Main thread function. */
        void main();

};

#endif // NOC_ROUTER_H
