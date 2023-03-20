/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondData/MdtCondDbData.h"
#include "MuonIdHelpers/MdtIdHelper.h"

// --- writing identifiers -------
MdtCondDbData::MdtCondDbData(const MdtIdHelper& id_helper):
    m_id_helper(id_helper){}
// setDeadTube
void
MdtCondDbData::setDeadTube(std::string_view id_name, Identifier Id){
    m_cachedDeadTubes  .emplace(std::string(id_name));
    m_cachedDeadTubesId.insert(Id  );
}

// setDeadLayer
void
MdtCondDbData::setDeadLayer(std::string_view id_name, Identifier Id){
    m_cachedDeadLayers  .emplace(id_name);
    m_cachedDeadLayersId.insert(Id  );
}

// setDeadMultilayer
void
MdtCondDbData::setDeadMultilayer(std::string_view id_name, Identifier Id){
    m_cachedDeadMultilayers  .emplace(id_name);
    m_cachedDeadMultilayersId.insert(Id  );
}

// setDeadStation (= a chamber dead by itself)
void
MdtCondDbData::setDeadStation(std::string_view id_name, Identifier Id){
    m_cachedDeadStations  .emplace(id_name);
    m_cachedDeadStationsId.insert(Id  );
}

// setDeadChamber (= a chamber with dead channels)
void
MdtCondDbData::setDeadChamber(Identifier Id){
    m_cachedDeadChambersId.insert(Id  );
}

// setNoisyTube
void
MdtCondDbData::setNoisyTube(Identifier Id){
    m_cachedNoisyTubesId.insert(Id  );
}

// setNoisyLayer
void
MdtCondDbData::setNoisyLayer(Identifier Id){
    m_cachedNoisyLayersId.insert(Id  );
}

// setNoisyMultilayer
void
MdtCondDbData::setNoisyMultilayer(Identifier Id){
    m_cachedNoisyMultilayersId.insert(Id  );
}

// setNoisyStation
void
MdtCondDbData::setNoisyStation(Identifier Id){
    m_cachedNoisyStationsId.insert(Id  );
}

// setNoisyChamber
void
MdtCondDbData::setNoisyChamber(Identifier Id){
    m_cachedNoisyChambersId.insert(Id  );
}





// --- reading identifiers -------

const std::set<std::string>& MdtCondDbData::getDeadTubes() const{return m_cachedDeadTubes;}
const std::set<std::string>& MdtCondDbData::getDeadLayers() const{return m_cachedDeadLayers;}
const std::set<std::string>& MdtCondDbData::getDeadMultilayers() const{ return m_cachedDeadMultilayers;}
const std::set<std::string>& MdtCondDbData::getDeadStations() const{ return m_cachedDeadStations;}
const std::set<std::string>& MdtCondDbData::getDeadChambers() const{return m_cachedDeadChambers;}



const std::set<Identifier>& MdtCondDbData::getDeadTubesId() const{ return m_cachedDeadTubesId;}
const std::set<Identifier>& MdtCondDbData::getDeadLayersId() const{ return m_cachedDeadLayersId; }
const std::set<Identifier>& MdtCondDbData::getDeadMultilayersId() const{ return m_cachedDeadMultilayersId;}
const std::set<Identifier>& MdtCondDbData::getDeadStationsId() const{ return m_cachedDeadStationsId;}
const std::set<Identifier>& MdtCondDbData::getDeadChambersId() const{ return m_cachedDeadChambersId;}



const std::set<std::string>& MdtCondDbData::getNoisyTubes() const{ return m_cachedNoisyTubes;}
const std::set<std::string>& MdtCondDbData::getNoisyLayers() const{ return m_cachedNoisyLayers;}
const std::set<std::string>& MdtCondDbData::getNoisyMultilayers() const{return m_cachedNoisyMultilayers;}
const std::set<std::string>& MdtCondDbData::getNoisyStations() const{ return m_cachedNoisyStations;}
const std::set<std::string>& MdtCondDbData::getNoisyChambers() const{ return m_cachedNoisyChambers;}



const std::set<Identifier>& MdtCondDbData::getNoisyTubesId() const{ return m_cachedNoisyTubesId;}
const std::set<Identifier>& MdtCondDbData::getNoisyLayersId() const{return m_cachedNoisyLayersId;}
const std::set<Identifier>& MdtCondDbData::getNoisyMultilayersId() const{return m_cachedNoisyMultilayersId;}
const std::set<Identifier>& MdtCondDbData::getNoisyStationsId() const{ return m_cachedNoisyStationsId;}
const std::set<Identifier>& MdtCondDbData::getNoisyChambersId() const{return m_cachedNoisyChambersId;}



// --- probing identifiers -------

bool 
MdtCondDbData::isGood(const Identifier & Id) const{
    // probing id in all lists
    const Identifier multilayerId = m_id_helper.multilayerID(Id); 
    const Identifier chamberId    = m_id_helper.elementID   (Id); 
    if(!isGoodStation   (chamberId   )) return false;
    if(!isGoodMultilayer(multilayerId)) return false;
    if(!isGoodTube      (Id          )) return false;
    return true;
}

bool MdtCondDbData::isGoodTube(const Identifier & Id) const{
    return !m_cachedDeadTubesId.count(Id);
} 

bool MdtCondDbData::isGoodLayer(const Identifier & Id) const{
    return !m_cachedDeadLayersId.count(Id);
} 

bool MdtCondDbData::isGoodMultilayer(const Identifier & Id) const{
    return !m_cachedDeadMultilayersId.count(Id);
} 

bool MdtCondDbData::isGoodStation(const Identifier & Id) const{
    return !m_cachedDeadStationsId.count(Id);
} 

// isGoodChamber
/// this method probably doesn't do what you expect it to - it returns whether there is a
/// bad multilayer/tube inside the chamber, not if the full chamber is disabled. For the
/// latter, you need to use isGoodStation.
bool
MdtCondDbData::isGoodChamber(const Identifier & Id) const{
    return !m_cachedDeadChambersId.count(Id);
} 



