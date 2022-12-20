// this file is -*- C++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//JetConstituentModSequence.h

#ifndef JETRECTOOLS_JETCONSTITUENTMODSEQUENCE_H
#define JETRECTOOLS_JETCONSTITUENTMODSEQUENCE_H

//
// // Michael Nelson, CERN & Univesity of Oxford
// // February, 2016

#include <string>
#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetExecuteTool.h"
#include "JetInterface/IJetConstituentModifier.h"
#include "AsgTools/ToolHandleArray.h"
#include "xAODCore/ShallowCopy.h"


class JetConstituentModSequence: public asg::AsgTool, virtual public IJetExecuteTool { // Changed from IJetExecute
  ASG_TOOL_CLASS(JetConstituentModSequence, IJetExecuteTool)
  public:
  JetConstituentModSequence(const std::string &name); // MEN: constructor 
  StatusCode initialize();
  int execute() const;
  void setInputClusterCollection(const xAOD::IParticleContainer* cont);
  xAOD::IParticleContainer* getOutputClusterCollection();
  
protected:
  std::string m_inputContainer = "";
  std::string m_outputContainer = "";
  const xAOD::IParticleContainer* m_trigInputConstits; // used in trigger context only
  mutable xAOD::IParticleContainer* m_trigOutputConstits;
  bool m_trigger;
  bool m_byVertex;
  
  // P-A : the actual type
  // Define as a basic integer type because Gaudi
  // doesn't support arbitrary property types
  unsigned short m_inputType; // 
  
  
  ToolHandleArray<IJetConstituentModifier> m_modifiers;

  bool m_saveAsShallow = true;

  /// helper function to cast, shallow copy and record a container (for offline) or deep copy and record a container (for trigger, where shallow copy isn't supported)
  template<class T, class Taux, class Tsingle>
  xAOD::IParticleContainer* copyAndRecord(const xAOD::IParticleContainer* cont, bool record, std::string suffix="", unsigned numCopies = 0) const {
    const T * constitCont = dynamic_cast<const T *>(cont);
    if(constitCont == 0) {
      ATH_MSG_ERROR( "Container "<<m_inputContainer+suffix<< " is not of type "<< m_inputType);
      return nullptr;
    }

    if(!m_trigger && numCopies < 2){
      std::pair< T*, xAOD::ShallowAuxContainer* > newconstit = xAOD::shallowCopyContainer(*constitCont );    
      newconstit.second->setShallowIO(m_saveAsShallow);
      if(record){
        if(evtStore()->record( newconstit.first, m_outputContainer+suffix ).isFailure() ||
	   evtStore()->record( newconstit.second, m_outputContainer+suffix+"Aux." ).isFailure() ){
          ATH_MSG_ERROR("Unable to record object collection " << m_outputContainer+suffix );
          return nullptr;
        }
      }
      //newconstit.second->setShallowIO(false);
      return newconstit.first;
    }
    
    
    // This is the trigger case, revert to a deep copy
    
    // Create the new container and its auxiliary store.
    T* constitCopy = new T();
    Taux* constitCopyAux = new Taux();
    constitCopy->setStore( constitCopyAux ); //< Connect the two
    typename T::const_iterator constit_itr;
    constit_itr = constitCont->begin();
    typename T::const_iterator constit_end = constitCont->end();

    for( ; constit_itr != constit_end; ++constit_itr ) {
      Tsingle* constit = new Tsingle();
      constitCopy->push_back( constit ); // jet acquires the goodJets auxstore
      *constit= **constit_itr; // copies auxdata from one auxstore to the other
      //constitCopy->push_back(new Tsingle(**constit_itr));
    }

    // If the user has requested that we make copies of the constituents, do so here
    if (numCopies > 1) {
      // Define the accessor which will add the index
      const SG::AuxElement::Accessor<unsigned> copyIndex("ConstituentCopyIndex");

      // Add an index of 0 to the existing constituents
      for ( typename T::iterator mod_itr = constitCopy->begin(); mod_itr != constitCopy->end(); ++mod_itr)
        copyIndex(**mod_itr) = 0;

      // Now add the duplicates of the constituents
      for (size_t iCopy {1} ; iCopy < numCopies; ++iCopy) {
        for ( constit_itr = constitCont->begin(); constit_itr != constit_end; ++constit_itr) {
          Tsingle* constit = new Tsingle();
          constitCopy->push_back(constit); // jet acquires the goodJets auxstore
          *constit = **constit_itr; // copies auxdata from one auxstore to the other
          copyIndex(*constitCopy->back()) = iCopy; // add the index of this copy
        }
      }

      // In this case, we do want to record it to evtStore, just as a deep copy
      if (record) {
        if (evtStore()->record(constitCopy, m_outputContainer+suffix).isFailure() ||
            evtStore()->record(constitCopyAux, m_outputContainer+suffix+"Aux.").isFailure() ) {
          ATH_MSG_ERROR("Unable to record object collection with " << numCopies << " copies, " << m_outputContainer+suffix);
          return nullptr;
        }
      }
      // Don't actually fill the trigger output container, return here
      return constitCopy;
    }

    // Store and return the container
    m_trigOutputConstits = constitCopy;
    return m_trigOutputConstits;

  }

};

#endif
