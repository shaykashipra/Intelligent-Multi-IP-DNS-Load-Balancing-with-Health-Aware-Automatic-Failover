#ifndef HELPERS_H
#define HELPERS_H

#include <omnetpp.h>

enum {
    DNS_QUERY = 10,
    DNS_RESPONSE = 11,
    HTTP_GET = 20,
    HTTP_RESPONSE = 21
};

// Helper function to create messages
inline omnetpp::cMessage* mk(const char *name, int kind, long src, long dest) {
    auto *msg = new omnetpp::cMessage(name);
    msg->setKind(kind);
    msg->addPar("src");
    msg->par("src").setLongValue(src);
    msg->addPar("dest");
    msg->par("dest").setLongValue(dest);
    return msg;
}

// Helper function to get source address
inline long SRC(omnetpp::cMessage* msg) {
    return msg->par("src").longValue();
}

// Helper function to get destination address
inline long DST(omnetpp::cMessage* msg) {
    return msg->par("dest").longValue();
}

#endif
