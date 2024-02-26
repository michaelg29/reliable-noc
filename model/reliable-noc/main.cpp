
#define NOC_MODE NOC_MODE_RELIABLE

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

// 2d3ab7cf510ad8cb 2f5ab8ae088820b8 f74947e7187f0e34 5da7e548e49835b7 886f83f06dd17dda d6fc4d1c77bc92ac 32333196932a1faa 9ec8c99c6e11f7ef 92fb7a2d94908923 cde9d4197b009c09 282d5ac19b291bc1 65ff512f07141b78 744eb876110eff9c d3aabb16bdf6b4f6 8c0c6f1a863845f1 8d003de405af9354
