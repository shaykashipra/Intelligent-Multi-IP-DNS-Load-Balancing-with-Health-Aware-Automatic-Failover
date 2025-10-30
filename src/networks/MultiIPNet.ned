package multi_ip_http_failover_system.networks;

import multi_ip_http_failover_system.Modules.PC.PC;
import multi_ip_http_failover_system.Modules.DNS.DNS;
import multi_ip_http_failover_system.Modules.HTTPServer.HTTPServer;
import ned.IdealChannel;


network MultiIPNet
{
     parameters:
        int numClients = 4;

    submodules:
        // Multiple Clients (like real users)
        pc1: PC {
            parameters:
                address = 1;
                initialDelay = 1;
                maxRandomDelay = 2;
                periodicInterval = 15;
                @display("p=100,100");
        }
        pc2: PC {
            parameters:
                address = 2;
                initialDelay = 1;
                maxRandomDelay = 2;
                periodicInterval = 15;
                @display("p=100,200");
        }
        pc3: PC {
            parameters:
                address = 3;
                initialDelay = 1;
                maxRandomDelay = 2;
                periodicInterval = 15;
                @display("p=100,300");
        }

        // DNS Server (single DNS like real world)
        dns: DNS {
            parameters:
                address = 10;
                loadBalanceStrategy = 1; // ROUND_ROBIN
                @display("p=300,200");
        }

        // Multiple HTTP Servers (like server farm)
        web1: HTTPServer {
            parameters:
                address = 20;
                @display("p=500,100");
        }
        web2: HTTPServer {
            parameters:
                address = 21;
                @display("p=500,200");
        }
        web3: HTTPServer {
            parameters:
                address = 22;
                @display("p=500,300");
        }

    connections:
        //        pc.out[0] --> IdealChannel --> dns.in;
        //        dns.out --> IdealChannel --> pc.in[0];
        //        pc.out[1] --> IdealChannel --> http1.in;
        //        http1.out --> IdealChannel --> pc.in[1];
        //        pc.out[2] --> IdealChannel --> http2.in;
        //        http2.out --> IdealChannel --> pc.in[2];
//        pc2.out[0] --> IdealChannel --> http1.in[1];
//        pc2.out[1] --> IdealChannel --> http2.in[1];
//        pc2.out[2] --> IdealChannel --> dns.in[1];
//
//        http1.out[1] --> IdealChannel --> pc2.in[0];
//        http2.out[1] --> IdealChannel --> pc2.in[1];
//        dns.out[1] --> IdealChannel --> pc2.in[2];
//
//        pc1.out[0] --> IdealChannel --> dns.in[0];
//        pc1.out[1] --> IdealChannel --> http1.in[0];
//        pc1.out[2] --> IdealChannel --> http2.in[0];
//
//        dns.out[0] --> IdealChannel --> pc1.in[0];
//        http1.out[0] --> IdealChannel --> pc1.in[1];
//        http2.out[0] --> IdealChannel --> pc1.in[2];


    // All PCs connect to DNS (gate 0)
    pc1.out[0] --> IdealChannel --> dns.in[0];
    pc2.out[0] --> IdealChannel --> dns.in[1];
    pc3.out[0] --> IdealChannel --> dns.in[2];

    // DNS responses to all PCs
    dns.out[0] --> IdealChannel --> pc1.in[0];
    dns.out[1] --> IdealChannel --> pc2.in[0];
    dns.out[2] --> IdealChannel --> pc3.in[0];

    // All PCs connect to HTTP servers
    // Web1 connections
    pc1.out[1] --> IdealChannel --> web1.in[0];
    pc2.out[1] --> IdealChannel --> web1.in[1];
    pc3.out[1] --> IdealChannel --> web1.in[2];

    // Web2 connections
    pc1.out[2] --> IdealChannel --> web2.in[0];
    pc2.out[2] --> IdealChannel --> web2.in[1];
    pc3.out[2] --> IdealChannel --> web2.in[2];

    // Web3 connections
    pc1.out[3] --> IdealChannel --> web3.in[0];
    pc2.out[3] --> IdealChannel --> web3.in[1];
    pc3.out[3] --> IdealChannel --> web3.in[2];

    // HTTP server responses
    // Web1 responses
    web1.out[0] --> IdealChannel --> pc1.in[1];
    web1.out[1] --> IdealChannel --> pc2.in[1];
    web1.out[2] --> IdealChannel --> pc3.in[1];

    // Web2 responses
    web2.out[0] --> IdealChannel --> pc1.in[2];
    web2.out[1] --> IdealChannel --> pc2.in[2];
    web2.out[2] --> IdealChannel --> pc3.in[2];

    // Web3 responses
    web3.out[0] --> IdealChannel --> pc1.in[3];
    web3.out[1] --> IdealChannel --> pc2.in[3];
    web3.out[2] --> IdealChannel --> pc3.in[3];
}


