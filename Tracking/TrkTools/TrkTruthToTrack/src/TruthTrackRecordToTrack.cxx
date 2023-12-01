/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTruthToTrack/TruthTrackRecordToTrack.h"

#include <cmath>
#include <memory>

#include "GaudiKernel/IPartPropSvc.h"

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"

#include "HepPDT/ParticleDataTable.hh"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrackRecord/TrackRecord.h"


//================================================================
Trk::TruthTrackRecordToTrack::TruthTrackRecordToTrack(const std::string& type, const std::string& name,
                                            const IInterface* parent)
  : AthAlgTool(type,name,parent),
    m_particleDataTable(nullptr),
    m_extrapolator("Trk::Extrapolator/AtlasExtrapolator")
{
  declareInterface<ITruthToTrack>(this);

  declareProperty("Extrapolator", m_extrapolator);
  declareProperty("TrackRecordKey",m_reccollkey="CosmicPerigee");
}

//================================================================
StatusCode Trk::TruthTrackRecordToTrack::initialize() {

  // get the Particle Properties Service
  IPartPropSvc* partPropSvc = nullptr;
  StatusCode sc =  service("PartPropSvc", partPropSvc, true);
  if (sc.isFailure()) {
    ATH_MSG_ERROR ("Could not initialize Particle Properties Service");
    return StatusCode::FAILURE;
  }
  m_particleDataTable = partPropSvc->PDT();

  if ( m_extrapolator.retrieve().isFailure() ) {
    ATH_MSG_FATAL ("Failed to retrieve tool " << m_extrapolator );
    return StatusCode::FAILURE;
  }
    ATH_MSG_INFO("Retrieved tool " << m_extrapolator);


  ATH_CHECK( m_reccollkey.initialize() );

  return StatusCode::SUCCESS;
}

//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makeProdVertexParameters(HepMC::ConstGenParticlePtr part) const {

  if (part == nullptr || m_particleDataTable==nullptr) return nullptr;

  Trk::TrackParameters *result = nullptr;
  Amg::Vector3D prodVertexVector;
  prodVertexVector.setZero();
  Amg::Vector3D globalPos;
  Amg::Vector3D globalMom;
  int id=0;
  double charge = 0.0;
  const HepPDT::ParticleData* pd = nullptr;


  SG::ReadHandle<TrackRecordCollection> recordCollection(m_reccollkey);

  if (recordCollection.isValid()) {
    ATH_MSG_ERROR ("Could not get track record!");
    return nullptr;
  }
  ATH_MSG_DEBUG("reading from track record, size=" << recordCollection->size());

  if (recordCollection->empty()) ATH_MSG_WARNING ("action required but record size is 0");

  const int barcodepart = HepMC::barcode(part);
  for (const auto & record : *recordCollection){

    if ( record.GetBarCode() != barcodepart ) continue;

      id = record.GetPDGCode();
      pd = m_particleDataTable->particle(std::abs(id));
      if (!pd) {
        ATH_MSG_WARNING ("Found particle with problematic PDG ID " << part << " , " << id);
        continue;
      }

      CLHEP::Hep3Vector tv = record.GetPosition();
      prodVertexVector = Amg::Vector3D(tv.x(),tv.y(),tv.z());
      globalPos = prodVertexVector;

      Amg::Vector3D hv2(record.GetMomentum().x(), record.GetMomentum().y(),
                            record.GetMomentum().z());
      globalMom = hv2;

      ATH_MSG_DEBUG("Found particle " << part << ", momentum " << hv2 << " production " << globalPos);


  }   // loop over G4 records

  if (pd) {
    charge = (id>0) ? pd->charge() : -pd->charge();

    Amg::Translation3D prodSurfaceCentre( prodVertexVector.x(),
					  prodVertexVector.y(),
					  prodVertexVector.z() );

    Amg::Transform3D tmpTransf =  prodSurfaceCentre *  Amg::RotationMatrix3D::Identity();

    Trk::PlaneSurface planeSurface(tmpTransf, 5., 5. );
    result = new Trk::AtaPlane(globalPos, globalMom, charge, planeSurface);

  } else {
    ATH_MSG_WARNING ("Could not get particle data for particle ID="<<id);
  }
  return result;
}



