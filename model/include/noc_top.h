
#include "systemc.h"

#include "system.h"
#include "noc_tile.h"
#include "noc_adapter.h"
#include "noc_router.h"

/** Top-level module. */
class noc_top {

    public:

        /** Constructor. */
        noc_top();

        /** Generate the modules in the NoC. */
        void generate_network();

    private:

        // modules
        noc_tile *_tiles[NOC_Y_SIZE][NOC_X_SIZE];
        noc_adapter *_adapters[NOC_Y_SIZE][NOC_X_SIZE];
        noc_router *_routers[NOC_Y_SIZE][NOC_X_SIZE];

};
