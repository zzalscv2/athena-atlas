/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATOR_PYTHIA8_USER_RESONANCE_FACTORY_H
#define GENERATOR_PYTHIA8_USER_RESONANCE_FACTORY_H

#include "Pythia8/ResonanceWidths.h"
#include "Pythia8/Pythia.h"

#include <string>
#include <memory>

namespace Pythia8_UserResonance{
 
  using Pythia8::ResonanceWidths;
  
  class UserResonanceFactory{
    
  public:
    
    /**
     *  Call this with the name of the ResonanceWidth and PDG ID to which it will be applied
     *  e.g. create("MyResonance", 23) will return a MyResonance instance that will be applied to the Z
     *
     */
    static std::shared_ptr<ResonanceWidths> create(const std::string &name, int pdgid);
    
  private:
    
    UserResonanceFactory(){};
    
    class ICreator{
    public:
      virtual std::shared_ptr<ResonanceWidths> create(int idResIn)const = 0;
      virtual ~ICreator(){};
    };
    
  public:
    
    template <class T>
    class Creator: public ICreator{
      
    public:
      Creator(const std::string &name){
        m_name = name;
        UserResonanceFactory::s_creators()[name] = this;
      }
      
      ~Creator(){
        if(s_creators()[m_name] == this){
          s_creators().erase(m_name);
        }
      }
      
      std::shared_ptr<ResonanceWidths> create(int idResIn)const{
        return std::make_shared<T>(idResIn);
      }
      
    private:
      
      std::string m_name;
    };
    
  private:
    
    static std::map<std::string, const ICreator*> &s_creators();
    
  };
}

#endif
