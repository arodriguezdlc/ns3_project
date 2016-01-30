/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef HTTP_GENERATOR_SERVER_H
#define HTTP_GENERATOR_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"

namespace ns3 {

class Address;
class Socket;
class Packet;


class HttpGeneratorServer : public Application 
{
public:
  
  static TypeId GetTypeId (void);
  HttpGeneratorServer ();

  virtual ~HttpGeneratorServer ();

  
  uint32_t GetTotalRx () const;

  
  Ptr<Socket> GetListeningSocket (void) const;

  std::list<Ptr<Socket> > GetAcceptedSockets (void) const;
 
protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /**
   * \brief Handle a packet received by the application
   * \param socket the receiving socket
   */
  void HandleRead (Ptr<Socket> socket);
  /**
   * \brief Handle an incoming connection
   * \param socket the incoming connection socket
   * \param from the address the connection is from
   */
  void HandleAccept (Ptr<Socket> socket, const Address& from);
  /**
   * \brief Handle an connection close
   * \param socket the connected socket
   */
  bool HandleConnectionRequest (Ptr<Socket> socket, const Address& from);
  /**
   * \brief Handle a new connection request from HttpGeneratorClient
   * \param socket the connected socket
   * \return return true if connection is accepted and false if not. 
   */
  void HandlePeerClose (Ptr<Socket> socket);
  /**
   * \brief Handle an connection error
   * \param socket the connected socket
   */
  void HandlePeerError (Ptr<Socket> socket);

  // In the case of TCP, each socket accept returns a new socket, so the 
  // listening socket is stored separately from the accepted sockets
  Ptr<Socket>     m_socket;       //!< Listening socket
  std::list<Ptr<Socket> > m_socketList; //!< the accepted sockets

  Address         m_local;        //!< Local address to bind to
  uint32_t        m_totalRx;      //!< Total bytes received
  TypeId          m_tid;          //!< Protocol TypeId

  uint32_t        m_sendSize;     //!< Size of data to send each time
  uint32_t        m_maxBytes;     //!< Limit total number of bytes sent
  uint32_t        m_totBytes;     //!< Total bytes sent so far

  /// Traced Callback: received packets, source address.
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

};

} // namespace ns3

#endif /* HTTP_GENERATOR_SERVER_H */

