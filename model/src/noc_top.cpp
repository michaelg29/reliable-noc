
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

    // create _tiles
    _tiles[0][0] = new noc_commander("main_tile");
    _tiles[0][1] = new noc_responder("responder_tile");

    // create _routers and _adapters
    for (int y = 0; y < NOC_Y_SIZE; ++y) {
        for (int x = 0; x < NOC_X_SIZE; ++x) {
            if (_tiles[y][x]) {
                _adapters[y][x] = new noc_adapter(("adapter_" + std::to_string(x) + std::to_string(y)).c_str(), x, y);
                _routers[y][x] = new noc_router(("router_" + std::to_string(x) + std::to_string(y)).c_str(), x, y);
            }
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
