// this file is -*- C++ -*-
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETREC_JETCLUSTERERBYVERTEX_H
#define JETREC_JETCLUSTERERBYVERTEX_H
///***********************************************
///
/// \class JetClustererByVertex
///
/// Creates a new JetContainer by running fastjet on an input PseudoJetVector
/// The container contains jets reconstructed with respect to a collection of vertices
///
/// This tool implements the IJetProvider interface. The JetContainer it returns is build by
/// running a fastjet::ClusterSequence according to the tool's Properties (alg type, radius, ...)
/// The ClusterSequence is run on an input PseudoJetVector which must be present in the event store (this PseudoJetVector key is also a Property of the tool).
///


#include "xAODEventInfo/EventInfo.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"

#include "JetInterface/IJetProvider.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

#include "JetRec/PseudoJetContainer.h"
#include "JetRec/JetFromPseudojet.h"
#include "JetRec/JetClusterer.h"

#include "JetEDM/PseudoJetVector.h"
#include "JetEDM/ClusterSequence.h"

#include "fastjet/PseudoJet.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/JetDefinition.hh"

#include "xAODJet/JetAuxContainer.h"
#include "xAODTracking/VertexContainer.h"

class JetClustererByVertex: public JetClusterer{
  ASG_TOOL_CLASS(JetClustererByVertex, IJetProvider)


public:

  JetClustererByVertex(const std::string &name);
  
  StatusCode initialize() override;

  /// Return the final jets with their aux store.
  /// Can return a pair of null pointers if an error occurs.
  std::pair<std::unique_ptr<xAOD::JetContainer>, std::unique_ptr<SG::IAuxStore> > getJets() const override; 


private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainer_key{this, "VertexContainer", "PrimaryVertices", "SG key for input vertex container"};
};


#endif
