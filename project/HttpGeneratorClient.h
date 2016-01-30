#ifndef HTTP_GENERATOR_CLIENT_APPLICATION_H
#define HTTP_GENERATOR_CLIENT_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class Socket;

class HttpGeneratorClient : public Application
{

public:  
  static TypeId GetTypeId (void);
  HttpGeneratorClient ();
  virtual ~HttpGeneratorClient ();
  void SetMaxBytes (uint32_t maxBytes);
  Ptr<Socket> GetSocket (void) const;

protected:
  virtual void DoDispose (void);

private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  void SendData ();
  void Request ();
  void CreateNewSocket ();
  void HandleRead (Ptr<Socket> socket);


  Ptr<Socket>     m_socket;       //!< Associated socket
  Address         m_peer;         //!< Peer address
  bool            m_connected;    //!< True if connected
  uint32_t        m_sendSize;     //!< Size of data to send each time
  uint32_t        m_maxBytes;     //!< Limit total number of bytes sent
  uint32_t        m_totBytes;     //!< Total bytes sent so far
  TypeId          m_tid;          //!< The type of protocol to use.
  double          m_timeBetweenRequests;  //!< Mean time between HTTP requests (in seconds).
  bool            m_stop;

  Ptr<ExponentialRandomVariable> m_expRandom;
  /// Traced Callback: sent packets
  TracedCallback<Ptr<const Packet> > m_txTrace;

private:
  
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void DataSend (Ptr<Socket>, uint32_t); // for socket's SetSendCallback
  void ConnectionClosed (Ptr<Socket> socket);
  void ConnectionClosedWithError (Ptr<Socket> socket);
};

} // namespace ns3

#endif /* HTTP_GENERATOR_CLIENT_APPLICATION_H */
