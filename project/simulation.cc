/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include <ns3/gnuplot.h>

#include "G711Generator.h"
#include "G711GeneratorHelper.h"

#include "HttpGeneratorClientHelper.h"
#include "HttpGeneratorClient.h"
#include "HttpGeneratorServerHelper.h"
#include "HttpGeneratorServer.h"

#include "Observador.h"

#define PORTVOIP 9
#define PORTHTTP 10
#define T_SIMULACION 60.0
#define MUESTRAS 2
#define TSTUDENT 2.2622


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("simulation");

void simulacion (uint32_t nVoip, uint32_t nHttpClient, bool tracing, double* retardo_medio, double* porcentaje_correctos, double* factor_R);

int
main (int argc, char *argv[])
{
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Time::SetResolution (Time::NS);


  // Parametros de la simulacion

  uint32_t nVoipMax = 10;
  uint32_t nVoipMin = 7;
  uint32_t nHttpClientMax = 50;
  uint32_t pasoHttp = 10;
  uint32_t nHttpClient = 0;
  bool     tracing = false;
  Average<double> a_retardo;
  Average<double> a_correctos;
  Average<double> a_fr;
  double retardo_medio = 0;
  double porcentaje_correctos = 0;
  double factor_R = 0;

  // Preparar los parametros

  CommandLine cmd;
  cmd.AddValue ("VoipMax", "Número maximo de nodos VoIP", nVoipMax);
  cmd.AddValue ("VoipMin", "Número minimo de nodos VoIP", nVoipMin);
  cmd.AddValue ("pasoHttp", "Incremento de nodos HTTP", pasoHttp);
  cmd.AddValue ("HttpClientMax", "Número maximo de nodos cliente HTTP", nHttpClientMax);
  cmd.AddValue ("tracing", "flag to enable/disable pcap tracing", tracing);
  cmd.Parse (argc,argv);

  /*********************************************************
   * Preparamos las graficas y las diferentes simulaciones *
   *********************************************************/

  // Creamos graficas

  Gnuplot plot_correctos;
  plot_correctos.SetTitle("Porcentaje de paquetes correctamente transmitidos");
  plot_correctos.SetLegend("Nº de llamadas VoIP","Porcentaje de paquetes correctamente transmitidos (%)");
  
  Gnuplot plot_retardo;
  plot_retardo.SetTitle("Retardo medio");
  plot_retardo.SetLegend("Nº de llamadas VoIP","Retardo medio (us)");

  Gnuplot plot_fr;
  plot_fr.SetTitle("Factor R");
  plot_fr.SetLegend("Nº de llamadas VoIP","Factor R ");

  uint32_t curvas = nHttpClientMax/pasoHttp;
  
  for (uint32_t k = 1 ; k <= curvas  ; k++) {

    nHttpClient = nHttpClient + k * pasoHttp;

    NS_LOG_INFO("**************************************************");
    NS_LOG_INFO("* Comienzan las simulaciones para "<< nHttpClient <<" nodos Http *");
    NS_LOG_INFO("**************************************************");

    std::ostringstream rotulo;
    rotulo << "Simulacion con " << nHttpClient << " clientes Http";
    
    // Creamos dataset
    Gnuplot2dDataset dataset_correctos;
    dataset_correctos.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dataset_correctos.SetErrorBars (Gnuplot2dDataset::Y);
    dataset_correctos.SetTitle (rotulo.str ());
    
    Gnuplot2dDataset dataset_retardo;
    dataset_retardo.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dataset_retardo.SetErrorBars (Gnuplot2dDataset::Y);
    dataset_retardo.SetTitle (rotulo.str ());

    Gnuplot2dDataset dataset_fr;
    dataset_fr.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    dataset_fr.SetErrorBars (Gnuplot2dDataset::Y);
    dataset_fr.SetTitle (rotulo.str ());

    for (uint32_t j = nVoipMin ; j <= nVoipMax ; j++) {
      NS_LOG_INFO("*** Simulaciones con " << j << " nodos VoIP ***");
      // Reseteamos acumuladores
      a_retardo.Reset();
      a_correctos.Reset();
      a_fr.Reset();

      for (uint32_t i = 0 ; i < MUESTRAS ; i++) {
        NS_LOG_INFO("Simulacion de la muestra numero " << i+1 << "...");
        // Iniciamos simulacion
        simulacion (j, nHttpClient, tracing, &retardo_medio, &porcentaje_correctos, &factor_R);
        // Actualizamos datos
        a_retardo.Update(retardo_medio);
        a_correctos.Update(100*porcentaje_correctos);
	a_fr.Update(factor_R);
        
      }
      
      // Ajustamos intervalo de confianza y metemos los puntos en el dataset
      double z = 0;

      z=TSTUDENT*sqrt(a_correctos.Var()/(MUESTRAS));
      dataset_correctos.Add(j,a_correctos.Mean(),z);

      z=TSTUDENT*sqrt(a_retardo.Var()/(MUESTRAS));
      dataset_retardo.Add(j,a_retardo.Mean(),z);

      z=TSTUDENT*sqrt(a_fr.Var()/(MUESTRAS));
      dataset_fr.Add(j,a_fr.Mean(),z);     

    }

    // Metemos los dataset en las graficas
     plot_correctos.AddDataset (dataset_correctos);
     plot_retardo.AddDataset (dataset_retardo);
     plot_fr.AddDataset (dataset_fr);

  }
  // Primera grafica
  std::ofstream plotFile1 ("grafica_correctos.plt");
  plot_correctos.GenerateOutput (plotFile1);
  plotFile1 << "pause -1"<<std::endl;
  plotFile1.close ();

  // Segunda grafica
  std::ofstream plotFile2 ("grafica_retardo.plt");
  plot_retardo.GenerateOutput (plotFile2);
  plotFile2 << "pause -1"<<std::endl;
  plotFile2.close ();

  // Tercera grafica
  std::ofstream plotFile3 ("grafica_factorR.plt");
  plot_fr.GenerateOutput (plotFile3);
  plotFile3 << "pause -1"<<std::endl;
  plotFile3.close ();
    
  return 0;
}






