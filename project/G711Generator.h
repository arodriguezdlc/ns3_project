/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

using namespace ns3;

#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/application.h"
#include "ns3/socket.h" 


class G711Generator : public Application
{
public:
  G711Generator (std::string socketType, 
                  Address remote);
  
  void SetRate (double rate);
  void SetSize (uint32_t size);


private:
  virtual void StartApplication (void){
    // NS_LOG_INFO("Un nuevo generador de G711 comienza a transmitir");
    DoGenerate();
  }

  virtual void StopApplication (void){ //Hay que comprobar que se deje de transmitir, quizá haya que cancelar el próximo evento del Dogenerate
    // NS_LOG_INFO("Se detiene un generador de G711");    
  }
  
  void DoGenerate (void);
  

  double    	tbPkts;
  uint32_t  	sizePkt;
  Ptr<Socket> 	m_socket;
  EventId     	m_next;

};