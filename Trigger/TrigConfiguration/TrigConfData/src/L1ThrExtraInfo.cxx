/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigConfData/L1ThrExtraInfo.h"

#include <boost/lexical_cast.hpp>
#include <stdexcept>

using namespace std;

std::unique_ptr<TrigConf::L1ThrExtraInfoBase>
TrigConf::L1ThrExtraInfo::createExtraInfo(const std::string & thrTypeName, const boost::property_tree::ptree & data) {
   std::unique_ptr<TrigConf::L1ThrExtraInfoBase> extraInfo(nullptr);

   if( thrTypeName == "EM" )
      return std::make_unique<L1ThrExtraInfo_EMTAULegacy>(thrTypeName, data);

   if( thrTypeName == "TAU" )
      return std::make_unique<L1ThrExtraInfo_EMTAULegacy>(thrTypeName, data);      

   if( thrTypeName == "JET" )
      return std::make_unique<L1ThrExtraInfo_JETLegacy>(thrTypeName, data);

   if( thrTypeName == "XS" )
      return std::make_unique<L1ThrExtraInfo_XSLegacy>(thrTypeName, data);      

   if( thrTypeName == "MU" )
      return std::make_unique<L1ThrExtraInfo_MU>(thrTypeName, data);

   if( thrTypeName == "eEM" )
      return std::make_unique<L1ThrExtraInfo_eEM>(thrTypeName, data);

   if( thrTypeName == "eTAU" )
      return std::make_unique<L1ThrExtraInfo_eTAU>(thrTypeName, data);

   if( thrTypeName == "jTAU" )
      return std::make_unique<L1ThrExtraInfo_jTAU>(thrTypeName, data);

   if( thrTypeName == "cTAU" )
      return std::make_unique<L1ThrExtraInfo_cTAU>(thrTypeName, data);

   if( thrTypeName == "jJ" )
      return std::make_unique<L1ThrExtraInfo_jJ>(thrTypeName, data);      

   // if no special extra information is supplied for the threshold type return base class
   return std::make_unique<L1ThrExtraInfoBase>(thrTypeName, data);
}

void
TrigConf::L1ThrExtraInfo::clear()
{
   m_thrExtraInfo.clear();
}


std::weak_ptr<TrigConf::L1ThrExtraInfoBase>
TrigConf::L1ThrExtraInfo::addExtraInfo(const std::string & thrTypeName, const boost::property_tree::ptree & data) {
   try {
      if( auto extraInfo = L1ThrExtraInfo::createExtraInfo( thrTypeName, data) ) {
         auto success = m_thrExtraInfo.emplace(thrTypeName, std::shared_ptr<TrigConf::L1ThrExtraInfoBase>(std::move(extraInfo)));
         return std::weak_ptr<TrigConf::L1ThrExtraInfoBase>( success.first->second );
      }
   }
   catch(std::exception & ex) {
      std::cerr << "L1ThrExtraInfo::addExtraInfo: exception occured when building extra info for " << thrTypeName << std::endl;
      throw;
   }
   return std::weak_ptr<TrigConf::L1ThrExtraInfoBase>( m_emptyInfo );
}

bool
TrigConf::L1ThrExtraInfo::hasInfo(const std::string & typeName) const
{
   return ( m_thrExtraInfo.find(typeName) != m_thrExtraInfo.end() );
}

const TrigConf::L1ThrExtraInfo_EMTAULegacy &
TrigConf::L1ThrExtraInfo::EM() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_EMTAULegacy&>( * m_thrExtraInfo.at("EM") );
}

const TrigConf::L1ThrExtraInfo_EMTAULegacy &
TrigConf::L1ThrExtraInfo::TAU() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_EMTAULegacy&>( * m_thrExtraInfo.at("TAU") );
}

const TrigConf::L1ThrExtraInfo_XSLegacy &
TrigConf::L1ThrExtraInfo::XS() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_XSLegacy&>( * m_thrExtraInfo.at("XS") );
}

