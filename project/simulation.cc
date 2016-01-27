/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/node.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/drop-tail-queue.h>
#include "BitAlternante.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("simulation");


int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::MS);

  // Parametros de la simulacion

  uint32_t nG711 = 2;
  uint32_t nHttpClient = 3;

  // Preparar los parametros

  cmd.AddValue ("G711", "Número de nodos VoIP", nG711);
  cmd.AddValue ("HttpClient", "Número de nodos cliente HTTP", nHttpClient);
  cmd.Parse (argc,argv);

  // Componentes del escenario:
 
  NodeContainer G711Nodes;
  G771Nodes.create(nG711);
  NodeContainer HttpClientNodes;
  HttpClientNodes.create(nHttpClient);

  // Creamos las aplicaciones
  // Aplicacion para VoIP

  G711Generator AplicationVoIP();

  // Aplicacion para cliente Http

  HttpGeneratorClient AplicationHttpClient();

  // Apliacion para servidor Http

  HttpGeneratorServer AplicationHttpServer();

  //*********************************************************************************************************************

  // Dos dispositivos de red
  Ptr<PointToPointNetDevice> dispTx = CreateObject<PointToPointNetDevice> ();
  Ptr<PointToPointNetDevice> dispRx = CreateObject<PointToPointNetDevice> ();
  // Un canal punto a punto
  Ptr<PointToPointChannel> canal = CreateObject<PointToPointChannel> ();;
  // Una aplicación transmisora
  BitAlternanteTx transmisor(dispRx, Time("10ms"), 2048);
  // Y una receptora
  BitAlternanteRx receptor(dispTx);

  // Montamos el escenario:
  // Añadimos una cola a cada dispositivo
  dispTx->SetQueue (CreateObject<DropTailQueue> ());
  dispRx->SetQueue (CreateObject<DropTailQueue> ());
  // Añadimos cada dispositivo a su nodo
  nodoTx->AddDevice (dispTx);
  nodoRx->AddDevice (dispRx);
  // Añadimos cada aplicación a su nodo
  nodoTx->AddApplication(&transmisor);
  nodoRx->AddApplication(&receptor);
  // Asociamos los dos dispositivos al canal
  dispTx->Attach (canal);
  dispRx->Attach (canal);
  
  // Modificamos los parámetos configurables
  canal->SetAttribute ("Delay", StringValue ("2ms"));
  dispTx->SetAttribute ("DataRate", StringValue ("5Mbps"));

  // Activamos el transmisor
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (10.0));

  NS_LOG_UNCOND ("Voy a simular");
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND ("Total paquetes: " << transmisor.TotalDatos());

  return 0;
}
