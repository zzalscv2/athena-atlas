/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtCablMezzAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/ArrayHelper.h"
#include "MuonCablingData/MdtMezzanineCard.h"
#include <fstream>
using Mapping = MdtMezzanineCard::Mapping;

MdtCablMezzAlg::MdtCablMezzAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm(name,pSvcLocator) {}


StatusCode MdtCablMezzAlg::initialize(){
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());
  return StatusCode::SUCCESS;
} 

StatusCode MdtCablMezzAlg::execute(){
  const EventContext& ctx = Gaudi::Hive::currentContext();
  ATH_MSG_INFO("Start validation of the Mdt cabling");

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorMgr{m_DetectorManagerKey, ctx};
  if (!detectorMgr.isValid()){
      ATH_MSG_FATAL("Failed to retrieve the Detector manager "<<m_DetectorManagerKey.fullKey());
      return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<MuonMDT_CablingMap> cabling{m_cablingKey,ctx};
  if (!cabling.isValid()) {
     ATH_MSG_ERROR("Failed to retrieve the Mdt cabling "<<m_cablingKey.fullKey());
     return StatusCode::FAILURE;
  }
  const MdtIdHelper& idHelper = m_idHelperSvc->mdtIdHelper();
 
  std::vector<MdtMezzanineCard> cached_cards{};
  std::set<MdtCablingData> cached_chnls{};
  for (unsigned int hash = 0; hash < MuonGM::MuonDetectorManager::MdtRElMaxHash; ++hash){
    const IdentifierHash id_hash{hash};

    const MuonGM::MdtReadoutElement* readEle = detectorMgr->getMdtReadoutElement(id_hash);
    if (!readEle) {
        ATH_MSG_DEBUG("Detector element does not exist. ");
        continue;
    }
    const Identifier station_id = idHelper.elementID(readEle->identify());
    ATH_MSG_DEBUG("Check station "<<m_idHelperSvc->toString(station_id));
    const unsigned int coveredTubes = 24 / readEle->getNLayers();
    const unsigned int numMezz = readEle->getNtubesperlayer() / coveredTubes + 
                                (readEle->getNtubesperlayer() % coveredTubes != 0);
    /// Struct to perform the mapping between online offline tube
    MdtMezzanineCard dummy_card(Mapping{}, readEle->getNLayers(), -1);
    for (unsigned int mezz = 0 ; mezz < numMezz ; ++mezz) {
      Mapping mezz_mapping{make_array<uint8_t,24>(MdtMezzanineCard::NOTSET)};
      const unsigned int tubeOffSet = (mezz * coveredTubes);
      for (int layer = 1 ; layer <= readEle->getNLayers(); ++layer){         
         for (unsigned int tube = 1 ; tube <= coveredTubes; ++tube ) {
             unsigned int tubeInLayer = tube + tubeOffSet;
             bool is_valid{false};
             const Identifier tube_id = idHelper.channelID(station_id, readEle->getMultilayer(), 
                                                           layer, tubeInLayer, is_valid);
              if (!is_valid) {
                ATH_MSG_VERBOSE("Invalid element");
                continue;
              }
              /// Create the cabling object
              MdtCablingData cabling_data{};
              cabling->convert(tube_id,cabling_data);
              /// Test if the online channel can be found
              if (!cabling->getOnlineId(cabling_data, msgStream())){
                ATH_MSG_ERROR("Could no retrieve a valid online channel for "<<m_idHelperSvc->toString(tube_id));
                return StatusCode::FAILURE;
              }
              mezz_mapping[cabling_data.channelId] = dummy_card.tubeNumber(layer, tubeInLayer);
         }
      }
      const Identifier mezz_id = idHelper.channelID(station_id, readEle->getMultilayer(), 
                                                    1, 1 + tubeOffSet);

      MdtCablingData mezzCablingId{};
      /// Load the offline identifier
      cabling->convert(mezz_id, mezzCablingId);
      /// Load the corresponding online cabling
      cabling->getOnlineId(mezzCablingId, msgStream());

      std::vector<MdtMezzanineCard>::const_iterator itr = std::find_if(cached_cards.begin(), cached_cards.end(), 
            [&dummy_card, &mezz_mapping](const MdtMezzanineCard& card ){
               if (dummy_card.numTubeLayers() != card.numTubeLayers()) return false;
               for (size_t ch =0; ch < mezz_mapping.size(); ++ch) {
                  if (mezz_mapping[ch] != card.tdcToTubeMap()[ch]) return false;
               }
              return true;
      });
      if (itr != cached_cards.end()) {
        mezzCablingId.mezzanine_type = itr->id();
      } else {
        cached_cards.emplace_back(mezz_mapping, dummy_card.numTubeLayers(), cached_cards.size() + 10);
        if (!cached_cards.back().checkConsistency(msgStream())) {
          ATH_MSG_ERROR("Wrong assignment for "<<m_idHelperSvc->toString(mezz_id));
          return StatusCode::FAILURE;
        }
        mezzCablingId.mezzanine_type = cached_cards.back().id();
      }
      cached_chnls.insert(std::move(mezzCablingId));
    }    
    /// Summary file
    {
        std::ofstream summary{m_summaryTxt};
        if (!summary.good()) {
          ATH_MSG_ERROR("Failed to write "<<m_summaryTxt);
          return StatusCode::FAILURE;
        }
        summary<<"Extracted "<<cached_cards.size()<<" mezzanine card layouts and "
              <<cached_chnls.size()<<" chamber channels. "<<std::endl<<std::endl<<std::endl;
        for (const MdtMezzanineCard& card : cached_cards) {
          summary<<card;
          MdtCablingOffData chamb{};
          for (const MdtCablingData& cabling : cached_chnls) {
              if (cabling.mezzanine_type != card.id()) continue;
              if (chamb != cabling) {
                chamb = cabling;
                summary<<std::endl<<" *** "<<idHelper.stationNameString(chamb.stationIndex);             
                summary<<static_cast<int>(std::abs(chamb.eta));
                summary<<(chamb.eta > 0 ? "A" : "C");
                summary<<static_cast<int>(chamb.phi);
                summary<<"M"<<static_cast<int>(chamb.multilayer);
                summary<<" --- tdcs: ";
              }
              summary<<static_cast<int>(cabling.tdcId)<<", ";
          }
          summary<<std::endl<<std::endl
                  <<"##############################################################"<<std::endl;
        }
    }
    /// Write mezzanine file
    {
      std::ofstream mezz_json{m_mezzJSON};
      if (!mezz_json.good()) {
          ATH_MSG_ERROR("Failed to write "<<m_summaryTxt);
          return StatusCode::FAILURE;
      }
      mezz_json<<"[";
      for (size_t i = 0; i < cached_cards.size() ; ++i) {
         const MdtMezzanineCard& card  = cached_cards[i];
         mezz_json<<"{ \"mezzId\": "<<static_cast<int>(card.id())<<", ";
         mezz_json<<"\"nTubeLayer\": "<<static_cast<int>(card.numTubeLayers())<<", ";
         mezz_json<<"\"tdcToTubeMap\": [";
         for (size_t ch = 0 ; ch < card.tdcToTubeMap().size(); ++ch) {
           mezz_json<<static_cast<int>(card.tdcToTubeMap()[ch]);
           if (ch + 1 != card.tdcToTubeMap().size())mezz_json<<", ";
         }
         mezz_json<<"] }";
         if (i +1 != cached_cards.size()) mezz_json<<",  ";
      }
      mezz_json<<"]";
    }
    {
      std::ofstream chamb_json{m_cablingJSON};
      if (!chamb_json.good()) {
         ATH_MSG_FATAL("Failed to write "<<m_cablingJSON);
         return StatusCode::FAILURE;
      }
      chamb_json<<"[";
      size_t i =0;
      for (const MdtCablingData& chamb : cached_chnls){ 
        chamb_json<<"{";
        chamb_json<<"\"station\": \""<<idHelper.stationNameString(chamb.stationIndex)<<"\", ";
        chamb_json<<"eta: "<<static_cast<int>(chamb.eta)<<", ";
        chamb_json<<"phi: "<<static_cast<int>(chamb.phi)<<", ";
        chamb_json<<"ml: "<<static_cast<int>(chamb.multilayer)<<", ";
        chamb_json<<"subDet: "<<static_cast<int>(chamb.subdetectorId)<<", ";
        chamb_json<<"csm: "<<static_cast<int>(chamb.csm)<<", ";
        chamb_json<<"mrod: "<<static_cast<int>(chamb.mrod)<<", ";
        chamb_json<<"tdcId: "<<static_cast<int>(chamb.tdcId)<<", ";
        chamb_json<<"mezzId: "<<static_cast<int>(chamb.mezzanine_type)<<", ";
        chamb_json<<"tubeZero: "<<static_cast<int>(chamb.tube);
        chamb_json<<"}";        
        if (i +1 != cached_chnls.size()) chamb_json<<",  ";
        ++i;
      }
      chamb_json<<"]";
    }

  }  
  return StatusCode::SUCCESS;
} 
   
