/*
*	Simulation to test HttpGeneratorClient and Server in a simple point to
*   point escenary
*/

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/ipv4-interface-container.h"


#include "G711Generator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Simulador");

int main (int argc, char *argv[]) 
{

	//Default values
	DataRate datarate("100Mbps");
	uint32_t requestSize = 100;
	uint32_t responseSize = 10000;
	Time     delay("5ms");


	//Command line parsing
	CommandLine cmd;
	cmd.AddValue ("datarate", "datarate for point to point link", datarate);
	cmd.AddValue ("requestSize", "size of Http request in bytes", requestSize);
	cmd.AddValue ("responseSize", "size of Http response in bytes", responseSize);
	cmd.AddValue ("delay", "channel delay", delay);
	cmd.Parse (argc, argv);

	/*********************
	* Scenary creation   *
	**********************/
 

	//Creating 2 nodes (client and server)
	NodeContainer nodes;
	nodes.Create (2);

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue(datarate));
	pointToPoint.SetChannelAttribute ("Delay", TimeValue(delay));

	NetDeviceContainer devices;
	devices = pointToPoint.Install (nodes);

 
	//Install the internet stack on the nodes
	InternetStackHelper internet;
	internet.Install (nodes);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);



	/******************************
	* Applications installation   *
	*******************************/
    uint16_t port = 9;
	Ipv4Address serverAddress =  Ipv4Address::GetAny ();
 
	//HERE WE HAVE TO INSTALL CLIENT APP
    G711Generator codec;
    nodes.Get(1)->AddApplication(&codec);
    codec.SetRemote("ns3::UdpSocketFactory",serverAddress);
 

    //HERE WE HAVE TO INSTALL SERVER APP
	PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (serverAddress, port));
	ApplicationContainer sinkApp = sink.Install (nodes.Get (0));
	sinkApp.Start (Seconds (1.0));
	sinkApp.Stop (Seconds (10.0));
 

	pointToPoint.EnablePcapAll ("httpGenerator");
	/*************************
	* Simulation execution   *
	**************************/

	Simulator::Run();
	Simulator::Destroy ();

return 0;
}