const TrigConf::L1ThrExtraInfo_JETLegacy &
TrigConf::L1ThrExtraInfo::JET() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_JETLegacy&>( * m_thrExtraInfo.at("JET") );
}

const TrigConf::L1ThrExtraInfo_eEM &
TrigConf::L1ThrExtraInfo::eEM() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_eEM&>( * m_thrExtraInfo.at("eEM") );
}

const TrigConf::L1ThrExtraInfo_eTAU &
TrigConf::L1ThrExtraInfo::eTAU() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_eTAU&>( * m_thrExtraInfo.at("eTAU") );
}

const TrigConf::L1ThrExtraInfo_jTAU &
TrigConf::L1ThrExtraInfo::jTAU() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_jTAU&>( * m_thrExtraInfo.at("jTAU") );
}

const TrigConf::L1ThrExtraInfo_cTAU &
TrigConf::L1ThrExtraInfo::cTAU() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_cTAU&>( * m_thrExtraInfo.at("cTAU") );
}

const TrigConf::L1ThrExtraInfo_jJ &
TrigConf::L1ThrExtraInfo::jJ() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_jJ&>( * m_thrExtraInfo.at("jJ") );
}

const TrigConf::L1ThrExtraInfo_gXE &
TrigConf::L1ThrExtraInfo::gXE() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_gXE&>( * m_thrExtraInfo.at("gXE") );
}

const TrigConf::L1ThrExtraInfo_MU &
TrigConf::L1ThrExtraInfo::MU() const {
   return dynamic_cast<const TrigConf::L1ThrExtraInfo_MU&>( * m_thrExtraInfo.at("MU") );
}

const TrigConf::L1ThrExtraInfoBase &
TrigConf::L1ThrExtraInfo::thrExtraInfo(const std::string & thrTypeName) const
{
   try {
      return * m_thrExtraInfo.at(thrTypeName);
   }
   catch(std::exception & ex) {
      std::cerr << "Threshold type " << thrTypeName << " does not have extra info defined" << endl;
      throw;
   }
}

/**
 * EM legacy extra info
 */
const TrigConf::IsolationLegacy &
TrigConf::L1ThrExtraInfo_EMTAULegacy::isolation(const std::string & thrType, size_t bit) const
{
   if(bit<1 or bit>5) {
      throw std::out_of_range("When accessing the legacy L1Calo EM or TAU isolation bit must be between 1 and 5, but bit=" 
                              + std::to_string(bit) + " was requested");
   }
   try {
      return m_isolation.at(thrType)[bit-1];
   }
   catch(std::exception & ex) {
      std::cerr << "Threshold type " << name() << " does not have isolation parameters for type " << thrType << endl;
      throw;
   }
}

void
TrigConf::L1ThrExtraInfo_EMTAULegacy::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "ptMinToTopo" ) {
         m_ptMinToTopoMeV = std::lround( 1000 * x.second.getValue<float>() );
      } else if( x.first == "isolation" ) {
         for( auto & y : x.second.data() ) {
            auto & isoV = m_isolation[y.first] = std::vector<IsolationLegacy>(5);
            for(auto & c : y.second.get_child("Parametrization") ) {
               auto iso = IsolationLegacy(c.second);
               isoV[iso.isobit()-1] = iso;
            }
         }
      }
   }
}


void
TrigConf::L1ThrExtraInfo_XSLegacy::load()
{
   if( hasExtraInfo("significance") ) {
      auto & sig = m_extraInfo["significance"];
      m_xeMin = sig.getAttribute<unsigned int>("xeMin");
      m_xeMax = sig.getAttribute<unsigned int>("xeMax");
      m_teSqrtMin = sig.getAttribute<unsigned int>("teSqrtMin");
      m_teSqrtMax = sig.getAttribute<unsigned int>("teSqrtMax");
      m_xsSigmaScale = sig.getAttribute<unsigned int>("xsSigmaScale");
      m_xsSigmaOffset = sig.getAttribute<unsigned int>("xsSigmaOffset");
   }
}



