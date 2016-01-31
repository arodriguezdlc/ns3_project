/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-interface-container.h"

#include "G711Generator.h"

#include "HttpGeneratorClientHelper.h"
#include "HttpGeneratorClient.h"
#include "HttpGeneratorServerHelper.h"
#include "HttpGeneratorServer.h"

#include "ns3/string.h"
#include "ns3/log.h"
#include <typeinfo>
#include "ns3/wave-mac-helper.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/unused.h"
#include "ns3/wifi-phy.h"
#include "ns3/ssid.h"
#include "ns3/assert.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/position-allocator.h"
#include "ns3/hierarchical-mobility-model.h"
#include "ns3/pointer.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/names.h"
#include <iostream>


#define PORTVOIP 9
#define PORTHTTP 10


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("simulation");


int
main (int argc, char *argv[])
{
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Time::SetResolution (Time::MS);


  // Parametros de la simulacion

  uint32_t nVoip = 2;
  uint32_t nHttpClient = 3;
  bool     tracing = true;


  // Preparar los parametros

  CommandLine cmd;
  cmd.AddValue ("Voip", "Número de nodos VoIP", nVoip);
  cmd.AddValue ("HttpClient", "Número de nodos cliente HTTP", nHttpClient);
  cmd.AddValue ("tracing", "flag to enable/disable pcap tracing", tracing);
  cmd.Parse (argc,argv);

  /*********************
  * Creacion escenario *
  **********************/

  // Nodos VoIP y Http
  NodeContainer VoipNodes;
  VoipNodes.Create(nVoip);
  NodeContainer HttpClientNodes;
  HttpClientNodes.Create(nHttpClient);
  
  
/*
  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
*/
/*
  // Nodos que pertenecen al enlace csma
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Add (VoipNodes);
  csmaNodes.Add (HttpClientNodes);
*/
  std::string phyMode ("DsssRate1Mbps");

NodeContainer ap;
ap.Create (1);
NodeContainer sta;
sta.Create (2);


WifiHelper wifi;
wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
// ns-3 supports RadioTap and Prism tracing extensions for 802.11
wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

YansWifiChannelHelper wifiChannel;
// reference loss must be changed since 802.11b is operating at 2.4GHz
wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
                                "Exponent", DoubleValue (3.0),
                                "ReferenceLoss", DoubleValue (40.0459));
wifiPhy.SetChannel (wifiChannel.Create ());


// Add a non-QoS upper mac, and disable rate control
NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                              "DataMode",StringValue (phyMode),
                              "ControlMode",StringValue (phyMode));

// Setup the rest of the upper mac
Ssid ssid = Ssid ("wifi-default");
// setup ap.
wifiMac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));
NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, ap);
NetDeviceContainer devices = apDevice;

// setup sta.
wifiMac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));
NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, sta);
devices.Add (staDevice);

// Configure mobility
MobilityHelper mobility;
Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
positionAlloc->Add (Vector (0.0, 0.0, 0.0));
positionAlloc->Add (Vector (5.0, 0.0, 0.0));
positionAlloc->Add (Vector (0.0, 5.0, 0.0));
mobility.SetPositionAllocator (positionAlloc);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (ap);
mobility.Install (sta);


// other set up (e.g. InternetStack, Application)



  ap.Add (VoipNodes);
  ap.Add (HttpClientNodes);
  sta.Add (VoipNodes);
  sta.Add (HttpClientNodes);



/*
  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  p2pDevices = pointToPoint.Install (p2pNodes);
*/

/*
  // Instalamos el dispositivo de red en los nodos csma
  CsmaHelper csma;
  NetDeviceContainer csmaDevices;
  csma.SetChannelAttribute ("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  csmaDevices = csma.Install (csmaNodes);
*/

  wifi80211pHelper wifihelp;
  NetDeviceContainer apDevices;
  NetDeviceContainer staDevices;
  wifihelp.SetChannelAttribute ("DataRate", StringValue("100Mbps"));
  wifihelp.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  apDevices = wifihelp.Install (ap);
  staDevices = wifihelp.Install (sta);



/*
  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);
*/
  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (ap);
  stack.Install (sta);



/*
  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace
  //      punto a punto
  //    - un rango para los nodos de la red de área local.
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);
  Ipv4InterfaceContainer csmaInterfaces;
  address.SetBase ("10.1.2.0", "255.255.255.0");
  csmaInterfaces = address.Assign (csmaDevices);
*/

  Ipv4AddressHelper address;

  Ipv4InterfaceContainer wifiInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  wifiInterfaces = address.Assign (apDevices);
  wifiInterfaces = address.Assign (staDevices);


  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /*******************************
   * Instalacion de aplicaciones *
   *******************************/

  /** Instalacion de aplicacion VoIP **/

  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), PORTVOIP))); //sumidero udp en el nodo p2p para todo lo que vaya a su ip y a ese puerto
  ApplicationContainer sinkapp = sink.Install (ap.Get (0));//modif de p2p

  G711Generator VoIPapp;
  for(uint32_t i = 0 ; i < nVoip ; i++){
    VoipNodes.Get(i)->AddApplication(&VoIPapp);
    VoIPapp.SetRemote("ns3::UdpSocketFactory", wifiInterfaces.GetAddress (0), PORTVOIP); //aplicacion Voip que envia a la ip del nodo p2p y por un puerto.
  }
  
  VoIPapp.SetStartTime (Seconds (1.0));
  VoIPapp.SetStopTime (Seconds (60.0));
  
  /** Instalacion de aplicacion HttpGenerator (cliente y servidor) **/

  // Cliente Http
  HttpGeneratorClientHelper httpClient ("ns3::TcpSocketFactory", InetSocketAddress (wifiInterfaces.GetAddress (0), PORTHTTP));        
  ApplicationContainer httpClientApp = httpClient.Install (HttpClientNodes);

  httpClientApp.Start (Seconds(1.0));
  httpClientApp.Stop  (Seconds(60.0)); 

  // Servidor Http
  HttpGeneratorServerHelper httpServer ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), PORTHTTP));
  ApplicationContainer httpServerApp = httpServer.Install (ap.Get (0));

  if (tracing) {
    wifihelp.EnablePcapAll ("project");//cambiado de p2p
  }

  /**********************
   * Empieza simulacion *
   **********************/
  
  NS_LOG_INFO ("Voy a simular");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

}
