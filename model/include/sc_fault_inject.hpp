
#include <time.h>

#include "systemc.h"

#ifndef SC_FAULT_INJECT_HPP
#define SC_FAULT_INJECT_HPP

/** Representation of variable to be upset. */
struct fault_injectable_variable_t {
    uint8_t *ptr;            // pointer to variable
    uint32_t size;           // size
    uint32_t rand_threshold; // probability of fault per byte
};

class sc_fault_injector;

/** Class to inject faults into registered variables. */
class sc_fault_injector {

    public:

        /** Global singleton instance. */
        static sc_fault_injector injector;

        /** Configure maximum and minimum probability bounds. Default is 1.0f and 0.0f. */
        static void configure_bounds(float max_prob, float min_prob) {
            if (max_prob > 1.0f) max_prob = 1.0f;
            if (min_prob < 0.0f) min_prob = 0.0f;

            injector._max_prob = max_prob;
            injector._min_prob = min_prob;
        }

        /**
         * Register a variable for fault injection.
         *
         * @param value The variable into which inject faults.
         * @param prob  The probability of fault per byte per simulation step when running `simulate`.
         */
        template <class T>
        static void set_injectable(T& value, float prob = 0.01f) {
            // check bounds
            if (prob > injector._max_prob) prob = injector._max_prob;
            if (prob < injector._min_prob) prob = injector._min_prob;

            // generate structure
            fault_injectable_variable_t registration;
            registration.ptr = (uint8_t*)&value;
            registration.size = sizeof(T);
            registration.rand_threshold = (uint32_t)(prob * (double)((uint32_t)0xffffffff)); // compute integer value for threshold
            injector._var_list.push_back(registration);
        }

        /** Run the simulation, stepping step_size of the time unit before testing for injection. Return the duration. */
        static sc_time simulate(double step_size, sc_time_unit time_unit, double max_time = -1.0) {
            srand(time(0));

            double time = 0.0;
            sc_time start_time = sc_time_stamp();
            while (true) {
                // simulate a step
                sc_start(step_size, time_unit);
                time += step_size;

                // inject faults into registration list
                uint32_t rand_val;
                uint8_t fault_bit;
                for (fault_injectable_variable_t var : injector._var_list) {
                    uint8_t *ptr = var.ptr;
                    uint32_t n_bytes = var.size;
                    while (n_bytes) {
                        // probability of random fault
                        rand_val = (uint32_t)rand();
                        if (rand_val < var.rand_threshold) {
                            // determine which bit to upset
                            fault_bit = (uint32_t)(rand()) & 0x7;
                            fault_bit = 0b1 << fault_bit;

                            // perform mask
                            if ((uint32_t)rand() & 0b1) {
                                // possible 0 --> 1 fault
                                *ptr |= fault_bit;
                            }
                            else {
                                // possible 1 --> 0 fault
                                *ptr ^= fault_bit;
                            }
                        }

                        // increment counter
                        n_bytes--;
                        ptr++;
                    }
                }

                // check if done
                if ((max_time > 0.0 && time >= max_time) || !sc_pending_activity_at_current_time()) {
                    break;
                }
            }
            sc_time stop_time = sc_time_stamp();

            return stop_time - start_time;
        }

    private:

        /** Bounds. */
        float _max_prob = 1.0f;
        float _min_prob = 0.0f;

        /** Internal list. */
        std::vector<fault_injectable_variable_t> _var_list;

};

#endif // SC_FAULT_INJECT_HPP
