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

//forward declarations
class Identifier;
class MdtIdHelper;


class MdtCondDbData {

  friend class MdtCondDbAlg;

public:

    MdtCondDbData(const MdtIdHelper& id_helper);
    
    
    virtual ~MdtCondDbData() = default;

    void setDeadTube      (std::string_view, Identifier);
    void setDeadLayer     (std::string_view, Identifier);
    void setDeadMultilayer(std::string_view, Identifier);
    void setDeadStation   (std::string_view, Identifier);
    void setDeadChamber   (Identifier);

    void setNoisyTube      (Identifier);
    void setNoisyLayer     (Identifier);
    void setNoisyMultilayer(Identifier);
    void setNoisyStation   (Identifier);
    void setNoisyChamber   (Identifier);
   
    const std::set<std::string>& getDeadTubes      () const;
    const std::set<std::string>& getDeadLayers     () const;
    const std::set<std::string>& getDeadMultilayers() const;
    const std::set<std::string>& getDeadStations   () const;
    const std::set<std::string>& getDeadChambers   () const;
    
    const std::set<Identifier>& getDeadTubesId      () const;
    const std::set<Identifier>& getDeadLayersId     () const;
    const std::set<Identifier>& getDeadMultilayersId() const;
    const std::set<Identifier>& getDeadStationsId   () const;
    const std::set<Identifier>& getDeadChambersId   () const;

    const std::set<std::string>& getNoisyTubes      () const;
    const std::set<std::string>& getNoisyLayers     () const;
    const std::set<std::string>& getNoisyMultilayers() const;
    const std::set<std::string>& getNoisyStations   () const;
    const std::set<std::string>& getNoisyChambers   () const;
    
    const std::set<Identifier>& getNoisyTubesId      () const;
    const std::set<Identifier>& getNoisyLayersId     () const;
    const std::set<Identifier>& getNoisyMultilayersId() const;
    const std::set<Identifier>& getNoisyStationsId   () const;
    const std::set<Identifier>& getNoisyChambersId   () const;
  
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
    ///  Returns true if the chamber is switched off in the 
    ///  database
    bool isGoodStation   (const Identifier & Id) const;
    ///  Returns true if the complete chamber has not dead channels
    bool isGoodChamber   (const Identifier & Id) const;
   
 
private:

    std::set<std::string> m_cachedDeadTubes{};
    std::set<std::string> m_cachedDeadLayers{};
    std::set<std::string> m_cachedDeadMultilayers{};
    std::set<std::string> m_cachedDeadStations{};
    std::set<std::string> m_cachedDeadChambers{};

    std::set<Identifier> m_cachedDeadTubesId{};
    std::set<Identifier> m_cachedDeadLayersId{};
    std::set<Identifier> m_cachedDeadMultilayersId{};
    std::set<Identifier> m_cachedDeadStationsId{};
    std::set<Identifier> m_cachedDeadChambersId{};
 
    std::set<std::string> m_cachedNoisyTubes{};
    std::set<std::string> m_cachedNoisyLayers{};
    std::set<std::string> m_cachedNoisyMultilayers{};
    std::set<std::string> m_cachedNoisyStations{};
    std::set<std::string> m_cachedNoisyChambers{};

    std::set<Identifier> m_cachedNoisyTubesId{};
    std::set<Identifier> m_cachedNoisyLayersId{};
    std::set<Identifier> m_cachedNoisyMultilayersId{};
    std::set<Identifier> m_cachedNoisyStationsId{};
    std::set<Identifier> m_cachedNoisyChambersId{};

    const  MdtIdHelper& m_id_helper;   

};

CLASS_DEF( MdtCondDbData, 58088442, 1)
CLASS_DEF( CondCont<MdtCondDbData>, 62077248, 0)

#endif
