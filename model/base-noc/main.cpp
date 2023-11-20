
#include "system.h"

#include "stats_wrapper.h"

#include "noc_top.h"

#include "systemc.h"
#include <iostream>
#include <string>

int sc_main(int argc, char* argv[]) {
    if (!parseCmdLine(argc, argv)) {
        return 1;
    }

    // initial state
    std::cout << "Initial state:" << std::endl;

    // ======================================
    // ===== CREATE AND CONNECT MODULES =====
    // ======================================
    
    noc_top top;
    top.generate_network();
    top.reset_stats();

    // ==============================
    // ===== RUN THE SIMULATION =====
    // ==============================
    
    std::cout << "Starting simulation..." << std::endl;
    sc_time startTime = sc_time_stamp();
    sc_start();
    sc_time stopTime = sc_time_stamp();
    
    // ===================
    // ===== CLEANUP =====
    // ===================

    std::cout << "Simulated for " << (stopTime - startTime) << std::endl;

    if (argc > 1) {
        std::ofstream f(argv[1]);
        std::cout << "Printing reports..." << std::endl;
        stats_wrapper::start_report(f);
        top.print_report(f);
        stats_wrapper::end_report(f);
        std::cout << "Done printing reports" << std::endl;
        f.close();
    }

    return 0;
}
