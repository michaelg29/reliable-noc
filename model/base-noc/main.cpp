
#include "system.h"

#include "systemc.h"
#include <iostream>
#include <string>

int sc_main(int argc, char* argv[]) {
    if (!parseCmdLine(argc, argv)) {
        return 1;
    }

    // initial state
    std::cout << "Hello, world!\n" << std::endl;

    // =====================================
    // ==== CREATE AND CONNECT MODULES =====
    // =====================================

    // =============================
    // ==== RUN THE SIMULATION =====
    // =============================
    sc_time startTime = sc_time_stamp();
    sc_start();
    sc_time stopTime = sc_time_stamp();

    cout << "Simulated for " << (stopTime - startTime) << endl;

    return 0;
}
