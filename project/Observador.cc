#include <ns3/core-module.h>
#include "Observador.h"
#include <ns3/ppp-header.h>
#include <math.h>       /* log */

NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador ()
{
  enviados  = 0;
  recibidos = 0;
  prevtime  = 0;
}


//Callback que se lanza al enviarse un paquete de las aplicaciones G711
void 
Observador::Envio(Ptr <const Packet> paquete){

  //Metemos el paquete en una lista
  lista_paquetes[paquete -> GetUid()] = Simulator::Now();
  enviados++;

  NS_LOG_INFO ("Paquete " << paquete -> GetUid()  << " enviado.");

}

//Callback que se lanza al recibirse paquetes
void 
Observador::Recepcion(Ptr <const Packet> paquete,  const Address &dir){
  
  search = lista_paquetes.find(paquete -> GetUid());
  //Si el paquete recibido está en la lista
  if(search != lista_paquetes.end()) {
    //aumentamos el número de paquetes recibidos correctamente
    recibidos++;
    
    //Actualizar el acumulador de tiempos de recepcion
    acumulaTiempo.Update(Simulator::Now().GetMicroSeconds() - 
                        search->second.GetMicroSeconds());
    
    //Y borrar el paquete del mapa
    lista_paquetes.erase (search); 
  
    NS_LOG_INFO ("Recibido paquete: " << paquete -> GetUid() << ", retardo: " << 
        Simulator::Now().GetMicroSeconds() - search->second.GetMicroSeconds() );

    if(Simulator::Now().GetSeconds() > prevtime){
      prevtime++;
      NS_LOG_ERROR ("Llevamos simulados: " << prevtime);
    }

 

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
double
Observador::getRFactor(){
  //Facor R según el anexo 2 de voip de la memoria http://www.nit.eu/czasopisma/JTIT/2002/2/53.pdf
  //R = 92.68 − Id − Ie
  //Id = 0.65 + (0.1*Ta - 15.93) * delta(Ta-165) ;
  //Ta = Tenc + Tp + Tdec + Tn ; 
  //Ie = 22*ln (1 + 0,2 * p.l.)
  
  double Ta = 20 + 0.25 + acumulaTiempo.Mean()/1000;
  //20ms para formar el paquete, 0.25 del codec segun anexio 1
  double Id = 0.65 + (0.1*Ta - 15.93);
  if(Ta<165)
    Id = 0;
  
  double Ie = 22*log((1+ 0.2*100*((double)getPaquetesPerdidos()/enviados)));
  rFactor = 92.68 - Id - Ie;
  
  return rFactor;
}