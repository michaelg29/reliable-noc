
#include "system.h"

#include "noc_if.h"
#include "noc_adapter.h"
#include "noc_tile.h"
#include "noc_router.h"

#include "systemc.h"
#include <iostream>
#include <string>

// modules
noc_tile *tiles[NOC_Y_SIZE][NOC_X_SIZE];
noc_adapter *adapters[NOC_Y_SIZE][NOC_X_SIZE];
noc_router *routers[NOC_Y_SIZE][NOC_X_SIZE];

// dummy interface
noc_dummy_if dummy_if("dummy_if");

/**
 * Generate network arrays.
 *
 * @param routers Array of nullptrs.
 */
void generate_network(noc_router *routers[NOC_Y_SIZE][NOC_X_SIZE], noc_adapter *adapters[NOC_Y_SIZE][NOC_X_SIZE], noc_tile *tiles[NOC_Y_SIZE][NOC_X_SIZE]) {
    for (int y = 0; y < NOC_Y_SIZE; ++y) {
        for (int x = 0; x < NOC_X_SIZE; ++x) {
            if (tiles[y][x]) {
                // connect tile and adapter
                tiles[y][x]->adapter_if(*adapters[y][x]);

                // connect adapter and router
                adapters[y][x]->router_if(*routers[y][x]);
                routers[y][x]->ports[NOC_DIR_TILE](*adapters[y][x]);
            }
            
            // connect router to neighbors
            if (x < NOC_X_SIZE - 1 && routers[y][x+1]) routers[y][x]->ports[NOC_DIR_X_PLUS](*routers[y][x+1]);
            else routers[y][x]->ports[NOC_DIR_X_PLUS](dummy_if);
            if (x > 0 && routers[y][x-1]) routers[y][x]->ports[NOC_DIR_X_MINUS](*routers[y][x-1]);
            else routers[y][x]->ports[NOC_DIR_X_MINUS](dummy_if);
            if (y < NOC_Y_SIZE - 1 && routers[y+1][x]) routers[y][x]->ports[NOC_DIR_Y_PLUS](*routers[y+1][x]);
            else routers[y][x]->ports[NOC_DIR_Y_PLUS](dummy_if);
            if (y > 0 && routers[y-1][x]) routers[y][x]->ports[NOC_DIR_Y_MINUS](*routers[y-1][x]);
            else routers[y][x]->ports[NOC_DIR_Y_MINUS](dummy_if);
            
            routers[y][x]->setup_ctrl();
        }
    }
}

int sc_main(int argc, char* argv[]) {
    if (!parseCmdLine(argc, argv)) {
        return 1;
    }

    // initial state
    std::cout << "Initial state:" << std::endl;

    // =====================================
    // ==== CREATE AND CONNECT MODULES =====
    // =====================================

    // create tiles
    tiles[0][0] = new noc_commander("main_tile");
    tiles[0][1] = new noc_responder("responder_tile");
    
    // create routers and adapters
    for (int y = 0; y < NOC_Y_SIZE; ++y) {
        for (int x = 0; x < NOC_X_SIZE; ++x) {
            if (tiles[y][x]) {
                adapters[y][x] = new noc_adapter(("adapter_" + std::to_string(x) + std::to_string(y)).c_str(), x, y);
                routers[y][x] = new noc_router(("router_" + std::to_string(x) + std::to_string(y)).c_str(), x, y);
            }
        }
    }
    
    // connect
    generate_network(routers, adapters, tiles);

    // =============================
    // ==== RUN THE SIMULATION =====
    // =============================
    std::cout << "Starting simulation..." << std::endl;
    sc_time startTime = sc_time_stamp();
    sc_start();
    sc_time stopTime = sc_time_stamp();

    std::cout << "Simulated for " << (stopTime - startTime) << std::endl;

    return 0;
}
