
#include "systemc.h"

#include "system.h"
#include "stats_wrapper.h"
#include "noc_tile.h"
#include "noc_adapter.h"
#include "noc_router.h"

/** Top-level module. */
class noc_top : public stats_wrapper {

    public:

        /** Constructor. */
        noc_top();

        /** Generate the modules in the NoC. */
        void generate_network();

        /** stats_wrapper functions */
        void reset_stats();
        void print_report(std::ostream& ostream);

    private:

        /** Print report. */
        void print_module_report(std::ostream& ostream);

        // modules
        noc_tile *_tiles[NOC_Y_SIZE][NOC_X_SIZE];
        noc_adapter *_adapters[NOC_Y_SIZE][NOC_X_SIZE];
        noc_router *_routers[NOC_Y_SIZE][NOC_X_SIZE];

};
