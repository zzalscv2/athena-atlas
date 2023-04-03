/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibIdentifier/MuonFixedLongId.h"

namespace MuonCalib {

void MuonFixedLongId::initFromFixedId(MuonFixedId other) {
  if (other.is_mdt()) {
    set_mdt();
    setStationName(static_cast<StationName>(other.stationName()));
    setStationEta(other.eta());
    setStationPhi(other.phi());
    setMdtMultilayer(other.mdtMultilayer());
    setMdtTubeLayer(other.mdtTubeLayer());
    setMdtTube(other.mdtTube());
  } else if (other.is_rpc()) {
    set_rpc();
    setStationName(static_cast<StationName>(other.stationName()));
    setStationEta(other.eta());
    setStationPhi(other.phi());
    setRpcDoubletR(other.rpcDoubletR());
    setRpcDoubletZ(other.rpcDoubletZ());
    setRpcDoubletPhi(other.rpcDoubletPhi());
    setRpcGasGap(other.rpcGasGap());
    setRpcMeasuresPhi(other.rpcMeasuresPhi());
    setRpcStrip(other.rpcStrip());
  } else if (other.is_csc()) {
    set_csc();
    setStationName(static_cast<StationName>(other.stationName()));
    setStationEta(other.eta());
    setStationPhi(other.phi());
    setCscChamberLayer(other.cscChamberLayer());
    setCscWireLayer(other.cscWireLayer());
    setCscMeasuresPhi(other.cscMeasuresPhi());
    setCscStrip(other.cscStrip());
  } else if (other.is_tgc()) {
    set_tgc();
    setStationName(static_cast<StationName>(other.stationName()));
    setStationEta(other.eta());
    setStationPhi(other.phi());
    setTgcGasGap(other.tgcGasGap());
    setTgcIsStrip(other.tgcIsStrip());
    setTgcChannel(other.tgcChannel());
  }
}

std::ostream& MuonFixedLongId::dump(std::ostream& os) const {
  os << technology() << ": stnEta " << eta() << ", stnPhi " << phi() << " " 
     << technologyString() << ": " << stationNameString() ;
  if( is_mdt() ){
     os << ", mdtMultiLayer  " << mdtMultilayer() << ", mdtTubeLayer "
	<< mdtTubeLayer() << ", mdtTube " << mdtTube() ; 
  } else if( is_rpc() ) { 
    os << ", rpcDoubletR " << rpcDoubletR() << ", rpcDoubletZ "
       << rpcDoubletZ() << ", rpcDoubletPhi " << rpcDoubletPhi() << ", rpcGasGap " << rpcGasGap() << ", rpcMeasuresPhi "
       << rpcMeasuresPhi() << ", rpcStrip " << rpcStrip() ; 
  } else if( is_tgc() ) { 
    os << ", tgcGasGap " << tgcGasGap() << ", tgcIsStrip " 
       <<  tgcIsStrip() << ", tgcChannel " << tgcChannel() ; 
  } else if( is_csc() ) {
    os << ", cscChamberLayer " << cscChamberLayer() << ", cscWireLayer "
       << cscWireLayer()  << ", cscMeasuresPhi " << cscMeasuresPhi() << ", cscStrip " << cscStrip() ; 
  } else if ( is_mmg() ) {
    os << ", mmgMultilayer " << mmgMultilayer() << ", mmgGasGap " << mmgGasGap() << ", mmgStrip " << mmgStrip();
  } else if ( is_stg() ) {
    os << ", stgMultilayer " << stgMultilayer() << ", stgGasGap " << stgGasGap()
      << ", stgChannelType " << stgChannelType() << ", stgChannel " << stgChannel();
  } else
    os << "Invalid MuonFixedLongId" ;
  
  return os;
}

const char MuonFixedLongId::kStationNameStrings[MuonFixedLongId::kNumberOfStationNames][4] = { 
     // 1      2      3      4      5      6      7      8      9     10     11     12     13
     "BIL", "BIS", "BML", "BMS", "BOL", "BOS", "BEE", "BIR", "BMF", "BOF", "BOG", "BME", "BIM",
     //14     15      16    17     18     19     20     21     22     23     24
     "EIC", "EIL", "EEL", "EES", "EMC", "EML", "EMS", "EOC", "EOL", "EOS", "EIS",
     //25     26     27     28     29     30     31     32     33     34     35 
     "T1F", "T1E", "T2F", "T2E", "T3F", "T3E", "T4F", "T4E", "CSS", "CSL", "BMG",
     //36     37     38     39
     "MMS", "MML", "STS", "STL"
};

const char MuonFixedLongId::kTechnologyStrings[MuonFixedLongId::kNumberOfTechnologies][4] = {
  "MDT", "CSC", "TGC", "RPC", "MMG", "STG"
};

} // end of namespace MuonCalib
