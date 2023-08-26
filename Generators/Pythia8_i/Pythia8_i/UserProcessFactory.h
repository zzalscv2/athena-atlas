/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOR_PYTHIA8_USER_PROCESS_FACTORY_H
#define GENERATOR_PYTHIA8_USER_PROCESS_FACTORY_H

#include "Pythia8/SigmaProcess.h"


#include <map>
#include <string>

namespace Pythia8_UserProcess{
  
  using Pythia8::Sigma2Process;
    
  
  class UserProcessFactory{
    
  public:
    
    static std::shared_ptr<Sigma2Process> create(const std::string &procName);
    
  private:
    
    UserProcessFactory(){};
    
    class ICreator{
    public:
      virtual std::shared_ptr<Sigma2Process> create() const = 0;
      virtual ~ICreator(){};
    };
    
  public:
    
    template <class T>
    class Creator: public ICreator{
      
    public:
      Creator(const std::string &name){
        m_name = name;
        UserProcessFactory::s_creators()[name] = this;
      }
      
      ~Creator(){
        if(s_creators()[m_name] == this){
          s_creators().erase(m_name);
        }
      }
      
      std::shared_ptr<Sigma2Process> create()const{
        return std::make_shared<T>();
      }
      
    private:
      
      std::string m_name;
      
    };
    
  private:
    static std::map<std::string, const ICreator*> &s_creators();
    
  };
}
#endif
