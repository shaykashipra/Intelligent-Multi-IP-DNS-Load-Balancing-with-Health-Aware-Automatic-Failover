#include <omnetpp.h>
#include "../../core/messages/MessageTypes.h"
#include "HTTPServer.h"

using namespace omnetpp;

void HTTPServer::initialize() {
    address = par("address");
    baseFailureProbability = 0.3;
    currentFailureProbability = baseFailureProbability;
    isCurrentlyFailing = false;
    maxFailureDuration = 60.0;
    minFailureDuration = 10.0;

    EV << "HTTP server " << address << " initialized (dynamic failures enabled)\n";
}

void HTTPServer::handleMessage(cMessage *msg) {
    if (msg->getKind() == HTTP_GET) {
        long src = msg->par("src").longValue();
        EV << "HTTP server " << address << " received request from " << src << "\n";

        bool shouldFail = checkDynamicFailure();

        if (shouldFail) {
            double processingTime = 0.1 * uniform(0.8, 1.2);
            EV << "HTTP server " << address << " FAILED to respond (processing time: "
               << processingTime << "s, dynamic failure state)\n";
        } else {
            double processingTime = 0.1 * uniform(0.8, 1.2);

            auto* response = new cMessage("http_response", HTTP_RESPONSE);
            response->addPar("src");
            response->par("src").setLongValue(address);
            response->addPar("dest");
            response->par("dest").setLongValue(src);
            response->addPar("bytes");
            response->par("bytes").setLongValue(2048);
            response->addPar("processing_time");
            response->par("processing_time").setDoubleValue(processingTime);

            int arrivalGate = msg->getArrivalGate()->getIndex();
            sendDelayed(response, processingTime, "out", arrivalGate);

            EV << "HTTP server " << address << " sent response (delay: "
               << processingTime << "s)\n";
        }
        delete msg;
    }
}

bool HTTPServer::checkDynamicFailure() {
    simtime_t now = simTime();

    if (isCurrentlyFailing) {
        simtime_t failureDuration = now - failureStartTime;
        double recoveryChance = failureDuration.dbl() / maxFailureDuration;

        if (uniform(0, 1) < recoveryChance) {
            isCurrentlyFailing = false;
            currentFailureProbability = baseFailureProbability;
            EV << "🔄 HTTP Server " << address << " recovered after "
               << failureDuration << " seconds of failure\n";
            return false;
        }
        return true;
    }

    if (uniform(0, 1) < currentFailureProbability) {
        isCurrentlyFailing = true;
        failureStartTime = now;
        currentFailureProbability = 0.9;
        EV << " HTTP Server " << address << " entered failure state\n";
        return true;
    }

    return false;
}

Define_Module(HTTPServer);
