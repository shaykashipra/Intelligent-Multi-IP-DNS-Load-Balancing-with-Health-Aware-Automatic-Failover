#ifndef DNS_H
#define DNS_H

#include <omnetpp.h>
using namespace omnetpp;

class DNS : public cSimpleModule {
private:
    int address;

    uint16_t nextTransactionId;
    HealthMonitor healthMonitor;
    int loadBalanceStrategy;


    cMessage* healthProbeTimer;
    // PRIVATE METHOD DECLARATIONS
    std::string getStrategyName(int strategy);
    std::string getContextString(RequestContext context);
    std::vector<int> orderServersByContext(const std::vector<int>& servers, RequestContext context);


    void performHealthProbes();

    std::map<std::string, std::vector<int>> domainServers;
    std::map<std::string, LoadBalanceStrategy> domainStrategies;

    void initializeDomains();
    std::vector<int> getServersForDomain(const std::string& domain, RequestContext context); // NEW
    int getActiveCircuitBreakers();

    cMessage* dashboardTimer;
    void printDashboard();
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif
