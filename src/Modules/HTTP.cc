#include <omnetpp.h>
#include "helpers.h"
#include <cstdlib>

using namespace omnetpp;
using namespace std;

class HTTP : public cSimpleModule {
private:
    int address;
    double failureProbability;
    bool isActive;
    int requestCount;

protected:
    virtual void initialize() override {
        address = par("address");
        failureProbability = (address == 3) ? 0.7 : 0.3; // Server 3 fails more often
        isActive = true;
        requestCount = 0;

        EV << "HTTP server " << address << " initialized (failure prob: " << failureProbability << ")\n";
    }

    virtual void handleMessage(cMessage *msg) override {
        if (msg->getKind() == HTTP_GET) {
            requestCount++;
            long src = SRC(msg);
            EV << "HTTP server " << address << " received request #" << requestCount << " from " << src << "\n";

            // Simulate random failure
            if (!isActive || (uniform(0, 1) < failureProbability)) {
                EV << "HTTP server " << address << " FAILED to respond (simulating downtime)\n";
                // Don't send response - this will trigger timeout on client
            } else {
                // Send successful response
                cMessage *response = mk("http_response", HTTP_RESPONSE, address, src);
                response->addPar("bytes");
                response->par("bytes").setLongValue(2048);

                // Add random delay
                simtime_t delay = uniform(0.1, 0.5);
                sendDelayed(response, delay, "out");

                EV << "HTTP server " << address << " sent response after " << delay << "s\n";
            }
        } else {
            EV << "HTTP server " << address << " received unexpected message type\n";
        }
        delete msg;
    }

    virtual void finish() override {
        EV << "HTTP server " << address << " finished, processed " << requestCount << " requests\n";
    }
};

Define_Module(HTTP);