/**
 * JET legacy extra info
 */
void
TrigConf::L1ThrExtraInfo_JETLegacy::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "ptMinToTopoLargeWindow" ) {
         m_ptMinToTopoLargeWindowMeV = std::lround( 1000 * x.second.getValue<float>() );
      } else if( x.first == "ptMinToTopoSmallWindow" ) {
         m_ptMinToTopoSmallWindowMeV = std::lround( 1000 * x.second.getValue<float>() );
      }
   }
}


/***********************************
 *
 * Extra info for new thresholds
 *
 ***********************************/

/*******
 * eEM
 *******/
TrigConf::L1ThrExtraInfo_eEM::WorkingPoints_eEM::WorkingPoints_eEM( const boost::property_tree::ptree & pt ) {
   m_isDefined = true;
   m_reta_d  = pt.get_optional<float>("reta").get_value_or(0);
   m_wstot_d = pt.get_optional<float>("wstot").get_value_or(0);
   m_rhad_d  = pt.get_optional<float>("rhad").get_value_or(0);
   m_reta_fw  = pt.get_optional<int>("reta_fw").get_value_or(0);
   m_wstot_fw = pt.get_optional<int>("wstot_fw").get_value_or(0);
   m_rhad_fw  = pt.get_optional<int>("rhad_fw").get_value_or(0);
   m_maxEt = pt.get_optional<unsigned int>("maxEt").get_value_or(0);
}

std::ostream &
TrigConf::operator<<(std::ostream & os, const TrigConf::L1ThrExtraInfo_eEM::WorkingPoints_eEM & iso) {
   os << "reta_fw=" << iso.reta_fw() << ", wstot_fw=" << iso.wstot_fw() << ", rhad_fw=" << iso.rhad_fw();
   return os;
}

void
TrigConf::L1ThrExtraInfo_eEM::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "ptMinToTopo" ) {
         m_ptMinToTopoMeV = lround(1000 * x.second.getValue<float>());
      } else if( x.first == "workingPoints" ) {
         for( auto & y : x.second.data() ) {
            auto wp = Selection::stringToWP(y.first);
            auto & iso = m_isolation.emplace(wp, string("eEM_WP_" + y.first)).first->second;
            for(auto & c : y.second ) {
               int etamin = c.second.get_optional<int>("etamin").get_value_or(-49);
               int etamax = c.second.get_optional<int>("etamax").get_value_or(49);
               unsigned int priority = c.second.get_optional<unsigned int>("priority").get_value_or(0);
               iso.addRangeValue(WorkingPoints_eEM(c.second), etamin, etamax, priority, /*symmetric=*/ false);
            }
         }
      }
   }
}


/*******
 * eTAU
 *******/
TrigConf::L1ThrExtraInfo_eTAU::WorkingPoints_eTAU::WorkingPoints_eTAU( const boost::property_tree::ptree & pt ) {
   m_isDefined = true;
   m_isoConeRel_d    = pt.get_optional<float>("isoConeRel").get_value_or(0);
   m_fEM_d           = pt.get_optional<float>("fEM").get_value_or(0);
   m_isoConeRel_fw   = pt.get_optional<float>("isoConeRel_fw").get_value_or(0);
   m_fEM_fw          = pt.get_optional<float>("fEM_fw").get_value_or(0);
   m_maxEt           = pt.get_optional<unsigned int>("maxEt").get_value_or(0);
}

std::ostream &
TrigConf::operator<<(std::ostream & os, const TrigConf::L1ThrExtraInfo_eTAU::WorkingPoints_eTAU & iso) {
   os << "isoConeRel_fw=" << iso.isoConeRel_fw() << ", fEM_fw=" << iso.fEM_fw() ;
   return os;
}

