
#include "systemc.h"

#include "system.h"
#include "noc_adapter.h"

#ifndef NOC_TILE_H
#define NOC_TILE_H

/** NoC tile module. */
class noc_tile : public sc_module {

    public:

        /** Corresponding adapter. */
        sc_port<noc_adapter_if> adapter_if;

        /** Constructor. */
        noc_tile(sc_module_name name);

};

/** Command issuer. */
class noc_commander : public noc_tile {

    public:

        /** Constructor. */
        SC_HAS_PROCESS(noc_commander);
        noc_commander(sc_module_name name);

    private:

        /** Main thread function. */
        void main();
        void dma_main();

};

/** Command responder. */
class noc_responder : public noc_tile {

    public:

        /** Constructor. */
        SC_HAS_PROCESS(noc_responder);
        noc_responder(sc_module_name name);

    private:
    
        /** Current state. */
        noc_cmd_t _cur_cmd;
        noc_cmd_t _cur_ack;
        noc_responder_state_e _state;
        

        /** Main thread functions. */
        void recv_listener();
        void main();

};

#endif // NOC_TILE_H
