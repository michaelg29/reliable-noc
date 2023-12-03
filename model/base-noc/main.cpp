
#define NOC_MODE NOC_MODE_BASE

#include "system.h"
#include "sc_fault_inject.hpp"
#include "sc_trace.hpp"
#include "noc_top.h"

#include "systemc.h"
#include <iostream>
#include <string>

// Statistics collecting classes
sc_fault_injector sc_fault_injector::injector;
sc_tracer sc_tracer::tracer;
latency_tracker latency_tracker::tracker;

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
    sc_time duration = sc_fault_injector::simulate();

    // ===================
    // ===== CLEANUP =====
    // ===================

    std::cout << "Simulated for " << duration << std::endl;

    sc_tracer::close();
    latency_tracker::print_report();

    return 0;
}