void
TrigConf::L1ThrExtraInfo_eTAU::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "ptMinToTopo" ) {
         m_ptMinToTopoMeV = lround(1000 * x.second.getValue<float>());
      } else if( x.first == "workingPoints" ) {
         for( auto & y : x.second.data() ) {
            auto wp = TrigConf::Selection::stringToWP(y.first);
            auto & iso = m_isolation.emplace(wp, string("eTAU_WP_" + y.first)).first->second;
            for(auto & c : y.second ) {
               int etamin = c.second.get_optional<int>("etamin").get_value_or(-49);
               int etamax = c.second.get_optional<int>("etamax").get_value_or(49);
               unsigned int priority = c.second.get_optional<unsigned int>("priority").get_value_or(0);
               iso.addRangeValue(WorkingPoints_eTAU(c.second), etamin, etamax, priority, /*symmetric=*/ false);
            }
         }
      }
   }
}

/*******
 * jTAU
 *******/
TrigConf::L1ThrExtraInfo_jTAU::WorkingPoints_jTAU::WorkingPoints_jTAU( const boost::property_tree::ptree & pt ) {
   m_isDefined = true;
   m_isolation_d    = pt.get_optional<float>("isolation").get_value_or(0);
   m_isolation_fw   = pt.get_optional<float>("isolation_fw").get_value_or(0);
   m_maxEt          = pt.get_optional<unsigned int>("maxEt").get_value_or(0); 
}

std::ostream &
TrigConf::operator<<(std::ostream & os, const TrigConf::L1ThrExtraInfo_jTAU::WorkingPoints_jTAU & iso) {
   os << "isolation_fw=" << iso.isolation_fw() ;
   return os;
}

void
TrigConf::L1ThrExtraInfo_jTAU::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "ptMinToTopo" ) {
         m_ptMinToTopoMeV = lround(1000 * x.second.getValue<float>());
      } else if( x.first == "workingPoints" ) {
         for( auto & y : x.second.data() ) {
            auto wp = TrigConf::Selection::stringToWP(y.first);
            auto & iso = m_isolation.emplace(wp, string("jTAU_WP_" + y.first)).first->second;
            for(auto & c : y.second ) {
               int etamin = c.second.get_optional<int>("etamin").get_value_or(-49);
               int etamax = c.second.get_optional<int>("etamax").get_value_or(49);
               unsigned int priority = c.second.get_optional<unsigned int>("priority").get_value_or(0);
               iso.addRangeValue(WorkingPoints_jTAU(c.second), etamin, etamax, priority, /*symmetric=*/ false);
            }
         }
      }
   }
}

/*******
 * cTAU
 *******/
TrigConf::L1ThrExtraInfo_cTAU::WorkingPoints_cTAU::WorkingPoints_cTAU( const boost::property_tree::ptree & pt ) {
   m_isDefined = true;
   m_isolation_d    = pt.get_optional<float>("isolation").get_value_or(0);
   m_isolation_fw   = pt.get_optional<float>("isolation_fw").get_value_or(0);
   m_maxEt          = pt.get_optional<unsigned int>("maxEt").get_value_or(0);
}

std::ostream &
TrigConf::operator<<(std::ostream & os, const TrigConf::L1ThrExtraInfo_cTAU::WorkingPoints_cTAU & iso) {
   os << "isolation_fw=" << iso.isolation_fw() ;
   return os;
}

void
TrigConf::L1ThrExtraInfo_cTAU::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "workingPoints" ) {
         for( auto & y : x.second.data() ) {
            auto wp = TrigConf::Selection::stringToWP(y.first);
            auto & iso = m_isolation.emplace(wp, string("cTAU_WP_" + y.first)).first->second;
            for(auto & c : y.second ) {
               int etamin = c.second.get_optional<int>("etamin").get_value_or(-49);
               int etamax = c.second.get_optional<int>("etamax").get_value_or(49);
               unsigned int priority = c.second.get_optional<unsigned int>("priority").get_value_or(0);
               iso.addRangeValue(WorkingPoints_cTAU(c.second), etamin, etamax, priority, /*symmetric=*/ false);
            }
         }
      }
   }
}


/*******
 * jJ
 *******/
