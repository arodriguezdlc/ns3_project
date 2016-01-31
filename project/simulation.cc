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

#define PORTVOIP 9;
#define PORTHTTP 10;



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("simulation");


int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::MS);

  // Parametros de la simulacion

  uint32_t nVoip = 2;
  uint32_t nHttpClient = 3;

  // Preparar los parametros

  cmd.AddValue ("Voip", "Número de nodos VoIP", nVoip);
  cmd.AddValue ("HttpClient", "Número de nodos cliente HTTP", nHttpClient);
  cmd.Parse (argc,argv);

  /*********************
  * Creacion escenario *
  **********************/

  // Nodos VoIP y Http
  NodeContainer VoipNodes;
  VoipNodes.create(nVoip);
  NodeContainer HttpClientNodes;
  HttpClientNodes.create(nHttpClient);

  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // Nodos que pertenecen al enlace csma
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Add (VoipNodes);
  csmaNodes.Add (HttpClientNodes);

  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  p2pDevices = pointToPoint.Install (p2pNodes);

  // Instalamos el dispositivo de red en los nodos csma
  CsmaHelper csma;
  NetDeviceContainer csmaDevices;
  csma.SetChannelAttribute ("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  csmaDevices = csma.Install (csmaNodes);

  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

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
  ApplicationContainer sinkapp = sink.Install (p2pNodes.Get (0));

  G711Generator VoIPapp;
  VoIPapp.SetRemote("ns3::UdpSocketFactory", p2pInterfaces.GetAddress (0), PORTVOIP); //aplicacion Voip que envia a la ip del nodo p2p y por un puerto.
  for(uint32_t i = 0 ; i < nVoip ; i++)
    VoipNodes.Get(i)->AddApplication(&VoIPapp);
  
  VoIPapp.Start (Seconds (1.0));
  VoIPapp.Stop (Seconds (60.0));
  
  /** Instalacion de aplicacion HttpGenerator (cliente y servidor) **/

  // Cliente Http
  HttpGeneratorClientHelper httpClient ("ns3::TcpSocketFactory", InetSocketAddress (p2pInterfaces.GetAddress (0), PORTHTTP));        
  ApplicationContainer httpClientApp = httpClient.Install (HttpClientNodes);

  // Servidor Http
  HttpGeneratorServerHelper httpServer ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), PORTHTTP));
  ApplicationContainer httpServerApp = httpServer.Install (p2pNodes.Get (0));

  httpClientApp.Start (Seconds(1.0));
  httpClientApp.Stop  (Seconds(60.0)); 

  /**********************
   * Empieza simulacion *
   **********************/
  
  NS_LOG_UNCOND ("Voy a simular");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
