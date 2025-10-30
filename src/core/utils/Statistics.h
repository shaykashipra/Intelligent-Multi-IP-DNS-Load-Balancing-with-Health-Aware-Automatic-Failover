#ifndef STATISTICS_H
#define STATISTICS_H

#include <omnetpp.h>

class Statistics {
public:
    static void recordFailover(const char* module, int count) {
        omnetpp::getSimulation()->getActiveEnvir()->recordScalar(module, "failovers", count);
    }

    static void recordSuccessRate(const char* module, double rate) {
        omnetpp::getSimulation()->getActiveEnvir()->recordScalar(module, "success_rate", rate);
    }
};

#endif
