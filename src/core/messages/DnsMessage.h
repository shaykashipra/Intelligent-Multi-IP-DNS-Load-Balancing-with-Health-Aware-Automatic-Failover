#ifndef DNSMESSAGE_H
#define DNSMESSAGE_H

#include <omnetpp.h>
#include <vector>
#include <string>
#include "MessageTypes.h"

class DnsMessage : public omnetpp::cMessage {
public:

    static DnsMessage* createQuery(const char* qname, long src, long dest) {
        auto* msg = new DnsMessage();
        msg->setName("dns_query");
        msg->setKind(DNS_QUERY);
        msg->addPar("src"); msg->par("src").setLongValue(src);
        msg->addPar("dest"); msg->par("dest").setLongValue(dest);
        msg->addPar("qname"); msg->par("qname").setStringValue(qname);
        return msg;
    }

    static DnsMessage* createResponse(const char* qname, const std::vector<long>& servers, long src, long dest) {
        auto* msg = new DnsMessage();
        msg->setName("dns_response");
        msg->setKind(DNS_RESPONSE);
        msg->addPar("src"); msg->par("src").setLongValue(src);
        msg->addPar("dest"); msg->par("dest").setLongValue(dest);
        msg->addPar("qname"); msg->par("qname").setStringValue(qname);
        msg->addPar("server_count"); msg->par("server_count").setLongValue(servers.size());

        for (size_t i = 0; i < servers.size(); i++) {
            std::string param = "server" + std::to_string(i);
            msg->addPar(param.c_str()); msg->par(param.c_str()).setLongValue(servers[i]);
        }
        return msg;
    }
};

#endif
