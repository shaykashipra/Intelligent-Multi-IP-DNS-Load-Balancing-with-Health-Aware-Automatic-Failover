#include <omnetpp.h>
#include "../../core/messages/MessageTypes.h"
#include "../../core/messages/DnsMessage.h"
#include "../../core/protocols/DnsProtocol.h"
#include "../../core/HealthMonitor.h"
#include <algorithm>

#include "DNS.h"

using namespace omnetpp;
using namespace std;

void DNS::initialize() {
    address = par("address");
    nextTransactionId = 1;
    loadBalanceStrategy = par("loadBalanceStrategy").intValue();

    // Initialize known servers with health monitor
    healthMonitor.recordResponse(20, 0.1, true);  // web1
    healthMonitor.recordResponse(21, 0.1, true);  // web2
    healthMonitor.recordResponse(22, 0.1, true);  // web3

    // Initialize multi-domain configuration
    initializeDomains(); // ← ADD THIS

    //  HEALTH PROBE TIMER
    healthProbeTimer = new cMessage("healthProbe");
    scheduleAt(simTime() + 15.0, healthProbeTimer);

    //  DASHBOARD TIMER
    dashboardTimer = new cMessage("dashboard"); // ← ADD THIS
    scheduleAt(simTime() + 10.0, dashboardTimer);

    EV << "DNS server " << address << " initialized with health monitoring\n";
    EV << "Load balancing strategy: " << getStrategyName(loadBalanceStrategy) << "\n";
}

void DNS::handleMessage(cMessage *msg) {
    // HEALTH PROBE HANDLING
    if (msg == healthProbeTimer) {
        performHealthProbes();
        scheduleAt(simTime() + 20.0, healthProbeTimer);
        return;
    }

    // DASHBOARD HANDLING
    if (msg == dashboardTimer) {
        printDashboard();
        scheduleAt(simTime() + 10.0, dashboardTimer); // <-- ADDED RESCHEDULE
        return;
    }

    if (msg->getKind() == DNS_QUERY) {
        long src = msg->par("src").longValue();
        string qname = msg->par("qname").stringValue();

        // EXTRACT CONTEXT INFORMATION
        RequestContext context = CONTEXT_DEFAULT;
        std::string path = "/";
        if (msg->hasPar("context")) {
            context = static_cast<RequestContext>(msg->par("context").longValue());
        }
        if (msg->hasPar("path")) {
            path = msg->par("path").stringValue();
        }

        EV << "DNS processing " << getContextString(context)
           << " context query for '" << qname << "' from " << src
           << " (path: " << path << ")\n";

        //  MULTI-DOMAIN ROUTING <-- MODIFIED
        vector<int> servers = getServersForDomain(qname, context);


        // Remove servers that are filtered by circuit breaker
        // Applied circuit breaker filtering
         vector<int> finalServers;
         for (int server : servers) {
             if (!healthMonitor.isCircuitOpen(server)) { // ← YOUR ORIGINAL METHOD!
                 finalServers.push_back(server);
             }
         }

        // If circuit breaker filtered all servers, use original list
        if (finalServers.empty()) {
            EV << "Circuit breaker filtered all servers, using original list\n";
            finalServers = servers;
        }

        // APPLIED CONTEXT-AWARE ORDERING
        vector<int> contextOrderedServers = orderServersByContext(finalServers, context);

        EV << "Context-aware DNS: " << contextOrderedServers.size() << " servers ordered for "
           << getContextString(context) << " context\n";

        // Convert to long for response
        vector<long> serverList;
        for (int serverAddr : contextOrderedServers) {
            serverList.push_back(serverAddr);
        }

        // If no servers, fall back to defaults
        if (serverList.empty()) {
            EV << "WARNING: No servers available, using fallback list\n";
            serverList = {20, 21, 22};
        }

        auto* response = DnsMessage::createResponse(qname.c_str(), serverList, address, src);

        // ADDED CONTEXT INFO TO RESPONSE FOR DEMONSTRATION
        response->addPar("context_strategy");
        response->par("context_strategy").setStringValue(getContextString(context).c_str());

        int arrivalGate = msg->getArrivalGate()->getIndex();
        send(response, "out", arrivalGate);

        EV << "DNS sent context-aware response with " << serverList.size()
           << " servers (context: " << getContextString(context)
           << ", strategy: " << getStrategyName(loadBalanceStrategy) << ")\n";
    }
    delete msg;
}

