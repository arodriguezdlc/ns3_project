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
  #include "ns3/random-variable-stream.h"
  
  #include "G711Generator.h"
  #include "G711GeneratorHelper.h"
  
  #include "HttpGeneratorClientHelper.h"
  #include "HttpGeneratorClient.h"
  #include "HttpGeneratorServerHelper.h"
  #include "HttpGeneratorServer.h"
  
  #include "Observador.h"
  
  #define PORTVOIP 9
  #define PORTHTTP 10
  #define T_SIMULACION 15.0
  
  
  using namespace ns3;
  
  NS_LOG_COMPONENT_DEFINE ("simulation");
  
  
  int
  main (int argc, char *argv[])
  {
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Time::SetResolution (Time::NS);
  
  
    // Parametros de la simulacion
  
    uint32_t nVoip = 9;
    uint32_t nHttpClient = 10;
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
    //phy.Set ("ShortGuardEnabled", BooleanValue (0));
    
    WifiHelper wifi = WifiHelper::Default ();
    //wifi.EnableLogComponents ();
    wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
    //wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
    //wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  
    //HtWifiMacHelper mac = HtWifiMacHelper::Default ();
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
  
  //  if (tracing) {
      //pointToPoint.EnablePcap ("project", p2pDevices.Get (1));
      //phy.EnablePcap ("project-wifi",staDevices);
  //  }
  
    phy.EnablePcap ("project-wifi",apDevice);

  
  
    /**********************
     * Empieza simulacion *
     **********************/
    
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
    
    NS_LOG_INFO ("Voy a simular");
    Simulator::Stop (Seconds (T_SIMULACION + 1));
    Simulator::Run ();
    Simulator::Destroy ();
  
    NS_LOG_INFO ("Tiempo medio entre paquetes: "  << m_observador.getMediaTiempo());
    NS_LOG_INFO ("Porcentaje de correctos " << m_observador.getTasaCorrectos());
  
    NS_LOG_INFO ("Factor R: "  << m_observador.getRFactor());

  
    return 0;
  
  }
