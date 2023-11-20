
#include <iostream>
#include <list>
#include <string.h>
#include <stdlib.h>

#include "systemc.h"

#include "stats_wrapper.h"

stats_wrapper::stats_wrapper(const char *typestr, const char *name) {
    if (strlen(name) == 0) {
        _name[0] = 0;
    }
    else {
        // latch type and name
        strcpy(_typestr, typestr);
        strcpy(_name, name);
    }
}

stats_wrapper::stats_wrapper(const char *typestr, std::string name)
    : stats_wrapper(typestr, name.c_str()) {}

stats_wrapper::stats_wrapper(const char *typestr, sc_module_name name)
    : stats_wrapper(typestr, (const char*)name) {}

void stats_wrapper::start_report(std::ostream& ostream) {
    ostream << "[" << std::endl;
}

void stats_wrapper::print_report(std::ostream& ostream) {
    if (!this->_name[0]) return;
    
    ostream << "  {" << std::endl
            << "    \"name\": \"" << this->_name << "\"," << std::endl
            << "    \"type\": \"" << this->_typestr << "\"," << std::endl;

    ostream << "    \"stats\": {" << std::endl;
    this->print_module_report(ostream);
    ostream << "    }" << std::endl;

    ostream << "  }," << std::endl;
}

void stats_wrapper::end_report(std::ostream& ostream) {
    ostream << "  {}" << std::endl << "]" << std::endl;
}
