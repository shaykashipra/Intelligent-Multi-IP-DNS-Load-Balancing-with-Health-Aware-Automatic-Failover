#ifndef HTTPMESSAGE_H
#define HTTPMESSAGE_H

#include <omnetpp.h>
#include "MessageTypes.h"

class HttpMessage : public omnetpp::cMessage {
public:

    static HttpMessage* createRequest(const char* method, const char* path, long src, long dest) {
        auto* msg = new HttpMessage();
        msg->setName("http_request");
        msg->setKind(HTTP_GET);
        msg->addPar("src"); msg->par("src").setLongValue(src);
        msg->addPar("dest"); msg->par("dest").setLongValue(dest);
        msg->addPar("method"); msg->par("method").setStringValue(method);
        msg->addPar("path"); msg->par("path").setStringValue(path);
        return msg;
    }

    static HttpMessage* createResponse(HttpStatus status, long bytes, long src, long dest) {
        auto* msg = new HttpMessage();
        msg->setName("http_response");
        msg->setKind(HTTP_RESPONSE);
        msg->addPar("src"); msg->par("src").setLongValue(src);
        msg->addPar("dest"); msg->par("dest").setLongValue(dest);
        msg->addPar("status"); msg->par("status").setLongValue(status);
        msg->addPar("bytes"); msg->par("bytes").setLongValue(bytes);
        return msg;
    }
};

#endif
