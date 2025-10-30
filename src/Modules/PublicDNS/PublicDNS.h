#ifndef PUBLICDNS_H
#define PUBLICDNS_H

#include <omnetpp.h>
using namespace omnetpp;


class PublicDNS : public omnetpp::cSimpleModule {
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
};

#endif
