#ifndef G711_GENERATOR_H
#define G711_GENERATOR_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"


namespace ns3 {

class Address;
class Socket;

class G711Generator : public Application
{
public:

  static TypeId GetTypeId (void);
  G711Generator ();

  virtual ~G711Generator ();
  Ptr<Socket> GetSocket (void) const;


  //void SetRemote(std::string socketType, Address remote,  uint16_t port);
 
protected:
  virtual void DoDispose (void);
 

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  void DoGenerate (void);
  
  TracedCallback <Ptr<const Packet> > m_txTrace;

  double        m_pktPeriod;
  double        m_pktRate;

  uint32_t      m_pktSize;
  Ptr<Socket>   m_socket;
  EventId       m_next;
  
  uint32_t      m_numPkts;
  
  TypeId        m_tid;
  

  uint16_t      r_port;
  Address       m_peer;
  std::string   m_socketType;
    
};

} //namespace ns3

#endif /* G711_GENERATOR_H */
