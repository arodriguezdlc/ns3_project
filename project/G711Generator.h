class G711Generator : public Application
{
public:
  G711Generator ();
  void SetRate (double rate);
  void SetSize (uint32_t size);
  void SetRemote (std::string socketType, 
                  Address remote);


private:
  
  virtual void StartApplication (void){
    NS_LOG_INFO("Un nuevo generador de G711 comienza a transmitir");
    DoGenerate();
  
  }

  virtual void StopApplication (void){ //Hay que comprobar que se deje de transmitir, quizá haya que cancelar el próximo evento del Dogenerate
    NS_LOG_INFO("Se detiene un generador de G711");
    
  }
  
  void DoGenerate (void);
  
  double    tbPkts = 0.02; //Por defecto son 50 pps
  uint32_t  sizePkt = 172; //Payload+RTP
  Ptr<Socket> m_socket;
};