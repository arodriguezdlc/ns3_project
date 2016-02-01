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

#include "Observador.h"

#include "G711GeneratorHelper.h"
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
	Address serverAddress = Address (interfaces.GetAddress (1));



	/******************************
	* Applications installation   *
	*******************************/

 
	//HERE WE HAVE TO INSTALL CLIENT APP
    G711GeneratorHelper g711 ("ns3::UdpSocketFactory", InetSocketAddress (interfaces.GetAddress (1), 9));
    ApplicationContainer g711App = g711.Install (nodes.Get(0));
    
    //nodes.Get(1)->AddApplication(&codec);
    //codec.SetRemote("ns3::UdpSocketFactory", serverAddress, port);
	
	g711App.Start (Seconds (1.0));
	g711App.Stop (Seconds (10));

    //HERE WE HAVE TO INSTALL SERVER APP
	PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (interfaces.GetAddress (1), 9));
	ApplicationContainer sinkApp = sink.Install (nodes.Get (1));
	sinkApp.Start (Seconds (1.0));
	sinkApp.Stop (Seconds (10.0));
 

	pointToPoint.EnablePcapAll ("httpGenerator");
	/*************************
	* Simulation execution   *
	**************************/

	Observador observador;
	
	nodes.Get (0) -> GetApplication(0) -> TraceConnectWithoutContext
            ("Rx", MakeCallback(&Observador::Recepcion, &observador));	
    nodes.Get (1) -> GetApplication(0) -> TraceConnectWithoutContext
            ("Rx", MakeCallback(&Observador::Recepcion, &observador));
 
	nodes.Get (1) -> GetApplication(0) -> TraceConnectWithoutContext
        ("Tx", MakeCallback(&Observador::Envio,  &observador));
	nodes.Get (0) -> GetApplication(0) -> TraceConnectWithoutContext
        ("Tx", MakeCallback(&Observador::Envio,  &observador));

	
	
	
	Simulator::Run();
	Simulator::Destroy ();
	
	
	NS_LOG_INFO("Retardo medio desde on/off hasta el sumidero: " << observador.getMediaTiempo());
	if( observador.getPaquetesPerdidos() != 0 )
		NS_LOG_ERROR("Se han perdido paquetes");
   
	std::cout <<  "Retardo medio desde on/off hasta el sumidero: " << observador.getMediaTiempo() << std::endl;
	if( observador.getPaquetesPerdidos() != 0 )
		std::cout <<    "Se han perdido paquetes: " << observador.getPaquetesPerdidos() << std::endl;
  
 

return 0;
}
