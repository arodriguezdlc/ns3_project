/*
*   Simulation to test HttpGeneratorClient and Server in a simple point to
*   point escenary
*/

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
//#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "HttpGeneratorClientHelper.h"
#include "HttpGeneratorClient.h"
#include "HttpGeneratorServerHelper.h"
#include "HttpGeneratorServer.h"
#include "Observador.h"

#define PORT 9

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("simulation");

int main (int argc, char *argv[]) 
{
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Time::SetResolution (Time::US);

    //Default values
    DataRate datarate("100Mbps");
    uint32_t requestSize = 100;
    uint32_t responseSize = 10000;
    Time     delay("5ms");
    bool     tracing = true;

    //Command line parsing
    CommandLine cmd;
    cmd.AddValue ("tracing", "flag to enable/disable pcap tracing", tracing);
    cmd.AddValue ("datarate", "datarate for point to point link", datarate);
    cmd.AddValue ("requestSize", "size of Http request in bytes", requestSize);
    cmd.AddValue ("responseSize", "size of Http response in bytes", responseSize);
    cmd.AddValue ("delay", "channel delay", delay);
    cmd.Parse (argc, argv);

    /*********************
    * Scenary creation   *
    **********************/

    //Creating client nodes (client and server)
    NodeContainer nodes;
    nodes.Create (3);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue(datarate));
    csma.SetChannelAttribute ("Delay", TimeValue(delay));

    NetDeviceContainer devices;
    devices = csma.Install (nodes);

    //Install the internet stack on the nodes
    InternetStackHelper internet;
    internet.Install (nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

    /******************************
    * Applications installation   *
    *******************************/

    //HERE WE HAVE TO INSTALL CLIENT APP
    HttpGeneratorClientHelper httpClient ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (2), PORT));        
    ApplicationContainer httpClientApp1 = httpClient.Install (nodes.Get (0));
    ApplicationContainer httpClientApp2 = httpClient.Install (nodes.Get (1));

    httpClientApp1.Start (Seconds(1.0));
    httpClientApp1.Stop  (Seconds(60.0)); 
    httpClientApp2.Start (Seconds(1.0));
    httpClientApp2.Stop  (Seconds(60.0)); 

    //HERE WE HAVE TO INSTALL SERVER APP
    HttpGeneratorServerHelper httpServer ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), PORT));
    ApplicationContainer httpServerApp = httpServer.Install (nodes.Get (2));
    httpServerApp.Start (Seconds (1.0));
    httpServerApp.Stop  (Seconds (60.0));

    if (tracing) {
       csma.EnablePcapAll ("httpGenerator");
    }

    Observador observador;
    for (int i = 0; i < 3; i++) {
       nodes.Get(i)->GetApplication(0)->TraceConnectWithoutContext ("Tx", MakeCallback(&Observador::PaqueteEnviado, &observador));
    }

    /*************************
    * Simulation execution   *
    **************************/
    NS_LOG_INFO ("Start Simulation");
    Simulator::Run();
    Simulator::Destroy ();
}
