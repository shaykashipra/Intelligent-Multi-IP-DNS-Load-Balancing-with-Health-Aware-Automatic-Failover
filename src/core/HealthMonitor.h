#ifndef HEALTHMONITOR_H
#define HEALTHMONITOR_H

#include <omnetpp.h>
#include <map>
#include <vector>

using namespace omnetpp;
using namespace std;


enum LoadBalanceStrategy {
    LB_SEQUENTIAL = 0,    // Try servers in order (current behavior)
    LB_ROUND_ROBIN = 1,   // Rotate through healthy servers
    LB_WEIGHTED = 2,      // Based on server health/performance
    LB_RESPONSE_TIME = 3  // Fastest healthy server first
};

enum ServerStatus {
    SERVER_UNKNOWN = 0,
    SERVER_HEALTHY = 1,
    SERVER_UNHEALTHY = 2,
    SERVER_DEGRADED = 3
};

struct ServerMetrics {
    ServerStatus status;
    double lastResponseTime;
    int totalRequests;
    int failedRequests;
    double successRate;
    omnetpp::simtime_t lastCheckTime;

    ServerMetrics() : status(SERVER_UNKNOWN), lastResponseTime(0.0),
                     totalRequests(0), failedRequests(0), successRate(100.0) {}
};

class HealthMonitor {
private:

    std::map<int, simtime_t> lastFailureTime; // server -> when it failed
    std::map<int, bool> autoRecoveryEnabled;   // server -> can recover
    // Circuit breaker tracking
    std::map<int, simtime_t> circuitOpenUntil; // server -> time when circuit closes
    std::map<int, int> failureCount;           // server -> recent failure count


    std::map<int, ServerMetrics> serverMetrics; // serverAddress -> metrics
    double healthCheckInterval;
    double responseTimeThreshold; // Max acceptable response time in seconds
    int roundRobinIndex; // ADD THIS for round-robin tracking

    // Health probe tracking
    std::map<int, simtime_t> lastProbeTime; // server -> last probe time
    std::map<int, int> consecutiveFailures; // server -> consecutive failure count

    double simpleRandom() {
        static bool initialized = false;
        if (!initialized) {
            // Use current time as seed for random generator
            srand(static_cast<unsigned int>(time(nullptr)));
            initialized = true;
        }
        return (double)rand() / RAND_MAX;
    }
public:
    HealthMonitor(double checkInterval = 10.0, double maxResponseTime = 2.0)
        : healthCheckInterval(checkInterval), responseTimeThreshold(maxResponseTime) ,
         roundRobinIndex(0) {}


    // Get all server addresses that HealthMonitor tracks
    std::vector<int> getAllTrackedServers() {
        std::vector<int> servers;
        for (const auto& entry : serverMetrics) {
            servers.push_back(entry.first);
        }
        return servers;
    }
    //  recovery check method
      void checkServerRecovery(int serverAddress) {
          ServerMetrics& metrics = serverMetrics[serverAddress];

          // If server failed recently, check if it should recover
          if (metrics.status == SERVER_UNHEALTHY || metrics.status == SERVER_DEGRADED) {
              simtime_t timeSinceFailure = simTime() - lastFailureTime[serverAddress];

              // Convert simtime_t to double for comparison
              double timeSinceFailureDbl = timeSinceFailure.dbl();

              // Server recovers after random time (30-60 seconds)
              if (timeSinceFailure > 30.0 && timeSinceFailure < 60.0) {
                  double recoveryChance = (timeSinceFailureDbl - 30.0) / 30.0; // 0% to 100%
                  if (simpleRandom() < recoveryChance) {
                      // Server recovers!
                      metrics.status = SERVER_HEALTHY;
                      metrics.failedRequests = 0; // Reset failure count
                      consecutiveFailures[serverAddress] = 0;
                      circuitOpenUntil.erase(serverAddress);

                      EV << "SERVER RECOVERY: Server " << serverAddress
                         << " automatically recovered after " << timeSinceFailure
                         << " seconds of downtime\n";
                  }
              }
          }
      }


    // Circuit breaker methods
      bool isCircuitOpen(int serverAddress) {
          if (circuitOpenUntil.find(serverAddress) == circuitOpenUntil.end()) {
              return false;
          }
          return omnetpp::simTime() < circuitOpenUntil[serverAddress];
      }

