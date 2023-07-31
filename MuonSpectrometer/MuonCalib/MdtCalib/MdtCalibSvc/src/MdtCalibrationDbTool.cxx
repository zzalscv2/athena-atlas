/*
  Copyright (C) 2019-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibSvc/MdtCalibrationDbTool.h"

#include "MdtCalibSvc/MdtCalibrationRegionSvc.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/BFieldCorFunc.h"
#include "MdtCalibData/WireSagCorFunc.h"
#include "MdtCalibData/MdtSlewCorFuncHardcoded.h"
#include "MdtCalibData/CalibFunc.h"
#include <atomic>

MdtCalibrationDbTool::MdtCalibrationDbTool(const std::string& type, const std::string &name, const IInterface* parent)
  : base_class(type, name, parent),
    m_regionSvc("MdtCalibrationRegionSvc", name)
{
  declareProperty("AccessTubeConstants", m_getTubeConstants = true,
		  "configure the Tool to retrieve the constants per tube (t0)");
  declareProperty("AccessCorrections", m_getCorrections = true,
		  "configure the Tool to retrieve the parameters of the correction functions");
  declareProperty("CreateBFieldFunctions", m_create_b_field_function = false,
		  "If set to true, the B-field correction functions are initialized for each rt-relation that is loaded.");
  declareProperty("CreateWireSagFunctions", m_createWireSagFunction = false,
		  "If set to true, the wire sag correction functions are initialized for each rt-relation that is loaded.");
  declareProperty("CreateSlewingFunctions", m_createSlewingFunction = false, 
		  "If set to true, the slewing correction functions are initialized for each rt-relation that is loaded.");
}

StatusCode MdtCalibrationDbTool::initialize() {

  if ( m_regionSvc.retrieve().isFailure() ) {
    ATH_MSG_ERROR( "Failed to retrieve MdtCalibrationRegionSvc" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_readKeyRt.initialize());
  ATH_CHECK(m_readKeyTube.initialize());
  ATH_CHECK(m_readKeyCor.initialize (m_createSlewingFunction || m_createWireSagFunction || m_create_b_field_function));
  
  if (!m_wasConfigured){
    ATH_MSG_WARNING( "This tool is too complicated to rely on defaults. Potential configuration issue." );
  }

  return StatusCode::SUCCESS;
}  //end MdtCalibrationDbTool::initialize

MuonCalib::MdtFullCalibData MdtCalibrationDbTool::getCalibration( const Identifier &idt ) const {

  Identifier id = m_idHelperSvc->mdtIdHelper().elementID( idt );

  IdentifierHash mlHash;     //hash for ML (needed when using ML-RT functions)
  m_idHelperSvc->mdtIdHelper().get_detectorElement_hash( id, mlHash );   //hash for the ML 

  IdentifierHash chamberHash;
  IdContext idCont = m_idHelperSvc->mdtIdHelper().module_context();
  m_idHelperSvc->mdtIdHelper().get_hash( id, chamberHash, &idCont );

  return getCalibration( chamberHash, mlHash );
}

MuonCalib::MdtFullCalibData MdtCalibrationDbTool::getCalibration( const IdentifierHash &chamberHash, const IdentifierHash &mlHash ) const {
  const MuonCalib::MdtRtRelation         *rt   = nullptr;
  const MuonCalib::MdtTubeCalibContainer *tube = nullptr;
  const MuonCalib::MdtCorFuncSet         *cor  = nullptr;
  
  IdentifierHash tubeHash = chamberHash;
  IdentifierHash rtHash;
  if( m_regionSvc->RegionType()==ONEPERMULTILAYER ) { //if using ML-RTs  
    rtHash = m_regionSvc->getRegionHash(mlHash);
  } else {
    rtHash = m_regionSvc->getRegionHash(chamberHash);
  }
  IdentifierHash corHash  = rtHash;   //correction functions done in same regions as RTs

  if( rtHash.is_valid() ) {
    // find rt relation
    rt = getRtCalibration( rtHash );
    if ( !rt ) { 
      ATH_MSG_WARNING( "No valid MdtRtRelation found " );
    } 
  } else {
    ATH_MSG_WARNING( "--- INVALID RT HASH, cannot access rt calibration constants " );
  }
   
  // find correction functions 
  if( m_getCorrections && corHash.is_valid()) {
    cor = getCorFunctions( corHash );
  }

  // find t0's
  if( m_getTubeConstants && tubeHash.is_valid() ) {
      tube = getTubeCalibContainer( tubeHash );
      if( !tube ){
        ATH_MSG_WARNING( "Not valid MdtTubeCalibContainer found " );
      }
  }
  
  return MuonCalib::MdtFullCalibData( cor, rt, tube );
}

const MuonCalib::MdtTubeCalibContainer* MdtCalibrationDbTool::getTubeCalibContainer( const Identifier &idt ) const {
  Identifier id = m_idHelperSvc->mdtIdHelper().elementID( idt );

  IdentifierHash hash;
  IdContext idCont = m_idHelperSvc->mdtIdHelper().module_context();
  m_idHelperSvc->mdtIdHelper().get_hash( id, hash, &idCont );
  
  return getTubeCalibContainer( hash );
}

const MuonCalib::MdtTubeCalibContainer* MdtCalibrationDbTool::getTubeCalibContainer( const IdentifierHash &hash ) const {
  if ( !hash.is_valid() ) {
    ATH_MSG_WARNING( "cannot get tube, invalid hash"  );
    return nullptr;
  }
  SG::ReadCondHandle<MdtTubeCalibContainerCollection> readHandleTube{ m_readKeyTube };
  const MdtTubeCalibContainerCollection* readCdoTube{*readHandleTube};
  if ( readCdoTube==nullptr ) {
    ATH_MSG_ERROR("readCdoTube==nullptr");
    return nullptr;
  }
  if ( hash < readCdoTube->size() ) {
    return (*readCdoTube)[ hash ];
  }
  ATH_MSG_WARNING( "cannot get tube, region hash out of range"  );
  return nullptr;
}

const MuonCalib::MdtRtRelation* MdtCalibrationDbTool::getRtCalibration( const Identifier &idt ) const {
  Identifier id = m_idHelperSvc->mdtIdHelper().elementID( idt );
  IdentifierHash hash;

  if( m_regionSvc->RegionType()==ONEPERMULTILAYER ) {
    m_idHelperSvc->mdtIdHelper().get_detectorElement_hash( id, hash );   //hash for the ML 
  } else {
    IdContext idCont = m_idHelperSvc->mdtIdHelper().module_context();
    m_idHelperSvc->mdtIdHelper().get_hash( id, hash, &idCont );          //hash for the chamber
  }
  hash = m_regionSvc->getRegionHash(hash);
  return getRtCalibration( hash );
}

const MuonCalib::MdtRtRelation* MdtCalibrationDbTool::getRtCalibration( const IdentifierHash &hash ) const {
  if ( !hash.is_valid() ) {
    ATH_MSG_WARNING( "cannot get rt, invalid hash"  );
    return nullptr;
  }
  SG::ReadCondHandle<MdtRtRelationCollection> readHandleRt{ m_readKeyRt };
  const MdtRtRelationCollection* readCdoRt{*readHandleRt};
  if ( readCdoRt==nullptr ) {
    ATH_MSG_ERROR("readCdoRt==nullptr");
    return nullptr;
  }
  // Get the RT using the hash as an index
  if( hash < readCdoRt->size() ) {
    return (*readCdoRt)[ hash ];
  }
  ATH_MSG_WARNING( "cannot get RT, region hash out of range"  );
  return nullptr;
}

const MuonCalib::MdtCorFuncSet* MdtCalibrationDbTool::getCorFunctions( const Identifier &idt ) const {
  Identifier id = m_idHelperSvc->mdtIdHelper().elementID( idt );
  IdentifierHash hash;

  if( m_regionSvc->RegionType()==ONEPERMULTILAYER ) {
    m_idHelperSvc->mdtIdHelper().get_detectorElement_hash( id, hash );   //hash for the ML 
  } else {
    IdContext idCont = m_idHelperSvc->mdtIdHelper().module_context();
    m_idHelperSvc->mdtIdHelper().get_hash( id, hash, &idCont );          //hash for the chamber
  }
  hash = m_regionSvc->getRegionHash(hash);
  return getCorFunctions( hash );
}

const MuonCalib::MdtCorFuncSet* MdtCalibrationDbTool::getCorFunctions( const IdentifierHash &hash ) const {
  if ( !hash.is_valid() ){
    ATH_MSG_WARNING( "cannot get cor, invalid hash"  );
    return nullptr;
  }
  //in this case the loadRt() was intended to keep m_corData nullptr and getCorFunctions() returned 0 here so
  if( !m_createSlewingFunction && !m_createWireSagFunction && !m_create_b_field_function ){
    return nullptr;
  }
  SG::ReadCondHandle<MdtCorFuncSetCollection> readHandleCor{ m_readKeyCor };
  const MdtCorFuncSetCollection* readCdoCor{*readHandleCor};
  if ( readCdoCor==nullptr ) {
    ATH_MSG_ERROR("readCdoCor==nullptr");
    return nullptr;
  }
  if( hash < readCdoCor->size() ) {
    return (*readCdoCor)[ hash ];
  }
  ATH_MSG_WARNING( "cannot get cor, region hash out of range"  );
  return nullptr;
}

void MdtCalibrationDbTool::initialize_B_correction(MuonCalib::MdtCorFuncSet *funcSet,
                                                  const MuonCalib::MdtRtRelation *rt_rel) {
  if (rt_rel==nullptr) {
    funcSet->setBField(nullptr);
    return;
  }
  ATH_MSG_VERBOSE( "initialize_B_correction..." );
  std::vector<double> corr_params(2);
  corr_params[0] = 3080.0; // high voltage (not correct for sMDT which use 2730V!)
  corr_params[1] = 0.11;   // epsilon parameter
  funcSet->setBField(std::make_unique<MuonCalib::BFieldCorFunc>("medium", corr_params, rt_rel->rt()));
}

void MdtCalibrationDbTool::initializeSagCorrection(MuonCalib::MdtCorFuncSet *funcSet) {
  ATH_MSG_VERBOSE( "initializeSagCorrection..." );
  std::vector<double> corr_params(0);
  funcSet->wireSag(std::make_unique<MuonCalib::WireSagCorFunc>(corr_params));
}
