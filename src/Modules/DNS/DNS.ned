
package multi_ip_http_failover_system.Modules.DNS;

simple DNS {
    parameters:
        int address;
        int loadBalanceStrategy = default(0);
        @display("i=device/server");
    gates:
        input in[3];
        output out[3];

//       input in;
//       output out;
}
