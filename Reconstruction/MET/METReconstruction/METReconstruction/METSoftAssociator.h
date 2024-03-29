///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METSoftAssociator.h 
// Header file for class METSoftAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
/////////////////////////////////////////////////////////////////// 
#ifndef METRECONSTRUCTION_METSOFTASSOCIATOR_H
#define METRECONSTRUCTION_METSOFTASSOCIATOR_H 1

// METReconstruction includes
#include "METReconstruction/METAssociator.h"

namespace met{
  class METSoftAssociator final
    : public METAssociator
  { 
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(METSoftAssociator, IMETAssocToolBase)


    /////////////////////////////////////////////////////////////////// 
    // Public methods: 
    /////////////////////////////////////////////////////////////////// 
    public: 

    // Constructor with name
    METSoftAssociator(const std::string& name);
    ~METSoftAssociator();

    // AsgTool Hooks
    StatusCode initialize();
    StatusCode finalize();

    /////////////////////////////////////////////////////////////////// 
    // Private data: 
    /////////////////////////////////////////////////////////////////// 
    protected: 

    StatusCode executeTool(xAOD::MissingETContainer* metCont, xAOD::MissingETAssociationMap* metMap) const final;

    StatusCode extractPFO(const xAOD::IParticle*,
                          std::vector<const xAOD::IParticle*>&,
                          const met::METAssociator::ConstitHolder&,
                          std::map<const xAOD::IParticle*,MissingETBase::Types::constvec_t>&) const final
    {return StatusCode::FAILURE;} // should not be called
    StatusCode extractFE(const xAOD::IParticle*,
                         std::vector<const xAOD::IParticle*>&,
                         const met::METAssociator::ConstitHolder&,
                         std::map<const xAOD::IParticle*,MissingETBase::Types::constvec_t>&) const final
    {return StatusCode::FAILURE;} // should not be called
    StatusCode extractTracks(const xAOD::IParticle*,
                             std::vector<const xAOD::IParticle*>&,
                             const met::METAssociator::ConstitHolder&) const final
    {return StatusCode::FAILURE;} // should not be called
    StatusCode extractTopoClusters(const xAOD::IParticle*,
                                   std::vector<const xAOD::IParticle*>&,
                                   const met::METAssociator::ConstitHolder&) const final
    {return StatusCode::FAILURE;} // should not be called

    private:
 
    /// Default constructor: 
    METSoftAssociator();

    bool m_decorateSoftTermConst;
    bool m_weight_soft_pfo = false;

    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_lcmodclus_key;
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_emmodclus_key;

  }; 

}

#endif //> !METRECONSTRUCTION_METSOFTASSOCIATOR_H
