/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/average.h>
#include <ns3/nstime.h>
#include <ns3/address.h>

using namespace ns3;

class Observador {

public:
  Observador ();
  void PaqueteEnviado (Ptr<const Packet> paquete);
  
private:  
  
};
