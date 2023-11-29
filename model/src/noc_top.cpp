
#include "system.h"
#include "noc_top.h"
#include "noc_if.h"
#include "noc_tile.h"
#include "noc_router.h"
#include "noc_adapter.h"

// dummy interface
noc_dummy_if dummy_if("dummy_if");

noc_top::noc_top() {}

void noc_top::generate_network() {
    // ==========================
    // ===== CREATE MODULES =====
    // ==========================

    // create commander tile
    _tiles[NOC_Y_COMMANDER][NOC_X_COMMANDER] = new noc_commander("commander");
    
    // create responder0 tile
    _tiles[NOC_Y_RESPONDER0][NOC_X_RESPONDER0] = new noc_responder("responder0");
    _apps[0] = new application("main_application0");
    ((noc_responder*)_tiles[NOC_Y_RESPONDER0][NOC_X_RESPONDER0])->proc_if(*_apps[0]);
    
    // create responder1 tile
    _tiles[NOC_Y_RESPONDER1][NOC_X_RESPONDER1] = new noc_responder("responder1");
    _apps[1] = new application("main_application1");
    ((noc_responder*)_tiles[NOC_Y_RESPONDER1][NOC_X_RESPONDER1])->proc_if(*_apps[1]);
    
    // create responder2 tile
    _tiles[NOC_Y_RESPONDER2][NOC_X_RESPONDER2] = new noc_responder("responder2");
    _apps[2] = new application("main_application2");
    ((noc_responder*)_tiles[NOC_Y_RESPONDER2][NOC_X_RESPONDER2])->proc_if(*_apps[2]);

    // create _routers and _adapters
    for (int y = 0; y < NOC_Y_SIZE; ++y) {
        for (int x = 0; x < NOC_X_SIZE; ++x) {
            if (_tiles[y][x]) {
                _adapters[y][x] = new noc_adapter(("adapter_" + std::to_string(x) + std::to_string(y)).c_str(), x, y);
            }

            _routers[y][x] = new noc_router(("router_" + std::to_string(x) + std::to_string(y)).c_str(), x, y);
        }
    }

    // ===========================
    // ===== CONNECT MODULES =====
    // ===========================

    for (int y = 0; y < NOC_Y_SIZE; ++y) {
        for (int x = 0; x < NOC_X_SIZE; ++x) {
            if (_tiles[y][x]) {
                // connect tile and adapter
                _tiles[y][x]->adapter_if(*_adapters[y][x]);

                // connect adapter and router
                _adapters[y][x]->router_if(*_routers[y][x]);
                _routers[y][x]->ports[NOC_DIR_TILE](*_adapters[y][x]);
            }
            else {
                _routers[y][x]->ports[NOC_DIR_TILE](dummy_if);
            }

            // connect router to neighbors
            if (x < NOC_X_SIZE - 1 && _routers[y][x+1]) _routers[y][x]->ports[NOC_DIR_X_PLUS](*_routers[y][x+1]);
            else _routers[y][x]->ports[NOC_DIR_X_PLUS](dummy_if);
            if (x > 0 && _routers[y][x-1]) _routers[y][x]->ports[NOC_DIR_X_MINUS](*_routers[y][x-1]);
            else _routers[y][x]->ports[NOC_DIR_X_MINUS](dummy_if);
            if (y < NOC_Y_SIZE - 1 && _routers[y+1][x]) _routers[y][x]->ports[NOC_DIR_Y_PLUS](*_routers[y+1][x]);
            else _routers[y][x]->ports[NOC_DIR_Y_PLUS](dummy_if);
            if (y > 0 && _routers[y-1][x]) _routers[y][x]->ports[NOC_DIR_Y_MINUS](*_routers[y-1][x]);
            else _routers[y][x]->ports[NOC_DIR_Y_MINUS](dummy_if);

            _routers[y][x]->setup_ctrl();
        }
    }
}
