/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: CaloAsymRingsBuilder.h 695492 2015-09-17 16:58:37Z wsfreund $
#ifndef CALORINGERTOOLS_CALOASYMRINGSBUILDER_H
#define CALORINGERTOOLS_CALOASYMRINGSBUILDER_H

// Base includes:
#include "CaloRingerTools/ICaloRingsBuilder.h"
#include "CaloRingsBuilder.h"

namespace Ringer {

class CaloAsymRingsBuilder : public CaloRingsBuilder
{

  public:

    /// @name CaloAsymRingsBuilder ctors and dtors:
    /// @{
    /** 
     * @brief Default constructor
     **/
    CaloAsymRingsBuilder(const std::string& type,
                         const std::string& name,
                         const IInterface* parent);

    /** 
     * @brief Destructor
     **/
    ~CaloAsymRingsBuilder();
    /// @}
    
    /// Tool main methods:
    /// @{
    /** 
     * @brief initialize method 
     **/
    virtual StatusCode initialize() ATH_OVERRIDE;
    /** 
     * @brief finalize method 
     **/
    virtual StatusCode finalize() ATH_OVERRIDE;
    /// @}
 
  protected:
    //
    /// Tool protected methods:
    /// @{
    /** 
     * @brief main method where the RingSets are built.
    /// @}

     **/
    StatusCode buildRingSet( 
        const xAOD::RingSetConf::RawConf &rawConf,
        const AtlasGeoPoint &seed,
        xAOD::RingSet *rs);
    /// @}

    // Tool pro (python configurables):
    /// @{
    /**
     * @brieg This can be set True to divide in two the eta axes.
    **/
    bool m_doEtaAxesDivision;
    /**
     * @brieg This can be set True to divide in two the phi axes.
    **/
    bool m_doPhiAxesDivision;

    /// @}
    
   
};

} // namespace Ringer

#endif


