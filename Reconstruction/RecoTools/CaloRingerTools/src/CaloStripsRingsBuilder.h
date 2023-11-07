/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALORINGERTOOLS_CALOSTRIPSRINGSBUILDER_H
#define CALORINGERTOOLS_CALOSTRIPSRINGSBUILDER_H

// Base includes:
#include "CaloRingerTools/ICaloRingsBuilder.h"
#include "CaloRingsBuilder.h"

namespace Ringer {

class CaloStripsRingsBuilder : public CaloRingsBuilder
{

  public:

    /// @name RawCaloStripsBuilder ctors and dtors:
    /// @{
    /**
     * @brief Default constructor
     **/
    CaloStripsRingsBuilder(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);

    /** 
     * @brief Destructor
     **/
    ~CaloStripsRingsBuilder();
    /// @}
    
    /// Tool main methods:
    /// @{
    /** 
     * @brief initialize method 
     **/
    virtual StatusCode initialize() override;
    /** 
     * @brief finalize method 
     **/
    virtual StatusCode finalize() override;
    /// @}

  protected:

    /**
     * @brief main method where the strips are build
     *
     * NOTE: Please note that RingSets are the strip sets
     **/
    virtual StatusCode buildRingSet(
        const xAOD::RingSetConf::RawConf &rawConf,
        const AtlasGeoPoint &seed,
        xAOD::RingSet *rs);
    /// @}

    /// Tool props (python configurables):
    /// @{
    /**
     * @brief the axis to build the strips
     **/
    std::size_t m_axis;
    /// @}
    // Tool pro (python configurables):
    /// @{
    /**
     * @brief This can be set True to divide in two the eta axes.
    **/
    bool m_doEtaAxesDivision;
    /**
     * @brief This can be set True to divide in two the phi axes.
    **/
    bool m_doPhiAxesDivision;

    bool m_doTransverseEnergy;


    /// @}

};

} // namespace Ringer

#endif


