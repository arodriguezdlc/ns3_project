/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include "Observador.h"

NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de observador
Observador::Observador ()
{
 
}

void
Observador::PaqueteEnviado (Ptr<const Packet> paquete) {
  NS_LOG_FUNCTION(this);
}
