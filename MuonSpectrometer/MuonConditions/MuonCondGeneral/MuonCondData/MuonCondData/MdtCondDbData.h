/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_MDTCONDDBDATA_H
#define MUONCONDDATA_MDTCONDDBDATA_H

//STL includes
#include <set>

//Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "MuonCondData/Defs.h"

//forward declarations
class Identifier;
class MdtIdHelper;


class MdtCondDbData {

  friend class MdtCondDbAlg;

public:

    MdtCondDbData(const MdtIdHelper& id_helper);
    
    
    virtual ~MdtCondDbData() = default;
    /// @brief  The indiviudal tube is dead
    void setDeadTube      (const Identifier& ident);
    /// @brief  All tubes in a drift layer are dead
    void setDeadLayer     (const Identifier& ident);
    /// @brief  All tubes in a multi layer are dead
    void setDeadMultilayer(const Identifier& ident);
    /// @brief All tubes in a chamber are dead
    void setDeadChamber   (const Identifier& ident);
   
    //// Returns a list of Identifiers of dead tubes / layers / etc.
    const std::set<Identifier>& getDeadTubesId      () const;
    const std::set<Identifier>& getDeadLayersId     () const;
    const std::set<Identifier>& getDeadMultilayersId() const;
    const std::set<Identifier>& getDeadChambersId   () const;
   
    /// Returns if the identifier (tube/multiLayer/chamber) is masked
    /// in the conditions database
    bool isGood          (const Identifier & Id) const;
    /// Returns whether the particular tube has 
    ///been markes as bad in the database
    bool isGoodTube      (const Identifier & Id) const;
    /// Returns whether the corresponding tube layer is
    /// marked as bad in the database    
    bool isGoodLayer     (const Identifier & Id) const;
    bool isGoodMultilayer(const Identifier & Id) const;
    ///  Returns true if the complete chamber has not dead channels
    bool isGoodChamber   (const Identifier & Id) const;

    using DcsFsmState = MuonCond::DcsFsmState;
    using DcsConstants = MuonCond::DcsConstants;

    /** @brief Adds a DCS state to the conditions object
     *       multiLayerID -> Identifier of a tube in the multilayer
     *       state -> DCS state flag
     *       standByVolt: voltage if system is at standby
     *       readyVolt: Voltage if system is ready for data-taking
    */ 
    void setHvState(const Identifier& multiLayerID, 
                    const DcsFsmState state,
                    const float standByVolt,
                    const float readyVolt);
    
    const DcsConstants& getHvState(const Identifier& multiLayerID) const;

    const std::vector<DcsConstants>& getAllHvStates() const;
private:
    std::set<Identifier> m_cachedDeadTubes{};
    std::set<Identifier> m_cachedDeadLayers{};
    std::set<Identifier> m_cachedDeadMultilayers{};
    std::set<Identifier> m_cachedDeadChambers{};

    std::vector<DcsConstants> m_dcsStates{};
    const MdtIdHelper& m_id_helper;

};

CLASS_DEF( MdtCondDbData, 58088442, 1)
CLASS_DEF( CondCont<MdtCondDbData>, 62077248, 0)

#endif
