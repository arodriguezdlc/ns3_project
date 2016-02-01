/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

#include "G711Generator.h"

#include "HttpGeneratorClientHelper.h"
#include "HttpGeneratorClient.h"
#include "HttpGeneratorServerHelper.h"
#include "HttpGeneratorServer.h"

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

  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // Nodos que pertenecen a la red Wifi
  NodeContainer wifiStaNodes;
  wifiStaNodes.Add(VoipNodes);
  wifiStaNodes.Add(HttpClientNodes);
  NodeContainer wifiApNode;
  wifiApNode.Add(p2pNodes.Get(1));
 
  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  p2pDevices = pointToPoint.Install (p2pNodes);
 
  /* Configuracion para la red Wifi */
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue (3.0));
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetErrorRateModel ("ns3::NistErrorRateModel");
  phy.SetChannel (channel.Create ());
  
  WifiHelper wifi = WifiHelper::Default ();
  wifi.EnableLogComponents ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default();

  Ssid ssid = Ssid ("SSID-project");

  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "BeaconGeneration", BooleanValue (true), "BeaconInterval", TimeValue (Seconds (5)));  
  NetDeviceContainer apDevice;
  apDevice = wifi.Install (phy, mac, wifiApNode);

  // Configuramos la movilidad
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
 
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (1.0, 0.0, 0.0));
  positionAlloc->Add (Vector (1.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
 
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (p2pNodes);
  stack.Install (wifiStaNodes);

  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace
  //      punto a punto
  //    - un rango para los nodos de la red de área local wifi.
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);
  Ipv4InterfaceContainer staNodeInterfaces;
  address.SetBase ("10.1.2.0", "255.255.255.0");
  staNodeInterfaces = address.Assign (staDevices);
  Ipv4InterfaceContainer apNodeInterfaces;
  apNodeInterfaces = address.Assign (apDevice);

  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /*******************************
   * Instalacion de aplicaciones *
   *******************************/

  /** Instalacion de aplicacion VoIP **/

  // Clientes a servidor
  PacketSinkHelper sink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), PORTVOIP))); //sumidero udp en el nodo p2p para todo lo que vaya a su ip y a ese puerto
  ApplicationContainer sinkapp = sink.Install (p2pNodes.Get (0));

  G711Generator VoIPappClient[nVoip];
  for(uint32_t i = 0 ; i < nVoip ; i++){
    VoipNodes.Get(i)->AddApplication(&VoIPappClient[i]);
    VoIPappClient[i].SetRemote("ns3::UdpSocketFactory", p2pInterfaces.GetAddress (0), PORTVOIP); //aplicacion Voip que envia a la ip del nodo p2p y por un puerto.
    VoIPappClient[i].SetStartTime (Seconds (1.0));
    VoIPappClient[i].SetStopTime (Seconds (60.0));
  }

  // Servidor a cliente
  G711Generator VoIPappServer[nVoip];
  for(uint32_t i = 0 ; i < nVoip ; i++){
    p2pNodes.Get (0)->AddApplication(&VoIPappServer[i]);
    VoIPappServer[i].SetRemote("ns3::UdpSocketFactory", staNodeInterfaces.GetAddress (i), PORTVOIP); //aplicacion Voip que envia a la ip del nodo p2p y por un puerto.
    VoIPappServer[i].SetStartTime (Seconds (1.0));
    VoIPappServer[i].SetStopTime (Seconds (60.0));
  }

  sinkapp.Add(sink.Install (VoipNodes));
  
  /** Instalacion de aplicacion HttpGenerator (cliente y servidor) **/

  // Cliente Http
  HttpGeneratorClientHelper httpClient ("ns3::TcpSocketFactory", InetSocketAddress (p2pInterfaces.GetAddress (0), PORTHTTP));        
  ApplicationContainer httpClientApp = httpClient.Install (HttpClientNodes);

  httpClientApp.Start (Seconds(1.0));
  httpClientApp.Stop  (Seconds(60.0)); 

  // Servidor Http
  HttpGeneratorServerHelper httpServer ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), PORTHTTP));
  ApplicationContainer httpServerApp = httpServer.Install (p2pNodes.Get (0));

  if (tracing) {
    pointToPoint.EnablePcap ("project", p2pDevices.Get (1));
    phy.EnablePcap ("project-wifi",staDevices);
  }

  /**********************
   * Empieza simulacion *
   **********************/
  
  NS_LOG_INFO ("Voy a simular");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

}
