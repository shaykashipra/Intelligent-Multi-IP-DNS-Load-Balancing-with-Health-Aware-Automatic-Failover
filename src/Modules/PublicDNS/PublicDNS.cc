#include <omnetpp.h>
#include "../../core/messages/DnsMessage.h"
#include "../../core/messages/MessageTypes.h"
#include "PublicDNS.h"

void PublicDNS::initialize() {
    EV << "PublicDNS: Initialized - Ready to resolve domains to Load Balancer\n";
}
void PublicDNS::handleMessage(cMessage *msg) {
    if (msg->getKind() == DNS_QUERY) {
        // Always return Load Balancer IP
        long src = msg->par("src").longValue();
        std::string qname = msg->par("qname").stringValue();

        EV << "PublicDNS: Resolving " << qname << " to Load Balancer IP\n";

        // Convert "203.0.113.10" to long (your addressing scheme)
        std::vector<long> serverList = {203011310};

        auto response = DnsMessage::createResponse(qname.c_str(), serverList, 8, src);
        send(response, "out", msg->getArrivalGate()->getIndex());
    }
    delete msg;
}

Define_Module(PublicDNS);