//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makeProdVertexParameters(const xAOD::TruthParticle* part) const {

  if (part == nullptr || m_particleDataTable==nullptr) return nullptr;

  Trk::TrackParameters *result = nullptr;
  Amg::Vector3D prodVertexVector;
  Amg::Vector3D globalPos;
  Amg::Vector3D globalMom;
  int id=0;
  double charge = 0.0;
  const HepPDT::ParticleData* pd = nullptr;

   SG::ReadHandle<TrackRecordCollection> recordCollection(m_reccollkey);

  if (recordCollection.isValid()) {
    ATH_MSG_ERROR ("Could not get track record!");
    return nullptr;
  }

  ATH_MSG_DEBUG("reading from track record, size=" << recordCollection->size());

  if (recordCollection->empty()) ATH_MSG_WARNING ("action required but record size is 0");

  for (const auto & record : *recordCollection){

    if ( record.GetBarCode() == part->barcode() ) {

      id = record.GetPDGCode();
      pd = m_particleDataTable->particle(std::abs(id));
      if (!pd) {
        ATH_MSG_WARNING ("found particle with problematic PDG ID" << part << " , " << id);
        continue;
      }

      CLHEP::Hep3Vector  tv = record.GetPosition();
      prodVertexVector = Amg::Vector3D(tv.x(),tv.y(),tv.z());
      globalPos = prodVertexVector;

      Amg::Vector3D hv2(record.GetMomentum().x(), record.GetMomentum().y(), record.GetMomentum().z());
      globalMom = hv2;

      ATH_MSG_DEBUG("found particle " << part << ", momentum " << hv2 << " production " << globalPos);


    }
  }   // loop over G4 records

  if (pd) {
    charge = (id>0) ? pd->charge() : -pd->charge();

    Amg::Translation3D prodSurfaceCentre( prodVertexVector.x(),
					  prodVertexVector.y(),
					  prodVertexVector.z() );

    Amg::Transform3D tmpTransf =  prodSurfaceCentre *  Amg::RotationMatrix3D::Identity();

    Trk::PlaneSurface planeSurface(tmpTransf, 5., 5. );
    result = new Trk::AtaPlane(globalPos, globalMom, charge, planeSurface);

  } else {
    ATH_MSG_WARNING ("Could not get particle data for particle ID="<<id);
  }
  return result;
}



//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makePerigeeParameters(HepMC::ConstGenParticlePtr part) const {
  const Trk::TrackParameters* generatedTrackPerigee = nullptr;

  if(part && part->production_vertex() && m_particleDataTable && m_extrapolator) {

    MsgStream log(msgSvc(), name());

    std::unique_ptr<const Trk::TrackParameters> productionVertexTrackParams( makeProdVertexParameters(part) );
    if(productionVertexTrackParams) {

      // Extrapolate the TrackParameters object to the perigee. Direct extrapolation,
      // no material effects.
      generatedTrackPerigee =
        m_extrapolator->extrapolateDirectly(Gaudi::Hive::currentContext(),
                                            *productionVertexTrackParams,
                                            Trk::PerigeeSurface(),
                                            Trk::anyDirection,
                                            false,
                                            Trk::nonInteracting).release();
    }
  }

  return generatedTrackPerigee;
}

//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makePerigeeParameters(const xAOD::TruthParticle* part) const {
  const Trk::TrackParameters* generatedTrackPerigee = nullptr;

  if(part && part->hasProdVtx() && m_particleDataTable && m_extrapolator) {

    MsgStream log(msgSvc(), name());

    std::unique_ptr<const Trk::TrackParameters> productionVertexTrackParams( makeProdVertexParameters(part) );
    if(productionVertexTrackParams) {

      // Extrapolate the TrackParameters object to the perigee. Direct extrapolation,
      // no material effects.
      generatedTrackPerigee =
        m_extrapolator->extrapolateDirectly(Gaudi::Hive::currentContext(),
                                            *productionVertexTrackParams,
                                            Trk::PerigeeSurface(),
                                            Trk::anyDirection,
                                            false,
                                            Trk::nonInteracting).release();
    }
  }

  return generatedTrackPerigee;
}
//================================================================