void
TrigConf::L1ThrExtraInfo_jJ::load()
{
   for( auto & x : m_extraInfo ) {
      if( x.first == "ptMinToTopo" ) {
         for( auto & k : x.second.data() ) {
            auto etamin = k.second.get_child("etamin").get_value<unsigned int>();
            auto etamax = k.second.get_child("etamax").get_value<unsigned int>();
            auto value = k.second.get_child("value").get_value<float>();
            auto priority = k.second.get_optional<unsigned int>("priority").get_value_or(0);            
            m_ptMinToTopoMeV.addRangeValue( lround(1000*value),
                                            etamin, etamax, priority, /*symmetric=*/ false);
         }
      }
   }
}

/*******
 * gXE
 *******/
void
TrigConf::L1ThrExtraInfo_gXE::load()
{
}

/*******
 * MU
 *******/
unsigned int
TrigConf::L1ThrExtraInfo_MU::rpcIdxForPt(unsigned int pt) const
{
   try {
      return m_rpcPtMap.at(pt);
   }
   catch(std::exception & ex) {
      std::cerr << "No RPC index defined for pt " << pt << endl;
      throw;
   }   
}


unsigned int
TrigConf::L1ThrExtraInfo_MU::tgcIdxForPt(unsigned int pt) const
{
   try {
      return m_tgcPtMap.at(pt);
   }
   catch(std::exception & ex) {
      std::cerr << "No TGC index defined for pt " << pt << endl;
      throw;
   }
}

std::vector<unsigned int>
TrigConf::L1ThrExtraInfo_MU::knownRpcPtValues() const
{
   std::vector<unsigned int> ptValues;   
   for( auto & x : m_rpcPtMap ) {
      ptValues.emplace_back(x.first);
   }
   return ptValues;
}

std::vector<unsigned int>
TrigConf::L1ThrExtraInfo_MU::knownTgcPtValues() const
{
   std::vector<unsigned int> ptValues;   
   for( auto & x : m_tgcPtMap ) {
      ptValues.emplace_back(x.first);
   }
   return ptValues;
}


std::vector<std::string>
TrigConf::L1ThrExtraInfo_MU::exclusionListNames() const
{
   std::vector<std::string> listNames;   
   for( auto & x : m_roiExclusionLists ) {
      listNames.emplace_back(x.first);
   }
   return listNames;
}


const std::map<std::string, std::vector<unsigned int> > &
TrigConf::L1ThrExtraInfo_MU::exclusionList(const std::string & listName) const
{
   try {
      return m_roiExclusionLists.at(listName);
   }
   catch(std::exception & ex) {
      std::cerr << "No exclusion list '" << listName << "' defined in MU threshold exlusionLists" << endl;
      throw;
   }
}

void
TrigConf::L1ThrExtraInfo_MU::load()
{
   for( auto & x : m_extraInfo["roads"].getObject("rpc").data() ) {
      m_rpcPtMap.emplace( boost::lexical_cast<unsigned int, std::string>(x.first),
                          boost::lexical_cast<unsigned int, std::string>(x.second.data()));
   }
   for( auto & x : m_extraInfo["roads"].getObject("tgc").data() ) {
      m_tgcPtMap.emplace( boost::lexical_cast<unsigned int, std::string>(x.first),
                          boost::lexical_cast<unsigned int, std::string>(x.second.data()));
   }

   for( auto & x : m_extraInfo["exclusionLists"].data() ) {
      const std::string & listName = x.first;
      std::map<std::string, std::vector<unsigned int>> roisBySector;
      for( auto & list : x.second ) {
         const std::string & sectorName = list.second.get_child("sectorName").get_value<std::string>();
         std::vector<unsigned int> rois;
         for( auto & roi : list.second.get_child("rois") ) {
            rois.push_back( boost::lexical_cast<unsigned int, std::string>( roi.second.data() ) );
         }
         roisBySector.emplace(sectorName, std::move(rois));
      }
      m_roiExclusionLists.emplace(listName, std::move(roisBySector));
   }
}


