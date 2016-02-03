
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
#include "ns3/udp-socket-factory.h"
#include "G711Generator.h"

NS_LOG_COMPONENT_DEFINE ("G711");

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (G711Generator);


//Método para adecuarse al standard de API's de helpers
TypeId
G711Generator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::G711Generator")
    .SetParent<Application> ()
    .AddConstructor<G711Generator> ()
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&G711Generator::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&G711Generator::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("PktSize",
                   "Size of datagrams send",
                   UintegerValue (172),
                   MakeUintegerAccessor (&G711Generator::m_pktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("PktRate",
                   "Packet sending rate",
                   DoubleValue (50),
                   MakeDoubleAccessor (&G711Generator::m_pktRate),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                   MakeTraceSourceAccessor (&G711Generator::m_txTrace))
  ;
  return tid;
}

G711Generator::G711Generator ()
    : m_socket(0)
{

    NS_LOG_FUNCTION(this);
    //m_pktPeriod = 0.02; //Por defecto son 50 pps
    //m_pktSize = 172; //Payload+RTP
    m_numPkts = 0; 
    NS_LOG_WARN("ESTA VERSION DE G711 NO SE PUEDE ENTREGAR. Falta revisar: \n - Comentarios \n - Longitud de lineas y tabulación \n - m_numPkts y demás debugueo." );
}

G711Generator::~G711Generator ()
{
    NS_LOG_FUNCTION (this);
}

Ptr<Socket>
G711Generator::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
G711Generator::DoDispose (void)
{

  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

void G711Generator::StartApplication (void) 
{
    NS_LOG_FUNCTION(this);

    m_pktPeriod = 1/m_pktRate;
   
    m_socket = Socket::CreateSocket (GetNode (), m_tid);
    int err = m_socket->Bind ();    
    err += m_socket->Connect (m_peer);
    err += m_socket->ShutdownRecv ();
        
    if(err != 0)
        NS_LOG_ERROR("Fallo de configuración en SetRemote de G711");
 
    // NS_LOG_INFO("Un nuevo generador de G711 comienza a transmitir");
    DoGenerate();
}

void G711Generator::StopApplication (void){ 
    // NS_LOG_INFO("Se detiene un generador de G711");
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Paquetes enviados: " << m_numPkts);       
    Simulator::Cancel(m_next);

}

/* Eliminado, se configura en startaplication gracias al helper
void 
G711Generator::SetRemote(std::string socketType, 
                        Address remote,  uint16_t port)
{
    r_port = port;
    m_peer = remote;
    m_socketType = socketType;
    
}
*/

void
G711Generator::DoGenerate (void)
{
    m_next = Simulator::Schedule (Seconds (m_pktPeriod), 
                &G711Generator::DoGenerate, this);
                
    Ptr<Packet> p = Create<Packet> (m_pktSize);
  
    m_txTrace (p);
    int bytes = m_socket->Send (p) ;
    if(bytes >= 0)
        NS_LOG_LOGIC("Bytes enviados: " << bytes);
    else
        m_numPkts++;
}
