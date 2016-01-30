/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "G711Generator.h"
#include "ns3/command-line.h"

using namespace ns3;



NS_LOG_COMPONENT_DEFINE ("G711");


G711Generator::G711Generator ()
{
	tbPkts = 0.02; //Por defecto son 50 pps
	sizePkt = 172; //Payload+RTP
		std::cout << "Instanciado" << std::endl;

}


void 
G711Generator::SetRemote(std::string socketType, 
						Address remote)
{
 	TypeId tid = TypeId::LookupByName (socketType);
 	m_socket = Socket::CreateSocket (GetNode (), tid);
 	m_socket->Bind ();
 	m_socket->ShutdownRecv ();
 	m_socket->Connect (remote);
}


void 
G711Generator::SetRate (double rate)
{
	tbPkts = 1/rate;
}

void 
G711Generator::SetSize (uint32_t size)
{
	sizePkt = size;
}



void
G711Generator::DoGenerate (void)
{
  m_next = Simulator::Schedule (Seconds (tbPkts), 
                &G711Generator::DoGenerate, this);
                
  Ptr<Packet> p = Create<Packet> (sizePkt);
  
  m_socket->Send (p);
}
