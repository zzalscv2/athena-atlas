/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
   */

// $Id: getIsolationCorrectionAccessor.cxx 625328 2014-10-31 09:39:45Z laplace $

// Local include(s):
#include "xAODPrimitives/tools/getIsolationCorrectionAccessor.h"

namespace xAOD {

const SG::AuxElement::Accessor< uint32_t >
  getIsolationCorrectionBitsetAccessor( Iso::IsolationFlavour type ){
    std::string name(Iso::toCString(type));
    name+="CorrBitset";
    return SG::AuxElement::Accessor< uint32_t >( name );
  }

const SG::AuxElement::Accessor< float >
  getIsolationCorrectionAccessor( Iso::IsolationFlavour type, Iso::IsolationCaloCorrection corr, 
                                  Iso::IsolationCorrectionParameter param  ){
    std::string name(Iso::toCString(type));                                                                       
    if (corr == Iso::coreCone || corr == Iso::coreConeSC)
      name+=toCString(corr); 
    else{
        name = toCString(corr);
    }

    if (param==xAOD::Iso::coreEnergy || param==xAOD::Iso::coreArea){
      name+=toCString(param );    
    }
    name+="Correction";

    return SG::AuxElement::Accessor< float >( name );                                                                                                              
  }

// Isolation Calo 
const SG::AuxElement::Accessor< float >
  getIsolationCorrectionAccessor( Iso::IsolationType type, Iso::IsolationCaloCorrection corr){
    std::string name(Iso::toCString(type));
    name+=toCString(corr);
    name+="Correction";
    return SG::AuxElement::Accessor< float >( name );
  }

const SG::AuxElement::Accessor< float >
  getIsolationCorrectionAccessor( Iso::IsolationFlavour type, Iso::IsolationTrackCorrection corr ){
      std::string name(Iso::toCString(type));                                                                         
      name+=toCString(corr);    
      name+="Correction";
      return SG::AuxElement::Accessor< float >( name );

  }
} // namespace xAOD
