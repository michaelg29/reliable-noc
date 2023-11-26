
#include "system.h"
#include "noc_top.h"
#include "sc_trace.hpp"

#include "systemc.h"
#include <iostream>
#include <string>

sc_tracer sc_tracer::tracer;

int sc_main(int argc, char* argv[]) {
    if (!parse_cmd_line(argc, argv)) {
        return 1;
    }

    // initial state
    std::cout << "Initial state:" << std::endl;

    // ======================================
    // ===== CREATE AND CONNECT MODULES =====
    // ======================================

    noc_top top;
    top.generate_network();

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

    sc_tracer::close();

    return 0;
}
