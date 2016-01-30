#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/random-variable-stream.h"
#include "HttpGeneratorClient.h"

NS_LOG_COMPONENT_DEFINE ("HttpGeneratorClient");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpGeneratorClient);

TypeId
HttpGeneratorClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HttpGeneratorClient")
    .SetParent<Application> ()
    .AddConstructor<HttpGeneratorClient> ()
    .AddAttribute ("SendSize", "The amount of data to send each time.",
                   UintegerValue (512),
                   MakeUintegerAccessor (&HttpGeneratorClient::m_sendSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&HttpGeneratorClient::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. "
                   "Once these bytes are sent, "
                   "no data  is sent again. The value zero means "
                   "that there is no limit.",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&HttpGeneratorClient::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpGeneratorClient::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("TimeBetweenRequests", "Mean time between HTTP requests (in seconds).",
                   DoubleValue (5),
                   MakeDoubleAccessor (&HttpGeneratorClient::m_timeBetweenRequests),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                   MakeTraceSourceAccessor (&HttpGeneratorClient::m_txTrace))
  ;
  return tid;
}


HttpGeneratorClient::HttpGeneratorClient ()
  : m_socket (0),
    m_connected (false),
    m_totBytes (0)
{
  NS_LOG_FUNCTION (this);
  m_expRandom = CreateObject<ExponentialRandomVariable> ();
}

HttpGeneratorClient::~HttpGeneratorClient ()
{
  NS_LOG_FUNCTION (this);
}

void
HttpGeneratorClient::SetMaxBytes (uint32_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}

Ptr<Socket>
HttpGeneratorClient::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
HttpGeneratorClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void HttpGeneratorClient::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Set request rate attribute
  
  m_expRandom->SetAttribute ("Mean", DoubleValue (m_timeBetweenRequests));  
  
  /*
  if (m_connected)
    {
      SendData ();
    }
  */

  //Programing first request
  Simulator::Schedule (Simulator::Now (), &HttpGeneratorClient::Request, this);
}

void HttpGeneratorClient::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  if (m_socket != 0)
    {
      m_socket->Close ();
      m_connected = false;
    }
  else
    {
      NS_LOG_WARN ("HttpGeneratorClient found null socket to close in StopApplication");
    }
}




// Private helpers

void HttpGeneratorClient::Request () 
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("HTTPCLIENT: SENDING A NEW HTTP REQUEST (Time: " << Simulator::Now() << ")" );
  CreateNewSocket ();
  m_socket->Connect (m_peer);
}

void HttpGeneratorClient::CreateNewSocket () {
  
      m_socket = Socket::CreateSocket (GetNode (), m_tid);

      // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
      if (m_socket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
          m_socket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
        {
          NS_FATAL_ERROR ("Using HttpGeneratorClient with an incompatible socket type. "
                          "HttpGeneratorClient requires SOCK_STREAM or SOCK_SEQPACKET. "
                          "In other words, use TCP instead of UDP.");
        }

      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind6 ();
        }
      else if (InetSocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind ();
        }

      //m_socket->Connect (m_peer);
      //m_socket->ShutdownRecv ();
      m_socket->SetConnectCallback (
        MakeCallback (&HttpGeneratorClient::ConnectionSucceeded, this),
        MakeCallback (&HttpGeneratorClient::ConnectionFailed, this));
      m_socket->SetSendCallback (
        MakeCallback (&HttpGeneratorClient::DataSend, this));
    
}

/* SEND DATA UNTIL MAX BYTES */
void HttpGeneratorClient::SendData (void)
{
  NS_LOG_FUNCTION (this);

  while (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    { // Time to send more
      uint32_t toSend = m_sendSize;
      // Make sure we don't send too many
      if (m_maxBytes > 0)
        {
          toSend = std::min (m_sendSize, m_maxBytes - m_totBytes);
        }

      Ptr<Packet> packet;
      if (m_maxBytes - m_totBytes <= m_sendSize) { //If it's last packet
      
        NS_LOG_INFO("HttpClient: Sending last packet of request at " << Simulator::Now ());        
       
      } else {
      
        NS_LOG_INFO("HttpClient: Sending a new packet at " << Simulator::Now ());
        
      }

      packet = Create<Packet> (toSend);      
      
      m_txTrace (packet);
      int actual = m_socket->Send (packet);
      if (actual > 0)
        {
          m_totBytes += actual;
        }
      // We exit this loop when actual < toSend as the send side
      // buffer is full. The "DataSent" callback will pop when
      // some buffer space has freed ip.
      if ((unsigned)actual != toSend)
        {
          break;
        }
    }
  // Check if time to close (all sent)
  if (m_totBytes == m_maxBytes && m_connected)
    {
      m_socket->Close ();
      m_connected = false;
      m_totBytes = 0;
      //Schedule next request 
      Simulator::Schedule (Seconds(m_expRandom->GetValue()), &HttpGeneratorClient::Request, this);
    }
}

void HttpGeneratorClient::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("HttpGeneratorClient Connection succeeded");
  m_connected = true;
  SendData ();
}

void HttpGeneratorClient::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("HttpGeneratorClient, Connection Failed");
}

void HttpGeneratorClient::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);
  if (m_connected)
    { // Only send new data if the connection has completed
      Simulator::ScheduleNow (&HttpGeneratorClient::SendData, this);
    }
}



} // Namespace ns3

