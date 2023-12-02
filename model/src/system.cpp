
#include "system.h"
#include "sc_fault_inject.hpp"
#include "sc_trace.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>

// data file names
char *key_file_name = (char*)"../key.bin";
char *iv_file_name = (char*)"../iv.bin";
char *in_file_name = (char*)"../in.bin";
char *expected_out_file_name = (char*)"../out.bin";

// optional arguments
char *trace_file_name = NULL;
uint32_t simulation_step_size = 10;
sc_time_unit simulation_time_unit = SC_NS;

bool parse_cmd_line(int argc, char **argv) {
    // check usage
    if (argc < 1 || argc > 8) {
    std::cerr << "Usage: " << argv[0] << " [<KEY_FILE> <IV_FILE> <IN_FILE> <OUT_FILE>] [-t<TRACE_FILE>] [-s<FAULT_INJECT_STEP_SIZE> [(m|u|n|p)]]" << std::endl;
        return false;
    }

    int argi = 1;

    if (argc >= 5) {
        // latch file names
        key_file_name = argv[argi++];
        iv_file_name = argv[argi++];
        in_file_name = argv[argi++];
        expected_out_file_name = argv[argi++];
    }

    // enable tracing
    if (argi < argc && argv[argi][1] == 't') {
        sc_tracer::enable();
        trace_file_name = argv[argi++] + 2; // advance past "-t"
    }
    else {
        sc_tracer::disable();
    }

    // read step size
    if (argi < argc && argv[argi][1] == 's') {
        simulation_step_size = std::stoi((const char*)(argv[argi++] + 2)); // advance past "-s"

        // get time unit if not another option
        if (argi < argc && argv[argi][0] != '-') {
            char arg = argv[argi++][0];

            switch (arg) {
            case 'm': simulation_time_unit = SC_MS; break;
            case 'u': simulation_time_unit = SC_US; break;
            case 'n': simulation_time_unit = SC_NS; break;
            case 'p': simulation_time_unit = SC_PS; break;
            };
        }
    }

    // enable fault injector
    if (simulation_step_size > 0) {
        sc_fault_injector::enable();
        sc_fault_injector::init((double)simulation_step_size, simulation_time_unit);
    }
    else {
        sc_fault_injector::disable();
    }

    // initialize tracer with filename and time unit
    if (trace_file_name) {
        sc_tracer::init((const char*)trace_file_name, simulation_time_unit);
    }

    return true;
}

uint32_t read_input_files(uint8_t *out) {
    uint32_t n = 0;

    // read key file
    FILE *fp = fopen(key_file_name, "rb");
    if (fp) {
        n += fread(out + n, 1, 32, fp);
        fclose(fp);
    }
    else {
        return 0;
    }

    // read IV file
    fp = fopen(iv_file_name, "rb");
    if (fp) {
        n += fread(out + n, 1, 16, fp);
        fclose(fp);
    }
    else {
        return 0;
    }

    // read input file
    fp = fopen(in_file_name, "rb");
    if (fp) {
        // get file size
        fseek(fp, 0L, SEEK_END);
        uint32_t input_size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (input_size > MAX_DATA_SIZE) {
            input_size = MAX_DATA_SIZE;
        }

        n += fread(out + n, 1, input_size, fp);
        fclose(fp);
    }
    else {
        return 0;
    }

    return n;
}

uint32_t read_expected_output_file(uint8_t *out) {
    uint32_t n = 0;

    // read output file
    FILE *fp = fopen(expected_out_file_name, "rb");
    if (fp) {
        // get file size
        fseek(fp, 0L, SEEK_END);
        uint32_t output_size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (output_size > MAX_OUT_SIZE) {
            output_size = MAX_OUT_SIZE;
        }

        n += fread(out + n, 1, output_size, fp);
        fclose(fp);
    }
    else {
        return 0;
    }

    return n;
}
