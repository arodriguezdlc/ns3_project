using namespace ns3;

#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/application.h"
#include "ns3/socket.h" 




class G711Generator : public Application
{
public:

  static ns3::TypeId GetTypeId (void);
  G711Generator ();
  void SetRemote(std::string socketType, Address remote,  uint16_t port);
  void SetRate (double rate);
  void SetSize (uint32_t size);

 

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  void DoGenerate (void);
  
  TracedCallback <Ptr<const Packet> > m_txTrace;

  double        tbPkts;
  uint32_t      sizePkt;
  Ptr<Socket>   m_socket;
  EventId       m_next;
  
  uint32_t      num_pkts;
  
  TypeId        m_tid;
  
  uint16_t      r_port;
  Address       m_peer;
  std::string   m_socketType;
  
  
};

