
#include <iostream>
#include <list>

#include "systemc.h"

#ifndef STATS_MOD_H
#define STATS_MOD_H

#define MAX_NAME_LEN 32

class stats_wrapper;

/** Class wrapper to organize statistics collection. */
class stats_wrapper : public std::enable_shared_from_this<stats_wrapper> {

    public:

        /** Constructor. */
        stats_wrapper(const char *typestr, const char *name);
        stats_wrapper(const char *typestr, std::string name);
        stats_wrapper(const char *typestr, sc_module_name name);

        /** Reset the statistics for a module. */
        virtual void reset_stats() = 0;

        /** Print report for module in JSON format. */
        static void start_report(std::ostream& ostream);
        void print_report(std::ostream& ostream);
        static void end_report(std::ostream& ostream);
        
    protected:
    
        /** Print finalized report. */
        virtual void print_module_report(std::ostream& ostream) = 0;

    private:

        // module type and name
        char _typestr[MAX_NAME_LEN];
        char _name[MAX_NAME_LEN];

};

#endif // STATS_MOD_H
