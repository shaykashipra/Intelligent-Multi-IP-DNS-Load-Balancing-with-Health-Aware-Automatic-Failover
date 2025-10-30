#ifndef PC_H
#define PC_H

#include <omnetpp.h>
#include "../../core/messages/DnsMessage.h"
#include "../../core/messages/HttpMessage.h"
#include "../../core/utils/Logger.h"
#include "../../core/HealthMonitor.h"  // ADD THIS
#include <vector>

// ADD THESE ENUMS AT THE TOP OF PC.H (after includes)

class PC : public omnetpp::cSimpleModule {
private:
    int address;
    std::vector<long> serverList;
    int currentServerIndex;
    omnetpp::cMessage *startMsg;
    omnetpp::cMessage *timeoutMsg;
    bool waitingForResponse;
    int requestCount;
    int successCount;
    int failoverCount;

    // ADD THESE NEW MEMBERS:
    HealthMonitor healthMonitor;
    omnetpp::simtime_t requestSendTime;

    omnetpp::cMessage *periodicMsg;


    // 🚀 CHOICE 1: SMART FAILOVER ADDITIONS - ADD THESE 3 LINES
    std::map<RequestContext, std::vector<long>> contextSuccessHistory;
    std::vector<long> getPrioritizedServers(RequestContext context);
    double adaptiveTimeout = 3.0;  // For adaptive timeout

    RequestContext detectRequestContext(const char* path) {
       std::string pathStr(path);

       // Static content detection
       if (pathStr.find(".jpg") != std::string::npos ||
           pathStr.find(".png") != std::string::npos ||
           pathStr.find(".css") != std::string::npos ||
           pathStr.find(".js") != std::string::npos) {
           return CONTEXT_STATIC;
       }

       // API/Data detection
       if (pathStr.find("/api/") != std::string::npos ||
           pathStr.find("/data") != std::string::npos ||
           pathStr.find("/query") != std::string::npos) {
           return CONTEXT_DYNAMIC;
       }

       // Streaming detection
       if (pathStr.find("/video") != std::string::npos ||
           pathStr.find("/stream") != std::string::npos ||
           pathStr.find(".mp4") != std::string::npos) {
           return CONTEXT_STREAMING;
       }

       return CONTEXT_DEFAULT;
    }

    std::string getContextString(RequestContext context) {
       switch (context) {
           case CONTEXT_STATIC: return "STATIC";
           case CONTEXT_DYNAMIC: return "DYNAMIC";
           case CONTEXT_STREAMING: return "STREAMING";
           default: return "DEFAULT";
       }
    }

       RequestContext getCurrentRequestContext() {
             // You'll need to track the current context - add this:
             const char* demoPaths[] = {"/index.html", "/api/data", "/video/stream.mp4", "/image.jpg"};
             const char* path = demoPaths[address % 4];
             return detectRequestContext(path);
         }



protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    virtual void finish() override;

    void sendDNSQuery();
    void sendHttpRequest();
    void handleFailover();
    void processDnsResponse(omnetpp::cMessage *msg);
    void processHttpResponse(omnetpp::cMessage *msg);

public:
    PC();
    virtual ~PC();
};

#endif