void 
simulacion (uint32_t nVoip, uint32_t nHttpClient, bool tracing, double* retardo_medio, double* porcentaje_correctos, double* factor_R){
  
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
  //pointToPoint.SetQueue ("ns3::DropTailQueue"); // Configuramos cola
  p2pDevices = pointToPoint.Install (p2pNodes);
    
  /* Configuracion para la red Wifi */
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue (3.0));
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetErrorRateModel ("ns3::NistErrorRateModel");
  phy.SetChannel (channel.Create ());
  
  WifiHelper wifi = WifiHelper::Default ();

  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue("DsssRate11Mbps"), "ControlMode", StringValue("DsssRate11Mbps"));

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default();
  
  Ssid ssid = Ssid ("SSID-project");
  
  // Configuracion del AP
  
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "BeaconGeneration", BooleanValue (true), "BeaconInterval", TimeValue (Seconds (5)));  
  NetDeviceContainer apDevice;
  apDevice.Add(wifi.Install (phy, mac, wifiApNode));
  
  MobilityHelper mobilityAp;
  Ptr<ListPositionAllocator> positionAllocAp = CreateObject<ListPositionAllocator> ();
  
  positionAllocAp->Add (Vector (0.0, 0.0, 0.0));
  mobilityAp.SetPositionAllocator (positionAllocAp);
  mobilityAp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  mobilityAp.Install (wifiApNode);
  
  // Configuracion de los STA
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices;
  staDevices.Add(wifi.Install (phy, mac, wifiStaNodes));
  
  MobilityHelper mobilityStaHttp;
  Ptr<UniformRandomVariable> xhttp = CreateObject<UniformRandomVariable> ();
  xhttp->SetAttribute ("Min", DoubleValue (-4));
  xhttp->SetAttribute ("Max", DoubleValue (4));
  
  Ptr<UniformRandomVariable> yhttp = CreateObject<UniformRandomVariable> ();
  yhttp->SetAttribute ("Min", DoubleValue (5));
  yhttp->SetAttribute ("Max", DoubleValue (13));
  
  mobilityStaHttp.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                        "X", PointerValue (xhttp),
                                        "Y", PointerValue (yhttp));
  mobilityStaHttp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityStaHttp.Install (HttpClientNodes);
  
  MobilityHelper mobilityStaVoip;
  Ptr<UniformRandomVariable> xvoip = CreateObject<UniformRandomVariable> ();
  xhttp->SetAttribute ("Min", DoubleValue (-4));
  xhttp->SetAttribute ("Max", DoubleValue (4));
  
  Ptr<UniformRandomVariable> yvoip = CreateObject<UniformRandomVariable> ();
  yhttp->SetAttribute ("Min", DoubleValue (5));
  yhttp->SetAttribute ("Max", DoubleValue (7));
  
  mobilityStaVoip.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                        "X", PointerValue (xvoip),
                                        "Y", PointerValue (yvoip));
  mobilityStaVoip.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityStaVoip.Install (VoipNodes);
  
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
  Ipv4InterfaceContainer apNodeInterfaces;
  Ipv4InterfaceContainer staNodeInterfaces;
  address.SetBase ("10.1.2.0", "255.255.255.0");
  apNodeInterfaces = address.Assign (apDevice);
  staNodeInterfaces = address.Assign (staDevices);
  
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
  
  G711GeneratorHelper VoIP ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), PORTVOIP));
  VoIP.SetAttribute ("Remote",AddressValue(InetSocketAddress (p2pInterfaces.GetAddress (0), PORTVOIP)));
  ApplicationContainer VoIPClientApp = VoIP.Install (VoipNodes);
  
  VoIPClientApp.Start (Seconds (1.0));
  VoIPClientApp.Stop (Seconds (T_SIMULACION));
  
  // Servidor a cliente
  ApplicationContainer VoIPServerApp;
  for(uint32_t i = 0 ; i < nVoip ; i++){
    VoIP.SetAttribute ("Remote",AddressValue(InetSocketAddress (staNodeInterfaces.GetAddress (i), PORTVOIP)));
    VoIPServerApp.Add(VoIP.Install (p2pNodes.Get (0)));
  }
  VoIPServerApp.Start (Seconds (1.0));
  VoIPServerApp.Stop (Seconds (T_SIMULACION));
  
  sinkapp.Add(sink.Install (VoipNodes));
  
  /** Instalacion de aplicacion HttpGenerator (cliente y servidor) **/
  
  // Cliente Http
  HttpGeneratorClientHelper httpClient ("ns3::TcpSocketFactory", InetSocketAddress (p2pInterfaces.GetAddress (0), PORTHTTP));        
  ApplicationContainer httpClientApp = httpClient.Install (HttpClientNodes);
  
  httpClientApp.Start (Seconds(1.0));
  httpClientApp.Stop  (Seconds(T_SIMULACION)); 
  
  // Servidor Http
  HttpGeneratorServerHelper httpServer ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), PORTHTTP));
  ApplicationContainer httpServerApp = httpServer.Install (p2pNodes.Get (0));
  
  httpServerApp.Start (Seconds(0.0));
  httpServerApp.Stop  (Seconds(T_SIMULACION));
  
  if (tracing) {
    pointToPoint.EnablePcap ("project", p2pDevices.Get (1));
    phy.EnablePcap ("project-wifi",staDevices);
  }  
  
  
  /********************************
   * Configuracion del observador *
   ********************************/
  
  Observador m_observador;
  
  for(uint32_t i = 0; i< nVoip; i++ ){
    VoipNodes.Get (i) -> GetApplication(0) -> TraceConnectWithoutContext
      ("Tx", MakeCallback(&Observador::Envio, &m_observador));	
  }
  
  p2pNodes.Get (0) -> GetApplication(0) -> TraceConnectWithoutContext
    ("Rx", MakeCallback(&Observador::Recepcion,  &m_observador));
  
  
  
  /**********************
   * Empieza simulacion *
   **********************/
  
  Simulator::Stop (Seconds (T_SIMULACION + 1));
  Simulator::Run ();
  Simulator::Destroy ();
  
  *retardo_medio = m_observador.getMediaTiempo();
  NS_LOG_INFO ("Tiempo medio entre paquetes: "  << m_observador.getMediaTiempo());

  *porcentaje_correctos = m_observador.getTasaCorrectos();
  NS_LOG_INFO ("Porcentaje de correctos " << m_observador.getTasaCorrectos());  

  *factor_R = m_observador.getRFactor();
  NS_LOG_INFO ("Factor R: "  << m_observador.getRFactor());
    
}
