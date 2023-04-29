/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigConfData/HLTMonitoring.h"

#include <iomanip>

using TV = boost::property_tree::ptree::value_type;  // tree-value type
using namespace std;

TrigConf::HLTMonitoring::HLTMonitoring()
{}

TrigConf::HLTMonitoring::HLTMonitoring(const boost::property_tree::ptree & data) 
   : DataStructure(data)
{
   load();
}

void
TrigConf::HLTMonitoring::load()
{
   if(! isInitialized() || empty() ) {
      return;
   }
   m_name = getAttribute("name");

   // signatures
   try {
      for( const auto & [monGroupName, monGroup] : data().get_child( "signatures" ) ) {
         std::map<std::string, std::vector<std::string>> monGroupChainMap;
         for(const auto & [chainName, targetList] : monGroup) {
            std::vector<std::string> monTarget{};
            for(const auto & x: targetList) {
               monTarget.push_back(x.second.get_value<std::string>());
               m_targets.insert(monTarget.back());
            }
            monGroupChainMap.emplace( chainName, monTarget);
         }
         m_signatures.emplace( monGroupName, monGroupChainMap);
      }
   }
   catch(std::exception & ex) {
      std::cerr << "ERROR: problem when building the HLT monitoring. " << ex.what() << std::endl;
      throw;
   }
}

const std::set<std::string> & 
TrigConf::HLTMonitoring::targets() const {
   return m_targets;
}

const std::map<std::string, std::map<std::string, std::vector<std::string>>> &
TrigConf::HLTMonitoring::signatures() const {
   return m_signatures;
}

std::vector<std::string>
TrigConf::HLTMonitoring::signatureNames() const {
   std::vector<std::string> keys;
   keys.reserve(m_signatures.size());
   for(const auto& [key, value] : m_signatures) {
      keys.push_back(key);
   }
   return keys;
}

std::vector<std::string>
TrigConf::HLTMonitoring::chainsBySignatureAndTarget(const std::string & signature, const std::string & target) const {
   std::vector<std::string> chains{};
   for(const auto & [chain, targets] : m_signatures.at(signature)) {
      if(find(begin(targets), end(targets), target) != end(targets)) {
         chains.push_back(chain);
      }
   }
   return chains;
}

std::size_t
TrigConf::HLTMonitoring::size() const {
   return data().get_child("signatures").size();
}

void
TrigConf::HLTMonitoring::clear()
{
   m_smk = 0;
   m_name = "";
}

unsigned int
TrigConf::HLTMonitoring::smk() const {
   return m_smk;
}

void
TrigConf::HLTMonitoring::setSMK(unsigned int smk) {
   m_smk = smk;
}

void
TrigConf::HLTMonitoring::printMonConfig(bool full) const {
   cout << "HLT monitoring '" << name() << "'" << endl;
   cout << "Signatures: " << size() << endl;
   cout << "Targets: " << targets().size() << endl;
   if(full) {
      cout << "Signatures: ";
      bool first(true);
      size_t sigNameWidth(0);
      for( auto & sigName : signatureNames() ) {
         if(first) {
            first = false;
         } else {
            cout << ", ";
         }
         cout << sigName;
         sigNameWidth = max(sigNameWidth,sigName.size());
      }
      cout << endl << "Targets: ";
      first = true;
      std::vector<size_t> tnwidth{};
      for( auto & target : targets() ) {
         if(first) {
            first = false;
         } else {
            cout << ", ";
         }
         cout << target;
         tnwidth.push_back(target.size());
      }
      cout << endl;

      cout << "Count of monitored chains by target:" << endl;
      first = true;
      for( auto & target : targets() ) {
         if(first) {
            first = false;
            cout << setw(sigNameWidth+2) << "";
         } else {
            cout << ", ";
         }
         cout << target;
      }
      cout << endl;
      for( auto & sigName : signatureNames() ) {
         cout << setw(sigNameWidth) << left << sigName << ": " << right;
         size_t col{0};
         first = true;
         for( auto & target : targets() ) {
            if(first) {
               first = false;
            } else {
               cout << ", ";
            }
            cout << setw(tnwidth[col++]) << chainsBySignatureAndTarget(sigName, target).size();
         }
         cout << endl;
      }



   }
}

