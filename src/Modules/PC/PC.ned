package multi_ip_http_failover_system.Modules.PC;

simple PC {
    parameters:
        int address;
        double initialDelay = default(1);
        double maxRandomDelay = default(2);
        double periodicInterval = default(15);
        @display("i=device/pc");
    gates:
        input in[4];   // 0=DNS, 1=web1, 2=web2, 3=web3
        output out[4]; // 0=DNS, 1=web1, 2=web2, 3=web3
}