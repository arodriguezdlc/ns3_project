#include <ns3/core-module.h>
#include "Observador.h"
#include <ns3/ppp-header.h>
 
NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador ()
{
  enviados  = 0;
  recibidos = 0;
}



/*********************** Funciones para media de tiempo ***********************/
void 
Observador::Envio(Ptr <const Packet> paquete){
  lista_paquetes[paquete -> GetUid()] = Simulator::Now();
  NS_LOG_INFO ("Paquete " << paquete -> GetUid()  << " enviado.");
  enviados++;
}


void 
Observador::Recepcion(Ptr <const Packet> paquete,  const Address &dir){
  
  search = lista_paquetes.find(paquete -> GetUid());
  
  if(search != lista_paquetes.end()) {
    //Si se encuentra el paquete (debería ser siempre)
    recibidos++;
    //Actualizar el acumulador
    acumulaTiempo.Update(Simulator::Now().GetMicroSeconds() - 
                        search->second.GetMicroSeconds());
    
    //Y borrar el paquete del mapa
    lista_paquetes.erase (search); 
  
    NS_LOG_INFO ("Recibido paquete: " << paquete -> GetUid() << ", retardo: " << 
        Simulator::Now().GetMicroSeconds() - search->second.GetMicroSeconds() );

  }else{
    NS_LOG_ERROR ("Recibido paquete inesperado, uid: " << paquete -> GetUid());
  }
  
}
  
  
 
/*******************  A partir de aquí empiezan los get ***********************/
 

double
Observador::getMediaTiempo(){
  return acumulaTiempo.Mean();
}
 
 
double
Observador::getTasaCorrectos(){
  return ((double)recibidos/enviados);
}

uint32_t
Observador::getPaquetesPerdidos(){
  return lista_paquetes.size();
}
 