      void recordFailureForCircuitBreaker(int serverAddress) {
          failureCount[serverAddress]++;

          // Open circuit after 3 consecutive failures
          if (failureCount[serverAddress] >= 3) {
              circuitOpenUntil[serverAddress] = omnetpp::simTime() + 30.0; // 30-second cooldown
              EV << " CIRCUIT BREAKER: Server " << serverAddress
                 << " circuit OPENED for 30 seconds (3 consecutive failures)\n";
              failureCount[serverAddress] = 0; // Reset counter
          } else {
              EV << "Circuit Breaker: Server " << serverAddress
                 << " failure count: " << failureCount[serverAddress] << "/3\n";
          }
      }

      void recordSuccessForCircuitBreaker(int serverAddress) {
          failureCount[serverAddress] = 0; // Reset on success
          circuitOpenUntil.erase(serverAddress); // Close circuit if open
      }

      std::vector<int> getServersWithCircuitBreaker() {
          std::vector<int> availableServers;
          std::vector<int> healthyServers = getHealthyServers();

          for (int server : healthyServers) {
              if (!isCircuitOpen(server)) {
                  availableServers.push_back(server);
              } else {
                  EV << "Circuit Breaker: Skipping server " << server
                     << " (circuit open until " << circuitOpenUntil[server] << ")\n";
              }
          }

          // If all circuits are open, return originals as fallback
          if (availableServers.empty() && !healthyServers.empty()) {
              EV << "🚨 CIRCUIT BREAKER: All circuits open, using fallback to original servers\n";
              return healthyServers;
          }

          return availableServers;
      }


    // Health probe methods
    bool needsHealthProbe(int serverAddress) {
        simtime_t now = omnetpp::simTime();
        if (lastProbeTime.find(serverAddress) == lastProbeTime.end()) {
            return true; // Never probed
        }
        return (now - lastProbeTime[serverAddress]) > healthCheckInterval;
    }

    void simulateHealthProbe(int serverAddress) {
        lastProbeTime[serverAddress] = omnetpp::simTime();

        // Simulate probe result (80% success rate for healthy servers)
        ServerMetrics& metrics = serverMetrics[serverAddress];
        bool success = simpleRandom() > 0.2; // 80% success rate


        if (success) {
            consecutiveFailures[serverAddress] = 0;
            double probeResponseTime = 0.05 + (simpleRandom() * 0.1); // Fast probe

            recordResponse(serverAddress, probeResponseTime, true);
            EV << "HealthMonitor: Probe SUCCESS for server " << serverAddress
               << " (response: " << probeResponseTime << "s)\n";
        } else {
            consecutiveFailures[serverAddress]++;
            recordResponse(serverAddress, 1.0, false); // Slow failed response
            EV << "HealthMonitor: Probe FAILED for server " << serverAddress
               << " (consecutive failures: " << consecutiveFailures[serverAddress] << ")\n";
        }
    }

    void performHealthProbes() {
        EV << "=== PERFORMING HEALTH PROBES ===\n";
        for (auto& entry : serverMetrics) {
            int serverAddr = entry.first;
            if (needsHealthProbe(serverAddr)) {
                simulateHealthProbe(serverAddr);
            }
        }
        EV << "=== HEALTH PROBES COMPLETED ===\n";
    }

    // Update metrics when a response is received
    void recordResponse(int serverAddress, double responseTime, bool success) {
        ServerMetrics& metrics = serverMetrics[serverAddress];
        metrics.totalRequests++;
        metrics.lastResponseTime = responseTime;

        if (success) {
            if (metrics.status == SERVER_UNKNOWN) metrics.status = SERVER_HEALTHY;
        } else {
            metrics.failedRequests++;
            lastFailureTime[serverAddress] = simTime(); // Track when failure occurred

        }

        // Calculate success rate
        metrics.successRate = (1.0 - (double)metrics.failedRequests / metrics.totalRequests) * 100.0;

        // Update status based on metrics
        if (metrics.successRate < 80.0) {
            metrics.status = SERVER_UNHEALTHY;
        } else if (metrics.successRate < 95.0 || responseTime > responseTimeThreshold) {
            metrics.status = SERVER_DEGRADED;
        } else {
            metrics.status = SERVER_HEALTHY;
        }

        metrics.lastCheckTime = omnetpp::simTime();

        EV << "HealthMonitor: Server " << serverAddress << " - Status: "
           << getStatusString(metrics.status) << ", Success: " << metrics.successRate
           << "%, Response: " << responseTime << "s\n";
    }

