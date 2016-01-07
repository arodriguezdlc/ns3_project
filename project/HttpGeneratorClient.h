class HttpGeneratorClient : public Application
{
  public:

  private:
    virtual void StartApplication (void){
        Simulator::Schedule (Seconds (m_delay.GetValue ()), 
                    &RandomGenerator::DoGenerate, this);
        Ptr<Packet> p = Create<Packet> (m_size.GetIntValue ());
    
        m_socket->Send (p);
    }
    

    virtual void StopApplication (void) {

    }

    void DoGenerate (void) {

    }

};