void DNS::performHealthProbes() {
    EV << "=== PERFORMING HEALTH PROBES & RECOVERY CHECKS ===\n";

    // Get all servers using the method you added
    std::vector<int> allServers = healthMonitor.getAllTrackedServers();

    // Check for server recovery first
    for (int serverAddr : allServers) {
        healthMonitor.checkServerRecovery(serverAddr);
    }

    // Then do regular health probes
    healthMonitor.performHealthProbes();

    EV << "=== HEALTH PROBES & RECOVERY CHECKS COMPLETED ===\n";
}

// MULTI-DOMAIN METHODS
void DNS::initializeDomains() {
    // Configure different domains with different servers
    domainServers["api.example.com"] = {20, 21};      // API servers
    domainServers["static.example.com"] = {22};       // Static content
    domainServers["video.example.com"] = {20, 21, 22}; // All servers

    domainStrategies["api.example.com"] = LB_RESPONSE_TIME;
    domainStrategies["static.example.com"] = LB_WEIGHTED;
    domainStrategies["video.example.com"] = LB_ROUND_ROBIN;
}

std::vector<int> DNS::getServersForDomain(const std::string& domain, RequestContext context) {
    if (domainServers.find(domain) != domainServers.end()) {
        auto servers = domainServers[domain];
        // Apply your existing context-aware ordering
        return orderServersByContext(servers, context);
    }
    return {20, 21, 22}; // Default fallback
}

// DASHBOARD METHOD
void DNS::printDashboard() {
    EV << "\n";
    EV << "  === REAL-TIME LOAD BALANCING DASHBOARD ===\n";
    EV << " Healthy Servers: " << healthMonitor.getHealthyServers().size() << "/3\n";
    EV << " Active Strategies: Round Robin, Weighted, Response Time\n";
    EV << " Circuit Breakers: " << getActiveCircuitBreakers() << " active\n";
    EV << " Context Awareness: Static, Dynamic, Streaming\n";
    EV << " Multi-Domain: api.example.com, static.example.com, video.example.com\n";
    EV << "============================================\n";
}

// CIRCUIT BREAKER COUNT METHOD
int DNS::getActiveCircuitBreakers() {
    int count = 0;
    vector<int> servers = {20, 21, 22};
    for (int server : servers) {
        if (healthMonitor.isCircuitOpen(server)) {
            count++;
        }
    }
    return count;
}

// EXISTING METHODS (keep as is)
std::string DNS::getStrategyName(int strategy) {
    switch (strategy) {
        case LB_SEQUENTIAL: return "SEQUENTIAL";
        case LB_ROUND_ROBIN: return "ROUND_ROBIN";
        case LB_WEIGHTED: return "WEIGHTED";
        case LB_RESPONSE_TIME: return "RESPONSE_TIME";
        default: return "UNKNOWN";
    }
}

std::string DNS::getContextString(RequestContext context) {
    switch (context) {
        case CONTEXT_STATIC: return "STATIC";
        case CONTEXT_DYNAMIC: return "DYNAMIC";
        case CONTEXT_STREAMING: return "STREAMING";
        default: return "DEFAULT";
    }
}

std::vector<int> DNS::orderServersByContext(const std::vector<int>& servers, RequestContext context) {
    std::vector<int> orderedServers = servers;

    switch (context) {
        case CONTEXT_STATIC:
            std::sort(orderedServers.begin(), orderedServers.end(),
                [this](int a, int b) {
                    double rateA = healthMonitor.getServerMetrics(a).successRate;
                    double rateB = healthMonitor.getServerMetrics(b).successRate;
                    return rateA > rateB;
                });
            break;

        case CONTEXT_DYNAMIC:
            std::sort(orderedServers.begin(), orderedServers.end(),
                [this](int a, int b) {
                    double timeA = healthMonitor.getServerMetrics(a).lastResponseTime;
                    double timeB = healthMonitor.getServerMetrics(b).lastResponseTime;
                    return timeA < timeB;
                });
            break;

        case CONTEXT_STREAMING:
            std::sort(orderedServers.begin(), orderedServers.end(),
                [this](int a, int b) {
                    ServerStatus statusA = healthMonitor.getServerStatus(a);
                    ServerStatus statusB = healthMonitor.getServerStatus(b);
                    if (statusA != statusB) {
                        return statusA < statusB;
                    }
                    double rateA = healthMonitor.getServerMetrics(a).successRate;
                    double rateB = healthMonitor.getServerMetrics(b).successRate;
                    return rateA > rateB;
                });
            break;

        default:
            break;
    }

    return orderedServers;
}

Define_Module(DNS);
