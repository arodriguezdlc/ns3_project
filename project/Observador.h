#include <ns3/packet.h>
#include <ns3/average.h>
#include <ns3/address.h>

using namespace ns3;


class Observador
{
public:

  Observador  ();

  void Envio(Ptr <const Packet> paquete);
  void Recepcion(Ptr <const Packet> paquete, const Address &dir);

  double   	getMediaTiempo();
  double   	getTasaCorrectos();  
  uint32_t 	getPaquetesPerdidos();
  double 	getRFactor();
 

private:
   
  uint64_t    enviados;
  uint64_t    recibidos;
  double      rFactor; 
  

  std::map<uint64_t, Time>            lista_paquetes;
  std::map<uint64_t, Time>::iterator  search;
 
  Average<double> acumulaTiempo;
  
  uint32_t prevtime;
  
};
