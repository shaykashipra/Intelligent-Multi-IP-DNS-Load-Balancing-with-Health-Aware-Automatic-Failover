#include <omnetpp.h>
#include "helpers.h"
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

class Router : public cSimpleModule {
  private:
    map<long,int> rt; // dst -> gateIndex
  protected:
    void initialize() override {
        const char* s = par("routes").stringValue();
        stringstream ss(s ? s : "");
        string item;

        EV << "=== Router Initializing ===" << endl;
        EV << "Gate size: " << gateSize("pppg") << endl;
        EV << "Routes parameter: " << (s ? s : "NULL") << endl;

        while (getline(ss, item, ',')) {
            if (item.empty()) continue;
            long d=-1; int g=-1;
            if (sscanf(item.c_str(), "%ld:%d", &d, &g) == 2) {
                rt[d] = g;
                EV << "Route: " << d << " -> gate[" << g << "]" << endl;
            }
        }
        EV << "Total routes: " << rt.size() << endl;
    }

    void handleMessage(cMessage *msg) override {
        EV << "Router received: " << msg->getName() << " dst=" << DST(msg) << endl;

        long dst = DST(msg);
        auto it = rt.find(dst);
        if (it != rt.end()) {
            int g = it->second;
            EV << "Found route: " << dst << " -> gate[" << g << "]" << endl;
            if (g >= 0 && g < gateSize("pppg")) {
                send(msg, "pppg$o", g);
                EV << "Message routed to gate[" << g << "]" << endl;
                return;
            } else {
                EV << "ERROR: Gate index " << g << " out of range [0," << gateSize("pppg")-1 << "]" << endl;
            }
        } else {
            EV << "No route found for " << dst << ", flooding..." << endl;
        }

        // fallback: flood (skip incoming gate)
        int inIdx = msg->getArrivalGate()->getIndex();
        EV << "Flooding from gate " << inIdx << " to all other gates" << endl;
        for (int i=0; i<gateSize("pppg"); ++i) {
            if (i != inIdx) {
                EV << "Sending to gate[" << i << "]" << endl;
                send(msg->dup(), "pppg$o", i);
            }
        }
        delete msg;
    }
};
Define_Module(Router);
