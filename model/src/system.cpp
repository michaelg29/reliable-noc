
#include "system.h"
#include "sc_trace.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>

bool parseCmdLine(int argc, char **argv) {
    // check usage
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [<TRACE_FILE>]" << std::endl;
        return false;
    }
    
    if (argc >= 2) {
        sc_tracer::enable();
        sc_tracer::init(argv[1]);
    }
    else {
        sc_tracer::disable();
    }
    
    return true;
}
