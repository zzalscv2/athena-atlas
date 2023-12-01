/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TGC_PrepDataToxAOD.h"

#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "StoreGate/ReadHandle.h"

#include <bitset>

TGC_PrepDataToxAOD::TGC_PrepDataToxAOD(const std::string &name, ISvcLocator *pSvcLocator) :
  MuonPrepDataToxAOD(name,pSvcLocator,"TGC_MeasurementsAllBCs","TGC_SDO") {
}

// Execute method:
StatusCode TGC_PrepDataToxAOD::execute()
{
  SG::ReadHandle<Muon::TgcPrepDataContainer> tgcPrds(m_inputContainerName);
  SG::ReadHandle<MuonSimDataCollection> tgcSdos(m_sdoContainerName);
  if(!tgcPrds.isPresent()){
    ATH_MSG_DEBUG("No "<<m_inputContainerName.key()<<" collection");
    return StatusCode::SUCCESS;
  }
  if(!tgcPrds.isValid()){
    ATH_MSG_WARNING(m_inputContainerName.key()<<" not valid");
    return StatusCode::FAILURE;
  }
  SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> trackMeasValCont(m_trackMeasVal);
  ATH_CHECK(trackMeasValCont.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));
  if( !buildCollections(tgcPrds.cptr(),tgcSdos.cptr(),trackMeasValCont.ptr()) ) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

StatusCode TGC_PrepDataToxAOD::initialize()
{
  ATH_CHECK(MuonPrepDataToxAOD::initialize());
  ATH_CHECK(m_clusterCreator.retrieve());
  ATH_CHECK(m_pullCalculator.retrieve());
  return StatusCode::SUCCESS;
}

void TGC_PrepDataToxAOD::addPRD_TechnologyInformation( xAOD::TrackMeasurementValidation& xprd, const Muon::TgcPrepData& prd ) const {
  xprd.auxdata<uint16_t>("bctag") = prd.getBcBitMap();
  xprd.auxdata<uint16_t>("measPhi") = m_idHelperSvc->measuresPhi(prd.identify());
  xprd.auxdata<uint16_t>("muonClusterSize") = (uint16_t)prd.rdoList().size();
  ATH_MSG_DEBUG(m_idHelperSvc->toString(prd.identify()) << "bctag "
                << ((prd.getBcBitMap()&Muon::TgcPrepData::BCBIT_PREVIOUS)==Muon::TgcPrepData::BCBIT_PREVIOUS) << " "
                << ((prd.getBcBitMap()&Muon::TgcPrepData::BCBIT_CURRENT)==Muon::TgcPrepData::BCBIT_CURRENT) << " "
                << ((prd.getBcBitMap()&Muon::TgcPrepData::BCBIT_NEXT)==Muon::TgcPrepData::BCBIT_NEXT)
                << " xaod " << xprd.auxdata<uint16_t>("bctag"));
}

void TGC_PrepDataToxAOD::addSDO_TechnologyInformation( xAOD::TrackMeasurementValidation& xprd, const Muon::TgcPrepData& prd, const MuonSimData* sdo ) const {

  float& residual = xprd.auxdata<float>("residual");
  float& pull = xprd.auxdata<float>("pull");
  residual = invalid_value;
  pull = invalid_value;
  if( !sdo ) return;

  Amg::Vector3D gpos = sdo->globalPosition();
  if( gpos.mag() > 1000 ){
    Amg::Transform3D gToL = prd.detectorElement()->transform(prd.identify()).inverse();
    Amg::Vector3D lpos = gToL*gpos;
    Amg::Vector3D ldir = gToL.linear()*gpos.unit();
    Amg::Vector3D lposCor = lpos - lpos.z()/ldir.z()*ldir;
    ATH_MSG_DEBUG("lpos  " << lpos.z() << " cor " << lposCor.z() );
    Amg::Vector3D gposCor = gToL.inverse()*lposCor;
    const Muon::MuonClusterOnTrack* clus = m_clusterCreator->createRIO_OnTrack(prd,gposCor);
    if( clus ) {
      const Trk::TrackParameters* pars =
        clus->associatedSurface()
          .createUniqueTrackParameters(gposCor, gposCor.unit(), 1., std::nullopt)
          .release();
      if (pars) {
        std::optional<Trk::ResidualPull> resPull = m_pullCalculator->residualPull( clus, pars, Trk::ResidualPull::HitOnly );
        if( resPull && !resPull->residual().empty() ) {
          residual = resPull->residual().front();
          pull     = resPull->pull().front();
        }else ATH_MSG_DEBUG("Failed to calculate residual");
      } else
        ATH_MSG_DEBUG("Failed to create track parameters");
      delete pars;
    }else ATH_MSG_DEBUG("Failed to create cluster on track");
    delete clus;
  }
  ATH_MSG_DEBUG("residual  " << residual << " pull " << pull );
}
