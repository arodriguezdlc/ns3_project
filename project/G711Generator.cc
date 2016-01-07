#include "G711Generator.h"


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
G711Generator::SetRemote (std::string socketType, 
                            Address remote)
{
  TypeId tid = TypeId::LookupByName (socketType);
  m_socket = Socket::CreateSocket (GetNode (), tid);
  m_socket->Bind ();
  m_socket->ShutdownRecv ();
  m_socket->Connect (remote);
}



void
G711Generator::DoGenerate (void)
{
  m_next = Simulator::Schedule (Seconds (tbPkts), 
                &RandomGenerator::DoGenerate, this);
  Ptr<Packet> p = Create<Packet> (sizePkt);
  m_socket->Send (p);
}
