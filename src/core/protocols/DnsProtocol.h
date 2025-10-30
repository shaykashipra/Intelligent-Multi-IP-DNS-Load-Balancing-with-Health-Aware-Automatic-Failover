#ifndef DNSPROTOCOL_H
#define DNSPROTOCOL_H

#include <omnetpp.h>
#include <vector>
#include <string>
#include <cstdint>

// DNS Header Structure (RFC 1035)
struct DnsHeader {
    uint16_t id;      // Identification number
    uint16_t flags;   // Flags + OPCODE + RCODE
    uint16_t qdcount; // Number of questions
    uint16_t ancount; // Number of answer RRs
    uint16_t nscount; // Number of authority RRs
    uint16_t arcount; // Number of additional RRs

    DnsHeader() : id(0), flags(0), qdcount(0), ancount(0), nscount(0), arcount(0) {}

    // Flag manipulation methods
    void setQR(bool isResponse) {
        if (isResponse) flags |= (1 << 15);
        else flags &= ~(1 << 15);
    }

    bool isResponse() const { return flags & (1 << 15); }
    uint8_t getRCODE() const { return flags & 0x0F; } // Response code
};

// DNS Resource Record
struct DnsResourceRecord {
    std::string name;
    uint16_t type;   // A record = 1
    uint16_t class_; // IN class = 1
    uint32_t ttl;    // Time to live
    std::vector<uint8_t> rdata; // Record data (IP address for A records)

    DnsResourceRecord() : type(1), class_(1), ttl(300) {} // Default A record, IN class, 5min TTL
};

// Complete DNS Packet
class DnsPacket {
private:
    DnsHeader header;
    std::vector<std::string> questions;
    std::vector<DnsResourceRecord> answers;

public:
    // Factory methods for common use cases
    static DnsPacket createQuery(uint16_t id, const std::string& domain) {
        DnsPacket packet;
        packet.header.id = id;
        packet.header.setQR(false); // Query
        packet.header.qdcount = 1;
        packet.questions.push_back(domain);
        return packet;
    }

    static DnsPacket createResponse(uint16_t id, const std::string& domain,
                                   const std::vector<uint32_t>& ipAddresses) {
        DnsPacket packet;
        packet.header.id = id;
        packet.header.setQR(true); // Response
        packet.header.qdcount = 1;
        packet.header.ancount = ipAddresses.size();
        packet.questions.push_back(domain);

        // Create A records for each IP
        for (uint32_t ip : ipAddresses) {
            DnsResourceRecord rr;
            rr.name = domain;
            rr.type = 1; // A record
            rr.class_ = 1; // IN class
            rr.ttl = 300; // 5 minutes

            // Convert IP to network byte order (simplified - using 32-bit)
            rr.rdata.resize(4);
            rr.rdata[0] = (ip >> 24) & 0xFF;
            rr.rdata[1] = (ip >> 16) & 0xFF;
            rr.rdata[2] = (ip >> 8) & 0xFF;
            rr.rdata[3] = ip & 0xFF;

            packet.answers.push_back(rr);
        }
        return packet;
    }

    // Getters
    const DnsHeader& getHeader() const { return header; }
    const std::vector<std::string>& getQuestions() const { return questions; }
    const std::vector<DnsResourceRecord>& getAnswers() const { return answers; }

    // Convert IP addresses from answers (for client use)
    std::vector<uint32_t> getIPAddresses() const {
        std::vector<uint32_t> ips;
        for (const auto& answer : answers) {
            if (answer.type == 1 && answer.rdata.size() == 4) { // A record with IPv4
                uint32_t ip = (answer.rdata[0] << 24) | (answer.rdata[1] << 16) |
                             (answer.rdata[2] << 8) | answer.rdata[3];
                ips.push_back(ip);
            }
        }
        return ips;
    }

    // For debugging
    std::string toString() const {
        std::string result = "DNS Packet: ";
        result += header.isResponse() ? "Response" : "Query";
        result += ", ID: " + std::to_string(header.id);
        result += ", Questions: " + std::to_string(questions.size());
        result += ", Answers: " + std::to_string(answers.size());
        return result;
    }
};

#endif
