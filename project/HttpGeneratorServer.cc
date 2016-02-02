/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "HttpGeneratorServer.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HttpGeneratorServer");
NS_OBJECT_ENSURE_REGISTERED (HttpGeneratorServer);

TypeId 
HttpGeneratorServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HttpGeneratorServer")
    .SetParent<Application> ()
    .AddConstructor<HttpGeneratorServer> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&HttpGeneratorServer::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpGeneratorServer::m_tid),
                   MakeTypeIdChecker ())    
    .AddAttribute ("SendSize", "The amount of data to send each time.",
                   UintegerValue (1400),
                   MakeUintegerAccessor (&HttpGeneratorServer::m_sendSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("MaxBytes",
                   "The total number of bytes to send. "
                   "Once these bytes are sent, "
                   "no data  is sent again. The value zero means "
                   "that there is no limit.",
                   UintegerValue (2262000),
                   MakeUintegerAccessor (&HttpGeneratorServer::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&HttpGeneratorServer::m_rxTrace))
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                   MakeTraceSourceAccessor (&HttpGeneratorServer::m_txTrace))
  ;
  return tid;
}

HttpGeneratorServer::HttpGeneratorServer ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;
}

HttpGeneratorServer::~HttpGeneratorServer()
{
  NS_LOG_FUNCTION (this);
}

uint32_t HttpGeneratorServer::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

Ptr<Socket>
HttpGeneratorServer::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
HttpGeneratorServer::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void HttpGeneratorServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}


// Application Methods
void HttpGeneratorServer::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
      //m_socket->ShutdownSend ();      
    }

  m_socket->SetRecvCallback (MakeCallback (&HttpGeneratorServer::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeCallback (&HttpGeneratorServer::HandleConnectionRequest, this),
    MakeCallback (&HttpGeneratorServer::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&HttpGeneratorServer::HandlePeerClose, this),
    MakeCallback (&HttpGeneratorServer::HandlePeerError, this));
}

void HttpGeneratorServer::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  m_totBytes.clear();
  if (m_socket) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void HttpGeneratorServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }
         
      m_totalRx += packet->GetSize ();
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s httpGeneratorServer received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }      
      m_rxTrace (packet, from);
    }
}



void HttpGeneratorServer::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void HttpGeneratorServer::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void HttpGeneratorServer::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  NS_LOG_DEBUG("HTTPSERVER: RECEIVED HTTP REQUEST, RESPONDING");
  s->SetRecvCallback (MakeCallback (&HttpGeneratorServer::HandleRead, this));
  //Adding socket to socket list
  m_socketList.push_back (s);
   //Adding node id of socket
  m_totBytes[s] = 0;
  s->SetSendCallback (
    MakeCallback (&HttpGeneratorServer::DataSend, this));
  //m_totBytes = 0;
  SendData(s);

  //SendData(s);
}

bool HttpGeneratorServer::HandleConnectionRequest (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  return true;
}

void HttpGeneratorServer::DataSend (Ptr<Socket> s, uint32_t)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleNow (&HttpGeneratorServer::SendData, this, s);  
}

void HttpGeneratorServer::SendData (Ptr <Socket> s)
{
  NS_LOG_FUNCTION (this);



  std::map<Ptr<Socket>, uint32_t>::iterator it;
  it=m_totBytes.find (s);
  if (it != m_totBytes.end()) {
    uint32_t totBytes = it->second;

    while (m_maxBytes == 0 || totBytes < m_maxBytes)
      { // Time to send more
        uint32_t toSend = m_sendSize;
        // Make sure we don't send too many
        if (m_maxBytes > 0)
          {
            toSend = std::min (m_sendSize, m_maxBytes - totBytes);
          }

        Ptr<Packet> packet;
        if (m_maxBytes - totBytes <= m_sendSize) { //If it's last packet
        
          NS_LOG_INFO("HttpServer: Sending last packet of response at " << Simulator::Now ());        
         
        } else {
        
          NS_LOG_INFO("HttpServer: Sending a new packet at " << Simulator::Now ());
          
        }

        packet = Create<Packet> (toSend);      
        
        m_txTrace (packet);
        int actual = s->Send (packet);
        if (actual > 0)
          {
            totBytes += actual;
          }
        // We exit this loop when actual < toSend as the send side
        // buffer is full. The "DataSent" callback will pop when
        // some buffer space has freed ip.
        if ((unsigned)actual != toSend)
          {
            break;
          }
      }
    //m_totBytes.erase (it);
    m_totBytes[s] = totBytes; //Update map of totBytes
    // Check if time to close (all sent)
    if (totBytes == m_maxBytes)
      {
        //m_totBytes.erase (it); //Delete totBytes for this socket
        s->Close ();      
      }
  } else {  //If iterator 
    NS_LOG_ERROR ("ERROR, ITERATOR NOT FOUND FOR SOCKET " << s);
  }
}


} // Namespace ns3
