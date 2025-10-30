#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <omnetpp.h>
using namespace omnetpp;

class HTTPServer : public cSimpleModule {
private:
    int address;
    double baseFailureProbability;
    double currentFailureProbability;
    bool isCurrentlyFailing;
    simtime_t failureStartTime;
    double maxFailureDuration;
    double minFailureDuration;

    bool checkDynamicFailure();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