    // Get healthy servers (for DNS response)
    std::vector<int> getHealthyServers() {
        std::vector<int> healthyServers;

        for (const auto& entry : serverMetrics) {
            int serverAddr = entry.first;
            const ServerMetrics& metrics = entry.second;

            if (metrics.status == SERVER_HEALTHY || metrics.status == SERVER_DEGRADED) {
                healthyServers.push_back(serverAddr);
            }
        }

        // If no healthy servers, return all known servers as fallback
        if (healthyServers.empty()) {
            for (const auto& entry : serverMetrics) {
                healthyServers.push_back(entry.first);
            }
        }

        return healthyServers;
    }

    // Get server status
    ServerStatus getServerStatus(int serverAddress) {
        if (serverMetrics.find(serverAddress) != serverMetrics.end()) {
            return serverMetrics[serverAddress].status;
        }
        return SERVER_UNKNOWN;
    }

    // Get server metrics for statistics
    ServerMetrics getServerMetrics(int serverAddress) {
        return serverMetrics[serverAddress];
    }

    // For debugging
    void printStatus() {
        EV << "=== Health Monitor Status ===\n";
        for (const auto& entry : serverMetrics) {
            const ServerMetrics& metrics = entry.second;
            EV << "Server " << entry.first << ": " << getStatusString(metrics.status)
               << ", Success: " << metrics.successRate << "%, "
               << "Resp: " << metrics.lastResponseTime << "s, "
               << "Req: " << metrics.totalRequests << "\n";
        }
        EV << "=============================\n";
    }

    std::vector<int> getServersByStrategy(LoadBalanceStrategy strategy) {
           std::vector<int> healthyServers = getHealthyServers();

           if (healthyServers.empty()) {
               // Fallback to all known servers
               for (const auto& entry : serverMetrics) {
                   healthyServers.push_back(entry.first);
               }
           }

           switch (strategy) {
               case LB_SEQUENTIAL:
                   // Return as-is (original order)
                   return healthyServers;

               case LB_ROUND_ROBIN:
                   return getRoundRobinOrder(healthyServers);

               case LB_WEIGHTED:
                   return getWeightedOrder(healthyServers);

               case LB_RESPONSE_TIME:
                   return getResponseTimeOrder(healthyServers);

               default:
                   return healthyServers;
           }
    }

           // Get next server for round-robin
              int getNextRoundRobinServer(const std::vector<int>& servers) {
                  if (servers.empty()) return -1;

                  int server = servers[roundRobinIndex];
                  roundRobinIndex = (roundRobinIndex + 1) % servers.size();
                  return server;
              }

private:
              // Round-robin: rotate through servers
                  std::vector<int> getRoundRobinOrder(const std::vector<int>& servers) {
                      if (servers.empty()) return servers;

                      std::vector<int> ordered = servers;
                      std::rotate(ordered.begin(), ordered.begin() + roundRobinIndex, ordered.end()); // @suppress("Function cannot be instantiated")

                      // ADD THIS LINE to make round robin actually rotate:
                      roundRobinIndex = (roundRobinIndex + 1) % servers.size();

                      return ordered;
                  }

                  // Weighted: sort by success rate (highest first)
                  std::vector<int> getWeightedOrder(const std::vector<int>& servers) {
                      std::vector<std::pair<double, int>> weightedServers;

                      for (int server : servers) {
                          double weight = serverMetrics[server].successRate;
                          weightedServers.push_back({weight, server});
                      }

                      // Sort by weight descending (highest success rate first)
                      std::sort(weightedServers.begin(), weightedServers.end(),
                               [](const auto& a, const auto& b) { return a.first > b.first; });

                      std::vector<int> result;
                      for (const auto& pair : weightedServers) {
                          result.push_back(pair.second);
                      }
                      return result;
                  }

                  // Response-time: sort by fastest response time
                  std::vector<int> getResponseTimeOrder(const std::vector<int>& servers) {
                      std::vector<std::pair<double, int>> timedServers;

                      for (int server : servers) {
                          double responseTime = serverMetrics[server].lastResponseTime;
                          timedServers.push_back({responseTime, server});
                      }

                      // Sort by response time ascending (fastest first)
                      std::sort(timedServers.begin(), timedServers.end());

                      std::vector<int> result;
                      for (const auto& pair : timedServers) {
                          result.push_back(pair.second);
                      }
                      return result;
                  }


    std::string getStatusString(ServerStatus status) {
        switch (status) {
            case SERVER_HEALTHY: return "HEALTHY";
            case SERVER_UNHEALTHY: return "UNHEALTHY";
            case SERVER_DEGRADED: return "DEGRADED";
            default: return "UNKNOWN";
        }
    }
};

#endif
