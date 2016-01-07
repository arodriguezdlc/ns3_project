#include "G711Generator.h"


void 
G711Generator::SetRate (Datarate datarate)
{
	rate = datarate;
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
  m_next = Simulator::Schedule (Seconds (m_delay.GetValue ()), 
                &RandomGenerator::DoGenerate, this);
  Ptr<Packet> p = Create<Packet> (m_size.GetIntValue ());
  m_socket->Send (p);
}
