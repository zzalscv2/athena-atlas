/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LArVolumeBuilder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Calo
#include "LArTrackingGeometry/LArVolumeBuilder.h"
// LAr
#include "LArReadoutGeometry/LArDetectorManager.h"
// CaloDepth
#include "CaloDetDescr/CaloDepthTool.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
// GeoModel
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoPVConstLink.h"
#include "GeoModelUtilities/StoredPhysVol.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"
// Trk
#include "TrkDetDescrInterfaces/ITrackingVolumeHelper.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkDetDescrGeoModelCnv/GeoMaterialConverter.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/BinnedMaterial.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/AlignableTrackingVolume.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkGeometrySurfaces/SlidingCylinderSurface.h"
#include "TrkGeometrySurfaces/SlidingDiscSurface.h"
#include "GaudiKernel/SystemOfUnits.h"

using Gaudi::Units::mm;

// constructor
LAr::LArVolumeBuilder::LArVolumeBuilder(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p),
  m_lArMgrLocation("LArMgr"),
  m_lArTrackingVolumeHelper("Trk::TrackingVolumeHelper/LArTrackingVolumeHelper"),
  m_trackingVolumeCreator("Trk::CylinderVolumeCreator/TrackingVolumeCreator"),
  m_lArBarrelEnvelope(25.*mm),
  m_lArEndcapEnvelope(25.*mm),
  m_useCaloSurfBuilder(true),
  m_lArLayersPerRegion(1),
  m_useCaloTrackingGeometryBounds(true),
  m_calosurf("CaloSurfaceBuilder"),
  m_scale_HECmaterial(1.1)
{
  declareInterface<Trk::ICaloTrackingVolumeBuilder>(this);
  // declare the properties via Python
  declareProperty("LArDetManagerLocation",             m_lArMgrLocation);
  declareProperty("TrackingVolumeCreator",            m_trackingVolumeCreator);
  declareProperty("TrackingVolumeHelper",              m_lArTrackingVolumeHelper);
  // endcap
  declareProperty("BarrelEnvelopeCover",               m_lArBarrelEnvelope);
  declareProperty("EndcapEnvelopeCover",               m_lArEndcapEnvelope);

  declareProperty("UseCaloSurfBuilder",                m_useCaloSurfBuilder);
  declareProperty("LayersPerRegion",                   m_lArLayersPerRegion);
  declareProperty("UseCaloTrackingGeometryBounds",     m_useCaloTrackingGeometryBounds);
  declareProperty("CaloSurfaceBuilder",                m_calosurf);
  declareProperty("ScaleFactor_HECmaterial",           m_scale_HECmaterial);

}

// destructor
LAr::LArVolumeBuilder::~ LArVolumeBuilder()
= default;


// Athena standard methods
// initialize
StatusCode LAr::LArVolumeBuilder::initialize()
{
  
  // Retrieve the tracking volume helper   -------------------------------------------------    
  if (m_lArTrackingVolumeHelper.retrieve().isFailure())
    {
      ATH_MSG_FATAL( "Failed to retrieve tool " << m_lArTrackingVolumeHelper );
      return StatusCode::FAILURE;
    } else
    ATH_MSG_DEBUG( "Retrieved tool " << m_lArTrackingVolumeHelper );

  // Retrieve the volume creator
  if (m_trackingVolumeCreator.retrieve().isFailure()){
    ATH_MSG_FATAL( "Failed to retrieve tool " << m_trackingVolumeCreator );
    return StatusCode::FAILURE;
  } else
    ATH_MSG_DEBUG( "Retrieved tool " << m_trackingVolumeCreator );
  
  if(m_useCaloSurfBuilder){
    if(m_calosurf.retrieve().isFailure())
      {
        ATH_MSG_FATAL( "Failed to retrieve tool " << m_calosurf );
        return StatusCode::FAILURE;
      } else
      ATH_MSG_DEBUG( "Retrieved tool " << m_calosurf );
  }
  
  ATH_MSG_DEBUG( name() << " initialize() successful" );
  return StatusCode::SUCCESS;
}

// finalize
StatusCode LAr::LArVolumeBuilder::finalize()
{
  ATH_MSG_DEBUG( "finalize() successful" );

  // empty the material garbage 
  for ( const auto *mat : m_materialGarbage ) {
    delete mat;
  }

  return StatusCode::SUCCESS;
}

std::vector<Trk::TrackingVolume*>*
LAr::LArVolumeBuilder::trackingVolumes(const CaloDetDescrManager& caloDDM) const
{
  // the return vector
  std::vector<Trk::TrackingVolume*>* lArTrackingVolumes = new std::vector<Trk::TrackingVolume*>;
  // the converter helpers
  Trk::GeoShapeConverter    geoShapeToVolumeBounds;
  Trk::GeoMaterialConverter geoMaterialToMaterialProperties;
  
  Trk::Material dummyMaterial;

  /** Helper to collect local garbage and transfer it into global garbage bin on return */
  struct GarbageCollector {
    GarbageCollector(MaterialGarbage& globalGarbage) : globalBin(globalGarbage) {}
    ~GarbageCollector() {
      static std::mutex mutex;
      std::scoped_lock lock(mutex);
      globalBin.merge(bin);
    }
    MaterialGarbage bin;        ///!< our local trash
    MaterialGarbage& globalBin; ///!< global trash
  };

  // Local garbage collector
  GarbageCollector gc(m_materialGarbage);

  // get LAr Detector Description Manager
  const LArDetectorManager* lArMgr=nullptr;
  if (detStore()->retrieve(lArMgr, m_lArMgrLocation).isFailure()) {
    ATH_MSG_FATAL( "Could not get LArDetectorManager! Calo TrackingGeometry will not be built");
  }

  // out of couriosity
  unsigned int numTreeTops =  lArMgr->getNumTreeTops();
  ATH_MSG_DEBUG( "Retrieved " << numTreeTops << " tree tops from the LArDetDescrManager. " );
  
  for (unsigned int itreetop = 0; itreetop<numTreeTops; ++itreetop){
    PVConstLink currentVPhysVolLink   = lArMgr->getTreeTop(itreetop);
    const GeoLogVol* currentLogVol = currentVPhysVolLink->getLogVol();
    
    unsigned int currentChilds = currentVPhysVolLink->getNChildVols();
    
    ATH_MSG_DEBUG( "Processing " << currentLogVol->getName() << "... has " 
		   << currentChilds << " childs, position " << currentVPhysVolLink->getX().translation());
    //printInfo( currentVPhysVolLink,2);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////
  // THE BARREL SECTION
  ATH_MSG_DEBUG( "============ Barrel Section ======================" );
  
  Trk::TrackingVolume* solenoid             = nullptr;
  Trk::TrackingVolume* solenoidLArBarrelGap = nullptr;
  Trk::TrackingVolume* lArBarrelPresampler  = nullptr;
  Trk::TrackingVolume* lArBarrel            = nullptr;
  
  Trk::CylinderVolumeBounds* solenoidBounds             = nullptr;
  Trk::CylinderVolumeBounds* solenoidLArBarrelGapBounds = nullptr;
  
  // dummy objects
  Trk::LayerArray* dummyLayers = nullptr;
  Trk::TrackingVolumeArray* dummyVolumes = nullptr;

  // default material definition
  Trk::Material solenoidMaterial = Trk::Material( 69.9, 811.5,  28.9, 13.8, 0.003); 
  const Trk::Material* lArBarrelPresamplerMaterial = new Trk::Material(130.,  634.4,  33.7, 15.4, 0.0017);          
  const Trk::Material* lArBarrelMaterial           = new Trk::Material( 26.2, 436.3,  65.4, 27.8, 0.0035);      

  gc.bin.insert(lArBarrelPresamplerMaterial);
  gc.bin.insert(lArBarrelMaterial);

  // load layer surfaces
  std::vector<std::pair<const Trk::Surface*, const Trk::Surface*>> entrySurf =
    m_calosurf->entrySurfaces(&caloDDM);
  std::vector<std::pair<const Trk::Surface*, const Trk::Surface*>> exitSurf =
    m_calosurf->exitSurfaces(&caloDDM);

  StoredPhysVol* storedPV = nullptr;

  // -> The BARREL Section ---------------------------------------------------------
  ATH_MSG_DEBUG( "Building Barrel ... " );
  
  Trk::CylinderVolumeBounds* lArBarrelPosBounds = nullptr;
  Trk::CylinderVolumeBounds* lArBarrelNegBounds = nullptr;
  
  if(detStore()->contains<StoredPhysVol>("EMB_POS"))
    {
      if(detStore()->retrieve(storedPV,"EMB_POS")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV EMB_POS" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArBarrelPosPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;

  //if (lArBarrelPosPhysVol) printInfo(lArBarrelPosPhysVol,2);
  
  if(detStore()->contains<StoredPhysVol>("EMB_NEG"))
    {
      if(detStore()->retrieve(storedPV,"EMB_NEG")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV EMB_NEG" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArBarrelNegPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  const GeoLogVol* lArBarrelPosLogVol = lArBarrelPosPhysVol ? lArBarrelPosPhysVol->getLogVol() : nullptr;
  const GeoLogVol* lArBarrelNegLogVol = lArBarrelNegPhysVol ? lArBarrelNegPhysVol->getLogVol() : nullptr;
  
  // get the material : needs averaging
  
  double lArBarrelHalflength              = 0.;
  std::vector<double> zBoundaries;
  
  // retrival worked out
  if (lArBarrelPosLogVol && lArBarrelNegLogVol){
    
    int poschilds = lArBarrelPosPhysVol->getNChildVols();
    int negchilds = lArBarrelNegPhysVol->getNChildVols();
    
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArBarrelPosPhysVol->getAbsoluteName()
		     << " (" << poschilds << " childs) ." );
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArBarrelNegPhysVol->getAbsoluteName()
		     << " (" << negchilds << " childs) ." );
    
    // and the shapes
    const GeoShape*    lArBarrelPosShape  = lArBarrelPosLogVol->getShape();
    const GeoShape*    lArBarrelNegShape  = lArBarrelNegLogVol->getShape();
        
    // dynamic cast to 'Tubs' shape
    const GeoPcon* lArBarrelPosPcon = dynamic_cast<const GeoPcon*>(lArBarrelPosShape);
    lArBarrelPosBounds = (lArBarrelPosPcon) ? Trk::GeoShapeConverter::convert(lArBarrelPosPcon, zBoundaries) : nullptr;
    const GeoPcon* lArBarrelNegPcon = dynamic_cast<const GeoPcon*>(lArBarrelNegShape);
    lArBarrelNegBounds = (lArBarrelNegPcon) ? Trk::GeoShapeConverter::convert(lArBarrelNegPcon, zBoundaries) : nullptr;
    
    if (lArBarrelPosBounds)
      ATH_MSG_VERBOSE( " -> Positive Barrel Bounds: " << *lArBarrelPosBounds );
    if (lArBarrelNegBounds)
      ATH_MSG_VERBOSE( " -> Negative Barrel Bounds: " << *lArBarrelNegBounds );
  
    
  }
  // conversion of PreSamplers done -> make one out of two
  if (lArBarrelPosBounds && lArBarrelNegBounds){
    
    // new dimensions
    double lArBarrelRmin = lArBarrelPosBounds->innerRadius();      
    double lArBarrelRmax = lArBarrelPosBounds->outerRadius();
    lArBarrelHalflength = zBoundaries[1];

    // create the static half-bounds
    Trk::CylinderVolumeBounds* lArBarrelBoundsPos = new Trk::CylinderVolumeBounds( lArBarrelRmin,
										   lArBarrelRmax,
										   0.5*lArBarrelHalflength);
    Trk::CylinderVolumeBounds* lArBarrelBoundsNeg = lArBarrelBoundsPos->clone();

    // position static half-volumes
    Amg::Vector3D lArBPos(0.,0.,0.5*lArBarrelHalflength);
    Amg::Vector3D lArBNeg(0.,0.,-0.5*lArBarrelHalflength);
    Amg::Transform3D* lArBPosTransform = new Amg::Transform3D(Amg::Translation3D(lArBPos));
    Amg::Transform3D* lArBNegTransform = new Amg::Transform3D(Amg::Translation3D(lArBNeg));
    
    // layer entry/exit    

    // binned material for LAr : steering in binEta
    std::vector<Trk::IdentifiedMaterial> matID;
    // layer material can be adjusted here
    int baseID = Trk::GeometrySignature(Trk::Calo)*1000;
    matID.emplace_back(lArBarrelMaterial,0);
    matID.emplace_back(lArBarrelMaterial,baseID+1);
    matID.emplace_back(lArBarrelMaterial,baseID+2);
    matID.emplace_back(lArBarrelMaterial,baseID+3);
    // scaling factors refer to avZ(avA) change
    matID.emplace_back(lArBarrelMaterial->scale(1.3),baseID+1);
    gc.bin.insert(matID.back().first);
    matID.emplace_back(lArBarrelMaterial->scale(1.3),baseID+2);
    gc.bin.insert(matID.back().first);
    matID.emplace_back(lArBarrelMaterial->scale(0.6),baseID+3);
    gc.bin.insert(matID.back().first);
    matID.emplace_back(lArBarrelMaterial->scale(0.7),baseID+3);
    gc.bin.insert(matID.back().first);
    matID.emplace_back(lArBarrelMaterial->scale(0.8),baseID+3);
    gc.bin.insert(matID.back().first);
    matID.emplace_back(lArBarrelMaterial->scale(0.9),baseID+3);
    gc.bin.insert(matID.back().first);
    matID.emplace_back(lArBarrelMaterial->scale(1.1),baseID+3);
    gc.bin.insert(matID.back().first);

    //
    Trk::BinUtility* bubn = new Trk::BinUtility(30,-1.5,0.,Trk::open,Trk::binEta);
    Trk::BinUtility* bubp = new Trk::BinUtility(30, 0.,1.5,Trk::open,Trk::binEta);
    // array of indices
    std::vector<std::vector<size_t> > indexP(bubn->bins());  
    std::vector<std::vector<size_t> > indexN;  
    // binned material for LAr : layer depth per eta bin
    std::vector< Trk::BinUtility*> layBUN(bubn->bins());
    std::vector< Trk::BinUtility*> layBUP(bubp->bins());
    // retrieve offset values (from surfaces on negative side)
    double r1 = entrySurf[CaloCell_ID::EMB1].second->bounds().r();     // first layer has no modulations
    double r2 = entrySurf[CaloCell_ID::EMB2].second->bounds().r();     // base value
    double r3 = entrySurf[CaloCell_ID::EMB3].second->bounds().r();     // base value

    std::vector<float> offset2{};
    const Trk::SlidingCylinderSurface* scyl2 = dynamic_cast<const Trk::SlidingCylinderSurface* > (entrySurf[CaloCell_ID::EMB2].second);
    if (scyl2) offset2 = scyl2->offset();
    std::vector<float> offset3{};
    const Trk::SlidingCylinderSurface* scyl3 = dynamic_cast<const Trk::SlidingCylinderSurface* > (entrySurf[CaloCell_ID::EMB3].second);
    if (scyl3) offset3 = scyl3->offset();
    // construct bin utilities symmetrically
    std::vector<float> steps;
    for (unsigned int i=0; i< bubn->bins(); i++) {
      steps.clear(); 
      steps.push_back(lArBarrelBoundsNeg->innerRadius());
      std::vector<size_t> indx; indx.clear();
      indx.push_back(0);
      steps.push_back(r1);
      indx.push_back( i<14 ? 1:4 );
      steps.push_back(r2 + offset2[i] );
      indx.push_back( i<14 ? 2:5 );
      steps.push_back(r3 + offset3[i]);
      // 3rd layer complicated
      if (i>19) indx.push_back(7);
      else if (i>17) indx.push_back(8);
      else if (i>15) indx.push_back(9);
      else if (i>13) indx.push_back(3);
      else if (i>11) indx.push_back(6);
      else if (i>9) indx.push_back(7);
      else if (i>7) indx.push_back(8);
      else if (i>5) indx.push_back(8);
      else if (i>4) indx.push_back(9);
      else indx.push_back(3);
      // end 3rd layer
      steps.push_back(lArBarrelBoundsNeg->outerRadius());
      Trk::BinUtility* rBU = new Trk::BinUtility(steps, Trk::open, Trk::binR);
      layBUN[i] = rBU;
      layBUP[bubp->bins()-1-i] = rBU->clone();
      indexN.push_back(indx);
      indexP[bubp->bins()-1-i] = std::vector<size_t>(indx);
    } 

    const Trk::BinnedMaterial* lArBarrelMaterialBinPos = new Trk::BinnedMaterial(lArBarrelMaterial,bubp,layBUP,indexP,matID);
    const Trk::BinnedMaterial* lArBarrelMaterialBinNeg = new Trk::BinnedMaterial(lArBarrelMaterial,bubn,layBUN,indexN,matID);

    Amg::Transform3D* align=nullptr;
    
    Trk::AlignableTrackingVolume* lArBarrelPos = new Trk::AlignableTrackingVolume(lArBPosTransform,align,
											lArBarrelBoundsPos,
											lArBarrelMaterialBinPos,
											1,
											"Calo::Detectors::LAr::BarrelPos");
    
    Trk::AlignableTrackingVolume* lArBarrelNeg = new Trk::AlignableTrackingVolume(lArBNegTransform,align,
											lArBarrelBoundsNeg,
											lArBarrelMaterialBinNeg,
											1,
											"Calo::Detectors::LAr::BarrelNeg");
    
    // glue barrel EM 
    std::vector<Trk::TrackingVolume*> volsB;
    volsB.push_back(lArBarrelNeg);
    volsB.push_back(lArBarrelPos);

    lArBarrel = m_trackingVolumeCreator->createContainerTrackingVolume(volsB,
								       dummyMaterial,
								       "Calo::Container::LAr::Barrel");
  }   
  // cleanup
  delete lArBarrelPosBounds; lArBarrelPosBounds = nullptr;
  delete lArBarrelNegBounds; lArBarrelNegBounds = nullptr;
       
  // (1) Build the Solenoid ------------------------------------------------------------
  ATH_MSG_DEBUG( "Building the Solenoid ... " );
  
  if(detStore()->contains<StoredPhysVol>("SOLENOID"))
    {
      if(detStore()->retrieve(storedPV,"SOLENOID")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV SOLENOID" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* solenoidPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  

  const GeoLogVol* solenoidLogVol = solenoidPhysVol ? solenoidPhysVol->getLogVol() : nullptr;
    
  // retrival worked out
  if (solenoidLogVol){
    int childs = solenoidPhysVol->getNChildVols();
    
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " 
		     << solenoidPhysVol->getAbsoluteName() << " (" << childs << " childs) ." );
    
    const GeoShape*    solenoidShape  = solenoidLogVol->getShape();
    // dynamic cast to 'Tubs' shape
    const GeoTubs* solenoidTubs = dynamic_cast<const GeoTubs*>(solenoidShape);
    solenoidBounds = new Trk::CylinderVolumeBounds(solenoidTubs->getRMin(),solenoidTubs->getRMax(),lArBarrelHalflength);
    // assing the material 
    const GeoMaterial* solenoidMaterialGM = solenoidLogVol->getMaterial();    
    if (solenoidMaterialGM) {
      solenoidMaterial =  Trk::GeoMaterialConverter::convert(solenoidMaterialGM);
    }
  }
  
  // conversion ok : create the volume
  if (solenoidBounds) {
    // output the bounds
    ATH_MSG_DEBUG( " -> Solenoid Bounds:      " << *solenoidBounds );

    solenoid = new Trk::TrackingVolume(nullptr,
				       solenoidBounds,
				       solenoidMaterial,
				       dummyLayers, dummyVolumes,
				       "Calo::Solenoid");
    
  }
  
  // (2) Build the Presampler ------------------------------------------------------------
  ATH_MSG_DEBUG( "Building Barrel Presampler ... " );
  
  Trk::CylinderVolumeBounds* lArBarrelPresamplerPosBounds = nullptr;
  Trk::CylinderVolumeBounds* lArBarrelPresamplerNegBounds = nullptr;
  
  if(detStore()->contains<StoredPhysVol>("PRESAMPLER_B_POS"))
    {
      if(detStore()->retrieve(storedPV,"PRESAMPLER_B_POS")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV PRESAMPLER_B_POS" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArBarrelPresamplerPosPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;

  
  
  if(detStore()->contains<StoredPhysVol>("PRESAMPLER_B_NEG"))
    {
      if(detStore()->retrieve(storedPV,"PRESAMPLER_B_NEG")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV PRESAMPLER_B_NEG" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArBarrelPresamplerNegPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;

  
  const GeoLogVol* lArBarrelPresamplerPosLogVol = lArBarrelPresamplerPosPhysVol ? lArBarrelPresamplerPosPhysVol->getLogVol() : nullptr;
  const GeoLogVol* lArBarrelPresamplerNegLogVol = lArBarrelPresamplerNegPhysVol ? lArBarrelPresamplerNegPhysVol->getLogVol() : nullptr;
 
  // retrival worked out
  if (lArBarrelPresamplerPosLogVol && lArBarrelPresamplerNegLogVol){
    
    int poschilds = lArBarrelPresamplerPosPhysVol->getNChildVols();
    int negchilds = lArBarrelPresamplerNegPhysVol->getNChildVols();
    
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume "
		     << lArBarrelPresamplerPosPhysVol->getAbsoluteName() << " (" << poschilds << " childs) ." );
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " 
		     << lArBarrelPresamplerNegPhysVol->getAbsoluteName() << " (" << negchilds << " childs) ." );
    
    Amg::Vector3D lArBPos(0.,0.,0.5*lArBarrelHalflength);
    Amg::Vector3D lArBNeg(0.,0.,-0.5*lArBarrelHalflength);
    Amg::Transform3D* lArPBPosTransform = new Amg::Transform3D(Amg::Translation3D(lArBPos));
    Amg::Transform3D* lArPBNegTransform = new Amg::Transform3D(Amg::Translation3D(lArBNeg));
    
    // and the shapes
    const GeoShape*    lArBarrelPresamplerPosShape  = lArBarrelPresamplerPosLogVol->getShape();
    
    // dynamic cast to 'Tubs' shape
    const GeoTubs* lArBarrelPresamplerPosTubs = dynamic_cast<const GeoTubs*>(lArBarrelPresamplerPosShape);
    lArBarrelPresamplerPosBounds = new Trk::CylinderVolumeBounds(lArBarrelPresamplerPosTubs->getRMin(),
								 lArBarrelPresamplerPosTubs->getRMax(),
								 0.5*lArBarrelHalflength);
    
     
    if (lArBarrelPresamplerPosBounds){
      lArBarrelPresamplerNegBounds = lArBarrelPresamplerPosBounds->clone();
      ATH_MSG_VERBOSE( " -> Positive Barrel Presampler Bounds: " 
		       << *lArBarrelPresamplerPosBounds );
    }
    if (lArBarrelPresamplerNegBounds){
      ATH_MSG_VERBOSE( " -> Negative Barrel Presampler Bounds: "
		       << *lArBarrelPresamplerNegBounds );
    }   
    
    // material needs averaging, don't use pure Ar

    Amg::Transform3D* align=nullptr;

    // trivial binning
    std::vector<float> bpsteps{float(lArBarrelPresamplerPosBounds->innerRadius()),
	float(lArBarrelPresamplerPosBounds->outerRadius())};
    Trk::BinUtility* rBU = new Trk::BinUtility(bpsteps, Trk::open, Trk::binR);
    Trk::BinUtility* rBUc = rBU->clone();

    std::vector<size_t> dummylay (1,0);
    // binned material for Presampler : 
    std::vector<Trk::IdentifiedMaterial> matBP;
    // layer material can be adjusted here
    int baseID = Trk::GeometrySignature(Trk::Calo)*1000;
    matBP.emplace_back(lArBarrelPresamplerMaterial,baseID);

    const Trk::BinnedMaterial* lArBarrelPresamplerMaterialBinPos = new Trk::BinnedMaterial(lArBarrelPresamplerMaterial,rBU,dummylay,matBP);
    const Trk::BinnedMaterial* lArBarrelPresamplerMaterialBinNeg = new Trk::BinnedMaterial(lArBarrelPresamplerMaterial,rBUc,dummylay,matBP);
      
    Trk::AlignableTrackingVolume* lArBarrelPresamplerPos = new Trk::AlignableTrackingVolume(lArPBPosTransform, align,
											    lArBarrelPresamplerPosBounds,
											    lArBarrelPresamplerMaterialBinPos,
											    0,
											    "Calo::Detectors::LAr::BarrelPresamplerPos");
    
    Trk::AlignableTrackingVolume* lArBarrelPresamplerNeg = new Trk::AlignableTrackingVolume(lArPBNegTransform, align,
											    lArBarrelPresamplerNegBounds,
											    lArBarrelPresamplerMaterialBinNeg,
											    0,
											    "Calo::Detectors::LAr::BarrelPresamplerNeg");
      
    // glue barrel presampler 
    std::vector<Trk::TrackingVolume*> volsBP;
    volsBP.push_back(lArBarrelPresamplerNeg);
    volsBP.push_back(lArBarrelPresamplerPos);

    lArBarrelPresampler = m_trackingVolumeCreator->createContainerTrackingVolume(volsBP,
										 dummyMaterial,
										 "Calo::Container::LAr::BarrelPresampler");
  }    
  
  // (3) Build the solenoid gap ------------------------------------------------------------

  if (solenoidBounds && lArBarrelPresamplerPosBounds) {
    solenoidLArBarrelGapBounds =  new Trk::CylinderVolumeBounds(solenoidBounds->outerRadius(),
                                                                lArBarrelPresamplerPosBounds->innerRadius(),
                                                                lArBarrelHalflength); 
   
    //Trk::MaterialProperties solenoidGapMaterial = Trk::MaterialProperties(1., 93.9/0.5, 0.0028*pow(0.5,3), 39.);
    // Trk::Material solenoidGapMaterial= Trk::Material(534.9, 2871.2, 18.6, 9.1, 0.0004);
    Trk::Material solenoidGapMaterial= Trk::Material(182.6, 1007., 22.9, 10.9, 0.0012);
 
    solenoidLArBarrelGap = new Trk::TrackingVolume(nullptr,
                                                   solenoidLArBarrelGapBounds,
                                                   solenoidGapMaterial,
                                                   dummyLayers, dummyVolumes,
                                                   "Calo::GapVolumes::LAr::SolenoidPresamplerGap");
  }

  ////////////////////////////////////////////////////////////////////////////////////////////
  // THE ENDCAP SECTION
  ATH_MSG_DEBUG( "============ Endcap Section ======================" );
  //   PRESAMPLER_EC_POS, PRESAMPLER_EC_NEG
  //   EMEC_POS, EMEC_NEG 
  //   HEC1_POS, HEC1_NEG
  //   HEC2_POS, HEC2_NEG
  //   FCAL1_POS, FCAL1_NEG
  //   FCAL2_POS, FCAL2_NEG
  //   FCAL3_POS, FCAL3_NEG
  
  // positive Side
  Trk::TrackingVolume* lArPositiveEndcapInnerGap   = nullptr;
  Trk::TrackingVolume* lArPositiveEndcap           = nullptr;
  Trk::TrackingVolume* lArPositiveHec              = nullptr;
  Trk::TrackingVolume* lArPositiveHecFcalCover     = nullptr;
  Trk::TrackingVolume* lArPositiveFcal             = nullptr;
  Trk::TrackingVolume* lArPosECPresampler          = nullptr;
  
  // negative Side
  Trk::TrackingVolume* lArNegativeEndcapInnerGap   = nullptr;
  Trk::TrackingVolume* lArNegativeEndcap           = nullptr;
  Trk::TrackingVolume* lArNegativeHec              = nullptr;
  Trk::TrackingVolume* lArNegativeHecFcalCover     = nullptr;
  Trk::TrackingVolume* lArNegativeFcal             = nullptr;
  Trk::TrackingVolume* lArNegECPresampler          = nullptr;
  
  // the smoothed ones
  Trk::CylinderVolumeBounds* lArPositiveHecBounds            = nullptr;
  Trk::CylinderVolumeBounds* lArPositiveHecFcalCoverBounds   = nullptr;
  Trk::CylinderVolumeBounds* lArPositiveFcalBounds           = nullptr;
 
  Trk::CylinderVolumeBounds* lArNegativeHecBounds            = nullptr;
  Trk::CylinderVolumeBounds* lArNegativeHecFcalCoverBounds   = nullptr;
  Trk::CylinderVolumeBounds* lArNegativeFcalBounds           = nullptr;
 
  // (1) now parse the EC
  std::unique_ptr<Trk::CylinderVolumeBounds> lArPositiveEndcapBounds;
  std::unique_ptr<Trk::CylinderVolumeBounds> lArNegativeEndcapBounds;

  if(detStore()->contains<StoredPhysVol>("EMEC_POS"))
    {
      if(detStore()->retrieve(storedPV,"EMEC_POS")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV EMEC_POS" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArPositiveEndcapPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;

  const GeoLogVol* lArPositiveEndcapLogVol = lArPositiveEndcapPhysVol ? lArPositiveEndcapPhysVol->getLogVol() : nullptr;

  if(detStore()->contains<StoredPhysVol>("EMEC_NEG"))
    {
      if(detStore()->retrieve(storedPV,"EMEC_NEG")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV EMEC_NEG" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArNegativeEndcapPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;

  const GeoLogVol* lArNegativeEndcapLogVol = lArNegativeEndcapPhysVol ? lArNegativeEndcapPhysVol->getLogVol() : nullptr;
  
  // get the material
  const GeoMaterial* lArPositiveEndcapMaterial = nullptr;
  //const GeoMaterial* lArNegativeEndcapMaterial = 0;
  
  std::vector<double> positiveEndcapZboundaries;
  std::vector<double> negativeEndcapZboundaries;
  
  double lArEndcapZpos      = 0.;
  
  // retrival worked out
  if (lArPositiveEndcapLogVol && lArNegativeEndcapLogVol){
    
    int poschilds = lArPositiveEndcapPhysVol->getNChildVols();
     int negchilds = lArNegativeEndcapPhysVol->getNChildVols();
     
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArPositiveEndcapPhysVol->getAbsoluteName()
		      << " (" << poschilds << " childs)." );
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArNegativeEndcapPhysVol->getAbsoluteName()
         << " (" << negchilds << " childs)." );


     // and the shapes
     const GeoShape*    lArPositiveEndcapShape  = lArPositiveEndcapLogVol->getShape();
     const GeoShape*    lArNegativeEndcapShape  = lArNegativeEndcapLogVol->getShape();

     // get the transforms
     const Amg::Transform3D& lArPositiveEndcapTransform = lArPositiveEndcapPhysVol->getAbsoluteTransform();
     //const Amg::Transform3D& lArNegativeEndcapTransform = Amg::CLHEPTransformToEigen(lArNegativeEndcapPhysVol->getAbsoluteTransform());
     Amg::Vector3D lArPositiveEndcapNomPosition = lArPositiveEndcapTransform.translation();
     //Amg::Vector3D lArNegativeEndcapNomPosition = lArNegativeEndcapTransform.translation();
     
     // dynamic cast to 'Tubs' shape
     const GeoPcon* lArPositiveEndcapPcon = dynamic_cast<const GeoPcon*>(lArPositiveEndcapShape);
     if (lArPositiveEndcapPcon)
       lArPositiveEndcapBounds = std::unique_ptr<Trk::CylinderVolumeBounds>
         (Trk::GeoShapeConverter::convert(lArPositiveEndcapPcon,
                                         positiveEndcapZboundaries));

     const GeoPcon* lArNegativeEndcapPcon = dynamic_cast<const GeoPcon*>(lArNegativeEndcapShape);
     if (lArNegativeEndcapPcon)
       lArNegativeEndcapBounds = std::unique_ptr<Trk::CylinderVolumeBounds>
         (Trk::GeoShapeConverter::convert(lArNegativeEndcapPcon,
                                         negativeEndcapZboundaries));
     
     if (lArPositiveEndcapBounds)
       ATH_MSG_DEBUG( " -> Positive Endcap Bounds: " << *lArPositiveEndcapBounds ); 
     if (lArNegativeEndcapBounds)
       ATH_MSG_DEBUG( " -> Negative Endcap Bounds: " << *lArNegativeEndcapBounds );
     
     double positiveEndcapZpos = 0.5 *(positiveEndcapZboundaries[1] + positiveEndcapZboundaries[0]);
     
     lArEndcapZpos = positiveEndcapZpos+lArPositiveEndcapNomPosition.z();
     
     ATH_MSG_DEBUG( "       located at z-positions " << lArEndcapZpos << " / " << -lArEndcapZpos );  
     
     // assing the material
     lArPositiveEndcapMaterial = lArPositiveEndcapLogVol->getMaterial();
     //lArNegativeEndcapMaterial = lArNegativeEndcapLogVol->getMaterial();
     
  }

  double lArEndcapHalfZ = 0.;
  double lArEndcapZmin = 0.;
  double lArEndcapZmax = 0.;
  double lArEndcapInnerRadius = 0;
  double lArEndcapOuterRadius = 0;

  // now create the Tracking Volumes
  if (lArPositiveEndcapBounds && lArNegativeEndcapBounds && lArPositiveEndcapMaterial){
    
    
    // create the material
    //Trk::MaterialProperties lArEndcapMaterial = Trk::MaterialProperties(1., 22.2/0.99, 0.0027*pow(0.99,3), 39.);
    const Trk::Material* lArEndcapMaterial=new Trk::Material(22.21, 402.2, 72.6, 30.5, 0.0039);
    gc.bin.insert(lArEndcapMaterial);

    lArEndcapHalfZ = lArPositiveEndcapBounds->halflengthZ();
    lArEndcapZmin = lArEndcapZpos - lArPositiveEndcapBounds->halflengthZ();
    lArEndcapZmax = lArEndcapZpos + lArPositiveEndcapBounds->halflengthZ();
    lArEndcapInnerRadius = lArPositiveEndcapBounds->innerRadius();
    lArEndcapOuterRadius = lArPositiveEndcapBounds->outerRadius();

    Amg::Vector3D lArEndcapPositionPos(0.,0.,lArEndcapZpos);
    Amg::Vector3D lArEndcapPositionNeg(0.,0.,-lArEndcapZpos);
    Amg::Transform3D* lArPositiveEndcapTransform = new Amg::Transform3D(Amg::Translation3D(lArEndcapPositionPos));
    Amg::Transform3D* lArNegativeEndcapTransform = new Amg::Transform3D(Amg::Translation3D(lArEndcapPositionNeg));
    
    Amg::Transform3D* align = nullptr;
    // binned material for LAr
    Trk::BinUtility* bup = new Trk::BinUtility(37,1.35,3.2,Trk::open,Trk::binEta);
    Trk::BinUtility* bun = new Trk::BinUtility(37,-3.2,-1.35,Trk::open,Trk::binEta);

    // binned material for LAr : steering in binEta
    std::vector<Trk::IdentifiedMaterial> matEID;
    // layer material can be adjusted here
    int baseID = Trk::GeometrySignature(Trk::Calo)*1000 + 4;
    matEID.emplace_back(lArEndcapMaterial,0);
    matEID.emplace_back(lArEndcapMaterial,baseID+1);
    matEID.emplace_back(lArEndcapMaterial,baseID+2);
    matEID.emplace_back(lArEndcapMaterial,baseID+3);
    // scaled
    matEID.emplace_back(lArEndcapMaterial->scale(1.05),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.1),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.15),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.2),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.25),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.3),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.35),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.4),baseID+1);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.05),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.1),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.15),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.2),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.25),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.3),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.35),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.4),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.45),baseID+2);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(0.7),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(0.75),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(0.8),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(0.85),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(0.9),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(0.95),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.05),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.1),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.15),baseID+3);
    gc.bin.insert(matEID.back().first);
    matEID.emplace_back(lArEndcapMaterial->scale(1.2),baseID+3);
    gc.bin.insert(matEID.back().first);

    // binned material for LAr : layer depth per eta bin
    std::vector< Trk::BinUtility*> layEUP(bup->bins());
    // array of indices
    std::vector<std::vector<size_t> > indexEP;  
    // retrieve offset values (positive)
    float z1 = entrySurf[CaloCell_ID::EME1].first->center().z();     // first layer has no modulations
    float z2 = entrySurf[CaloCell_ID::EME2].first->center().z();     // base value
    float z3 = entrySurf[CaloCell_ID::EME3].first->center().z();     // base value

    std::vector<float> offset2;
    const Trk::SlidingDiscSurface* sd2 = dynamic_cast<const Trk::SlidingDiscSurface* > (entrySurf[CaloCell_ID::EME2].first);
    if (sd2) offset2 = sd2->offset();
    std::vector<float>offset3;
    const Trk::SlidingDiscSurface* sd3 = dynamic_cast<const Trk::SlidingDiscSurface* > (entrySurf[CaloCell_ID::EME3].first);
    if (sd3) offset3 = sd3->offset();
    // construct bin utilities 
    std::vector<float> steps;
    for (unsigned int i=0; i< bup->bins(); i++) {
      steps.clear();
      std::vector<size_t> indx; indx.clear();
      steps.push_back( lArEndcapZmin);
      indx.push_back(0);
      steps.push_back(z1);
      if (i<4) indx.push_back(1);
      else if (i<6) indx.push_back(4);
      else if (i<8) indx.push_back(5);
      else if (i<10) indx.push_back(6);
      else if (i<12) indx.push_back(7);
      else if (i<14) indx.push_back(8);
      else if (i<16) indx.push_back(9);
      else if (i<18) indx.push_back(10);
      else if (i<23) indx.push_back(11);
      else indx.push_back(1);

      float z2c = z2 + offset2[i]; 
      if (z2c!= steps.back()) { steps.push_back(z2c); indx.push_back(2);}
      else { indx.back()=2; }
      if (i<4) {}
      else if (i<6) indx.back()=12;
      else if (i<8) indx.back()=13;
      else if (i<10) indx.back()=14;
      else if (i<12) indx.back()=15;
      else if (i<14) indx.back()=16;
      else if (i<16) indx.back()=17;
      else if (i<18) indx.back()=18;
      else if (i<21) indx.back()=19;
      else if (i<23) indx.back()=20;
      else if (i<25) indx.back()=14;
      else if (i<27) indx.back()=15;
      else if (i<29) indx.back()=16;
      else if (i<31) indx.back()=17;
      else if (i<33) indx.back()=18;
      else if (i<35) indx.back()=19;
      else if (i<37) indx.back()=20;
      
      steps.push_back(z3 + offset3[i] );
      if (i<6) indx.push_back(21);
      else if (i<8) indx.push_back(22);
      else if (i<10) indx.push_back(23);
      else if (i<12) indx.push_back(24);
      else if (i<14) indx.push_back(25);
      else if (i<16) indx.push_back(26);
      else if (i<18) indx.push_back(3);
      else if (i<20) indx.push_back(28);
      else if (i<23) indx.push_back(29);
      else if (i<25) indx.push_back(22);
      else if (i<27) indx.push_back(23);
      else if (i<29) indx.push_back(24);
      else if (i<31) indx.push_back(26);
      else if (i<33) indx.push_back(3);
      else if (i<35) indx.push_back(27);
      else indx.push_back(28);
      steps.push_back(lArEndcapZmax);
      Trk::BinUtility* zBU = new Trk::BinUtility(steps, Trk::open, Trk::binZ);
      layEUP[i] = zBU;
      indexEP.push_back(indx);
    } 

    // binned material for LAr : layer depth per eta bin
    std::vector< Trk::BinUtility*> layEUN(bun->bins());
    std::vector<std::vector<size_t> > indexEN;  
    for ( int j=indexEP.size()-1; j>-1; j--) {
      std::vector<size_t> indx; indx.clear();
      for ( int jj=indexEP[j].size()-1; jj>-1; jj--) {
        indx.push_back(indexEP[j][jj]);
      }
      indexEN.push_back(indx);
    } 
    // retrieve offset values (negative)
    z1 = entrySurf[CaloCell_ID::EME1].second->center().z();     // first layer has no modulations
    z2 = entrySurf[CaloCell_ID::EME2].second->center().z();     // base value
    z3 = entrySurf[CaloCell_ID::EME3].second->center().z();     // base value

    offset2.clear();
    sd2 = dynamic_cast<const Trk::SlidingDiscSurface* > (entrySurf[CaloCell_ID::EME2].second);
    if (sd2) offset2 = sd2->offset();
    offset3.clear();
    sd3 = dynamic_cast<const Trk::SlidingDiscSurface* > (entrySurf[CaloCell_ID::EME3].second);
    if (sd3) offset3 = sd3->offset();
    // construct bin utilities ( in increasing ordering )
    for (unsigned int i=0; i< bun->bins(); i++) {
      steps.clear();
      steps.push_back(-lArEndcapZmax);
      steps.push_back(z3 + offset3[i] );
      steps.push_back(z2 + offset2[i] );
      if (z1!= steps.back()) { steps.push_back(z1); }
      steps.push_back(-lArEndcapZmin);
      Trk::BinUtility* zBU = new Trk::BinUtility(steps, Trk::open, Trk::binZ);
      layEUN[i] = zBU;
    } 

    const Trk::BinnedMaterial* lArEndcapMaterialBinnedPos = new Trk::BinnedMaterial(lArEndcapMaterial,bup,layEUP,indexEP,matEID);
    const Trk::BinnedMaterial* lArEndcapMaterialBinnedNeg = new Trk::BinnedMaterial(lArEndcapMaterial,bun,layEUN,indexEN,matEID);

    lArPositiveEndcap = new Trk::AlignableTrackingVolume(lArPositiveEndcapTransform,align,
							 lArPositiveEndcapBounds.release(),
							 lArEndcapMaterialBinnedPos,
                                                         5,
							 //lpEntries,
							 "Calo::Detectors::LAr::PositiveEndcap");
    
    lArNegativeEndcap = new Trk::AlignableTrackingVolume(lArNegativeEndcapTransform,align,
							 lArNegativeEndcapBounds.release(),
							 lArEndcapMaterialBinnedNeg,
							 5,
							 //lnEntries,
							 "Calo::Detectors::LAr::NegativeEndcap");
  }

  // presampler 
  ATH_MSG_DEBUG( "Building Endcap Presampler ... " );
  
  Trk::CylinderVolumeBounds* lArECPresamplerBounds = nullptr;
  
  if(detStore()->contains<StoredPhysVol>("PRESAMPLER_EC_POS"))
    {
      if(detStore()->retrieve(storedPV,"PRESAMPLER_EC_POS")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV PRESAMPLER_EC_POS" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArECPresamplerPhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  // if (lArECPresamplerPhysVol) printInfo(lArECPresamplerPhysVol);

  const GeoLogVol* lArECPresamplerLogVol = lArECPresamplerPhysVol ? lArECPresamplerPhysVol->getLogVol() : nullptr;

  // binned material for EC Presampler : layers only
   std::vector<Trk::IdentifiedMaterial> matECP;
   const Trk::Material* mAr = new Trk::Material(140., 1170./1.4, 40., 18., 0.0014);
   const Trk::Material* mAl = new Trk::Material(88.93, 388.8, 27., 13., 0.0027);
   gc.bin.insert(mAr);
   gc.bin.insert(mAl);

   // layer material can be adjusted here
   int baseID = Trk::GeometrySignature(Trk::Calo)*1000 + 4;
   matECP.emplace_back(mAl,0);
   matECP.emplace_back(mAr,baseID);

  if (  lArECPresamplerLogVol ) {
  
    const GeoShape*    lArECPresamplerShape  = lArECPresamplerLogVol->getShape();
    const Amg::Transform3D& lArECPresamplerTransform = lArECPresamplerPhysVol->getAbsoluteTransform();

    // dynamic cast to 'Tubs' shape
    const GeoTubs* psTubs = dynamic_cast<const GeoTubs*>(lArECPresamplerShape);
 
    float d = psTubs->getZHalfLength();
    // presampler is embedded in 65 mm of Alu
    float ecd = 32.5 ; 
    // the new HepTransforms
    float zec = lArECPresamplerTransform.translation().z()-ecd+d;
    Amg::Vector3D lArECPresamplerPos(0.,0.,zec);
    Amg::Vector3D lArECPresamplerNeg(0.,0.,-zec);
    Amg::Transform3D* lArPosECPresamplerTransform = new Amg::Transform3D(Amg::Translation3D(lArECPresamplerPos));
    Amg::Transform3D* lArNegECPresamplerTransform = new Amg::Transform3D(Amg::Translation3D(lArECPresamplerNeg));

    lArECPresamplerBounds = new Trk::CylinderVolumeBounds(psTubs->getRMin(),psTubs->getRMax(),ecd);

    // layer binning in Z
    std::vector<float> ecp;
    ecp.push_back( zec-ecd);
    ecp.push_back( zec+ecd-2*d);
    ecp.push_back( zec+ecd);
    Trk::BinUtility* hecp = new Trk::BinUtility(ecp,Trk::open,Trk::binZ);

    // material index
    std::vector<size_t> iep{0,1};

    // binned material
    const Trk::BinnedMaterial* lArECPresamplerMaterialBinPos = new Trk::BinnedMaterial( lArBarrelPresamplerMaterial,hecp,iep,matECP);

    Amg::Transform3D* align=nullptr;

    lArPosECPresampler = new Trk::AlignableTrackingVolume(lArPosECPresamplerTransform, align,
							  lArECPresamplerBounds,
							  lArECPresamplerMaterialBinPos,
							  4,
							  "Calo::Detectors::LAr::PositiveECPresampler");

    // layer binning in Z
    std::vector<float> ecpn;
    ecpn.push_back(-zec-ecd);
    ecpn.push_back(-zec-ecd+2*d);
    ecpn.push_back(-zec+ecd);
    Trk::BinUtility* hecpn = new Trk::BinUtility(ecpn,Trk::open,Trk::binZ);

    // material index
    std::vector<size_t> ien{1,0};

    // binned material
    const Trk::BinnedMaterial* lArECPresamplerMaterialBinNeg = new Trk::BinnedMaterial( lArBarrelPresamplerMaterial,hecpn,ien,matECP);

    lArNegECPresampler = new Trk::AlignableTrackingVolume(lArNegECPresamplerTransform, align,
							  lArECPresamplerBounds->clone(),
							  lArECPresamplerMaterialBinNeg,
							  4,
							  "Calo::Detectors::LAr::NegativeECPresampler");


  }
  
  // (2) now parse the HEC
  Trk::CylinderVolumeBounds* lArPositiveHec1Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArPositiveHec2Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArNegativeHec1Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArNegativeHec2Bounds = nullptr;
  

  if(detStore()->contains<StoredPhysVol>("HEC1_POS")){
     
    if(detStore()->retrieve(storedPV,"HEC1_POS")==StatusCode::FAILURE){
      ATH_MSG_DEBUG( "Unable to retrieve Stored PV HEC1_POS" );
      storedPV = nullptr;
    }
    
  }
  GeoFullPhysVol* lArPositiveHec1PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  if(detStore()->contains<StoredPhysVol>("HEC2_POS")){
     
    if(detStore()->retrieve(storedPV,"HEC2_POS")==StatusCode::FAILURE){
      ATH_MSG_DEBUG( "Unable to retrieve Stored PV HEC2_POS" );
      storedPV = nullptr;
    }
    
  }
  GeoFullPhysVol* lArPositiveHec2PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  if(detStore()->contains<StoredPhysVol>("HEC1_NEG")){
    
    if(detStore()->retrieve(storedPV,"HEC1_NEG")==StatusCode::FAILURE){
      ATH_MSG_DEBUG( "Unable to retrieve Stored PV HEC1_NEG" );
      storedPV = nullptr;
    }
    
  }
  GeoFullPhysVol* lArNegativeHec1PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;

  if(detStore()->contains<StoredPhysVol>("HEC2_NEG")){
     
    if(detStore()->retrieve(storedPV,"HEC2_NEG")==StatusCode::FAILURE){
      ATH_MSG_DEBUG("Unable to retrieve Stored PV HEC2_NEG" );
      storedPV = nullptr;
     }
    
  }
  
  GeoFullPhysVol* lArNegativeHec2PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  const GeoLogVol* lArPositiveHec1LogVol = lArPositiveHec1PhysVol ? lArPositiveHec1PhysVol->getLogVol() : nullptr;
  const GeoLogVol* lArPositiveHec2LogVol = lArPositiveHec2PhysVol ? lArPositiveHec2PhysVol->getLogVol() : nullptr;
  const GeoLogVol* lArNegativeHec1LogVol = lArNegativeHec1PhysVol ? lArNegativeHec1PhysVol->getLogVol() : nullptr;
  const GeoLogVol* lArNegativeHec2LogVol = lArNegativeHec2PhysVol ? lArNegativeHec2PhysVol->getLogVol() : nullptr;

  std::vector<double> positiveEndcapZboundariesHec1;
  std::vector<double> positiveEndcapZboundariesHec2;
  std::vector<double> negativeEndcapZboundariesHec1;
  std::vector<double> negativeEndcapZboundariesHec2;
  double hecEnd = 0;
  
  // retrival worked out
  if (lArPositiveHec1LogVol && lArPositiveHec2LogVol && lArNegativeHec1LogVol && lArNegativeHec2LogVol){
    
    int poschildsHec1 = lArPositiveHec1PhysVol->getNChildVols();
    int poschildsHec2 = lArPositiveHec2PhysVol->getNChildVols();
    int negchildsHec1 = lArNegativeHec1PhysVol->getNChildVols();
    int negchildsHec2 = lArNegativeHec2PhysVol->getNChildVols();
    
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArPositiveHec1PhysVol->getAbsoluteName()
		     << " (" << poschildsHec1 << " childs) ." );
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArNegativeHec2PhysVol->getAbsoluteName()
		     << " (" << poschildsHec2 << " childs) ." );
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArPositiveHec1PhysVol->getAbsoluteName()
		     << " (" << negchildsHec1 << " childs) ." );
    ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArNegativeHec2PhysVol->getAbsoluteName()
		     << " (" << negchildsHec2 << " childs) ." );
    
    // and the shapes
    const GeoShape*    lArPositiveHec1Shape  = lArPositiveHec1LogVol->getShape();
    const GeoShape*    lArPositiveHec2Shape  = lArPositiveHec2LogVol->getShape();
    const GeoShape*    lArNegativeHec1Shape  = lArNegativeHec1LogVol->getShape();
    const GeoShape*    lArNegativeHec2Shape  = lArNegativeHec2LogVol->getShape();
    
    // get the transforms
    const Amg::Transform3D& lArPositiveHec1Transform = lArPositiveHec1PhysVol->getAbsoluteTransform();
    const Amg::Transform3D& lArPositiveHec2Transform = lArPositiveHec2PhysVol->getAbsoluteTransform();
    const Amg::Transform3D& lArNegativeHec1Transform = lArNegativeHec1PhysVol->getAbsoluteTransform();
    const Amg::Transform3D& lArNegativeHec2Transform = lArNegativeHec2PhysVol->getAbsoluteTransform();
    
    Amg::Vector3D lArPositiveHec1NomPosition = lArPositiveHec1Transform.translation();
    Amg::Vector3D lArPositiveHec2NomPosition = lArPositiveHec2Transform.translation();
    Amg::Vector3D lArNegativeHec1NomPosition = lArNegativeHec1Transform.translation();
    Amg::Vector3D lArNegativeHec2NomPosition = lArNegativeHec2Transform.translation();
     
    // dynamic cast to 'Pcon' shape
    const GeoPcon* lArPositiveHec1Pcon = dynamic_cast<const GeoPcon*>(lArPositiveHec1Shape);
    lArPositiveHec1Bounds = (lArPositiveHec1Pcon) ? Trk::GeoShapeConverter::convert(lArPositiveHec1Pcon, 
										   positiveEndcapZboundariesHec1) : nullptr;
    const GeoPcon* lArPositiveHec2Pcon = dynamic_cast<const GeoPcon*>(lArPositiveHec2Shape);
    lArPositiveHec2Bounds = (lArPositiveHec2Pcon) ? Trk::GeoShapeConverter::convert(lArPositiveHec2Pcon,
										   positiveEndcapZboundariesHec2) : nullptr;
    const GeoPcon* lArNegativeHec1Pcon = dynamic_cast<const GeoPcon*>(lArNegativeHec1Shape);
    lArNegativeHec1Bounds = (lArNegativeHec1Pcon) ? Trk::GeoShapeConverter::convert(lArNegativeHec1Pcon,
										   negativeEndcapZboundariesHec1) : nullptr;
    const GeoPcon* lArNegativeHec2Pcon = dynamic_cast<const GeoPcon*>(lArNegativeHec2Shape);
    lArNegativeHec2Bounds = (lArNegativeHec2Pcon) ? Trk::GeoShapeConverter::convert(lArNegativeHec2Pcon,
										   negativeEndcapZboundariesHec2) : nullptr;
    
    if (lArPositiveHec1Bounds)
      ATH_MSG_VERBOSE( " -> Positive Hec1 Bounds: " << *lArPositiveHec1Bounds );
    if (lArPositiveHec2Bounds)
      ATH_MSG_VERBOSE( " -> Positive Hec2 Bounds: " << *lArPositiveHec2Bounds );
    
    if (lArNegativeHec1Bounds)
      ATH_MSG_VERBOSE( " -> Negative Hec1 Bounds: " << *lArNegativeHec1Bounds );
    if (lArNegativeHec2Bounds)
      ATH_MSG_VERBOSE( " -> Negative Hec2 Bounds: " << *lArNegativeHec2Bounds );
    
    
    double positiveHec1Zpos = 0.5 *(positiveEndcapZboundariesHec1[1] + positiveEndcapZboundariesHec1[0]);
    double positiveHec2Zpos = 0.5 *(positiveEndcapZboundariesHec2[1] + positiveEndcapZboundariesHec2[0]);
    double negativeHec1Zpos = -fabs(0.5 *(negativeEndcapZboundariesHec1[1] + negativeEndcapZboundariesHec1[0]));
    double negativeHec2Zpos = -fabs(0.5 *(negativeEndcapZboundariesHec2[1] + negativeEndcapZboundariesHec2[0]));
    
    ATH_MSG_VERBOSE( "   Positive parts located at: " <<   positiveHec1Zpos + lArPositiveHec1NomPosition.z()
		     << " / " <<   positiveHec2Zpos + lArPositiveHec2NomPosition.z() );
    
    ATH_MSG_VERBOSE( "   Negative parts located at: " <<   negativeHec1Zpos + lArNegativeHec1NomPosition.z()
		     << " / " <<   negativeHec2Zpos + lArNegativeHec2NomPosition.z() );

  }
  
  // (3) Browser the FCAL, we construct some things
  // from HEC numbers, but we need the radius to build the HEC too
  // So we browse the FCAL before building the HEC
  // We will build both later
  //   FCAL1_POS, FCAL1_NEG
  //   FCAL2_POS, FCAL2_NEG
  //   FCAL3_POS, FCAL3_NEG
  Trk::CylinderVolumeBounds* lArPositiveFcal1Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArPositiveFcal2Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArPositiveFcal3Bounds = nullptr;
  

  Trk::CylinderVolumeBounds* lArNegativeFcal1Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArNegativeFcal2Bounds = nullptr;
  Trk::CylinderVolumeBounds* lArNegativeFcal3Bounds = nullptr;
  
  if(detStore()->contains<StoredPhysVol>("FCAL1_POS"))
   {
     if(detStore()->retrieve(storedPV,"FCAL1_POS")==StatusCode::FAILURE)
       {
	 ATH_MSG_DEBUG( "Unable to retrieve Stored PV FCAL1_POS" );
	 storedPV = nullptr;
       }
   }
  GeoFullPhysVol* lArPositiveFcal1PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  if(detStore()->contains<StoredPhysVol>("FCAL2_POS"))
    {
      if(detStore()->retrieve(storedPV,"FCAL2_POS")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV FCAL2_POS" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArPositiveFcal2PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  
  if(detStore()->contains<StoredPhysVol>("FCAL3_POS"))
    {
      if(detStore()->retrieve(storedPV,"FCAL3_POS")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV FCAL3_POS" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArPositiveFcal3PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  if(detStore()->contains<StoredPhysVol>("FCAL1_NEG"))
    {
      if(detStore()->retrieve(storedPV,"FCAL1_NEG")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV FCAL1_NEG" );
	  storedPV = nullptr;
	}
    }
  GeoFullPhysVol* lArNegativeFcal1PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
  
  if(detStore()->contains<StoredPhysVol>("FCAL2_NEG"))
    {
      if(detStore()->retrieve(storedPV,"FCAL2_NEG")==StatusCode::FAILURE)
	{
	  ATH_MSG_DEBUG( "Unable to retrieve Stored PV FCAL2_NEG" );
	  storedPV = nullptr;
	}
    }
   GeoFullPhysVol* lArNegativeFcal2PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
   
   if(detStore()->contains<StoredPhysVol>("FCAL3_NEG"))
     {
       if(detStore()->retrieve(storedPV,"FCAL3_NEG")==StatusCode::FAILURE)
	 {
	   ATH_MSG_DEBUG( "Unable to retrieve Stored PV FCAL3_NEG" );
	   storedPV = nullptr;
	 }
     }
   GeoFullPhysVol* lArNegativeFcal3PhysVol = storedPV ? storedPV->getPhysVol() : nullptr;
   
   const GeoLogVol* lArPositiveFcal1LogVol = lArPositiveFcal1PhysVol ? lArPositiveFcal1PhysVol->getLogVol() : nullptr;
   const GeoLogVol* lArPositiveFcal2LogVol = lArPositiveFcal2PhysVol ? lArPositiveFcal2PhysVol->getLogVol() : nullptr;
   const GeoLogVol* lArPositiveFcal3LogVol = lArPositiveFcal3PhysVol ? lArPositiveFcal3PhysVol->getLogVol() : nullptr;

   const GeoLogVol* lArNegativeFcal1LogVol = lArNegativeFcal1PhysVol ? lArNegativeFcal1PhysVol->getLogVol() : nullptr;
   const GeoLogVol* lArNegativeFcal2LogVol = lArNegativeFcal2PhysVol ? lArNegativeFcal2PhysVol->getLogVol() : nullptr;
   const GeoLogVol* lArNegativeFcal3LogVol = lArNegativeFcal3PhysVol ? lArNegativeFcal3PhysVol->getLogVol() : nullptr;

    // z position - force to be symmetric
   double lArFcalHalflength = 0.;
   double lArFcalZposition  = 0.;
   double lArFcalZmin       = 0.;
   double lArFcalZmax       = 0.;

   // retrival worked out
   if (lArPositiveFcal1LogVol && 
       lArPositiveFcal2LogVol && 
       lArPositiveFcal3LogVol && 
       lArNegativeFcal1LogVol && 
       lArNegativeFcal2LogVol &&
       lArNegativeFcal3LogVol){

     int poschildsFcal1 = lArPositiveFcal1PhysVol->getNChildVols();
     int poschildsFcal2 = lArPositiveFcal2PhysVol->getNChildVols();
     int poschildsFcal3 = lArPositiveFcal3PhysVol->getNChildVols();

     int negchildsFcal1 = lArNegativeFcal1PhysVol->getNChildVols();
     int negchildsFcal2 = lArNegativeFcal2PhysVol->getNChildVols();
     int negchildsFcal3 = lArNegativeFcal3PhysVol->getNChildVols();

     
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArPositiveFcal1PhysVol->getAbsoluteName()
         << " (" << poschildsFcal1 << " childs) ." );
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArPositiveFcal2PhysVol->getAbsoluteName()
         << " (" << poschildsFcal2 << " childs) ." );
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArPositiveFcal3PhysVol->getAbsoluteName()
         << " (" << poschildsFcal3 << " childs) ." );

     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArNegativeFcal1PhysVol->getAbsoluteName()
         << " (" << negchildsFcal1 << " childs) ." );
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArNegativeFcal2PhysVol->getAbsoluteName()
         << " (" << negchildsFcal2 << " childs) ." );
     ATH_MSG_VERBOSE( " -> Retrieved GeoModel Volume " << lArNegativeFcal3PhysVol->getAbsoluteName()
         << " (" << negchildsFcal3 << " childs) ." );

     // and the shapes
     const GeoShape*    lArPositiveFcal1Shape  = lArPositiveFcal1LogVol->getShape();
     const GeoShape*    lArPositiveFcal2Shape  = lArPositiveFcal2LogVol->getShape();
     const GeoShape*    lArPositiveFcal3Shape  = lArPositiveFcal3LogVol->getShape();

     const GeoShape*    lArNegativeFcal1Shape  = lArNegativeFcal1LogVol->getShape();
     const GeoShape*    lArNegativeFcal2Shape  = lArNegativeFcal2LogVol->getShape();
     const GeoShape*    lArNegativeFcal3Shape  = lArNegativeFcal3LogVol->getShape();


     // get the transforms
     const Amg::Transform3D& lArPositiveFcal1Transform = lArPositiveFcal1PhysVol->getAbsoluteTransform();
     const Amg::Transform3D& lArPositiveFcal2Transform = lArPositiveFcal2PhysVol->getAbsoluteTransform();
     const Amg::Transform3D& lArPositiveFcal3Transform = lArPositiveFcal3PhysVol->getAbsoluteTransform();

     const Amg::Transform3D& lArNegativeFcal1Transform = lArNegativeFcal1PhysVol->getAbsoluteTransform();
     const Amg::Transform3D& lArNegativeFcal2Transform = lArNegativeFcal2PhysVol->getAbsoluteTransform();
     const Amg::Transform3D& lArNegativeFcal3Transform = lArNegativeFcal3PhysVol->getAbsoluteTransform();

     Amg::Vector3D lArPositiveFcal1NomPosition = lArPositiveFcal1Transform.translation();
     Amg::Vector3D lArPositiveFcal2NomPosition = lArPositiveFcal2Transform.translation();
     Amg::Vector3D lArPositiveFcal3NomPosition = lArPositiveFcal3Transform.translation();

     Amg::Vector3D lArNegativeFcal1NomPosition = lArNegativeFcal1Transform.translation();
     Amg::Vector3D lArNegativeFcal2NomPosition = lArNegativeFcal2Transform.translation();
     Amg::Vector3D lArNegativeFcal3NomPosition = lArNegativeFcal3Transform.translation();

     // dynamic cast to 'Pcon' shape
     const GeoTubs* lArPositiveFcal1Tubs = dynamic_cast<const GeoTubs*>(lArPositiveFcal1Shape);
     lArPositiveFcal1Bounds = (lArPositiveFcal1Tubs) ? Trk::GeoShapeConverter::convert(lArPositiveFcal1Tubs) : nullptr;
     const GeoTubs* lArPositiveFcal2Tubs = dynamic_cast<const GeoTubs*>(lArPositiveFcal2Shape);
     lArPositiveFcal2Bounds = (lArPositiveFcal2Tubs) ? Trk::GeoShapeConverter::convert(lArPositiveFcal2Tubs) : nullptr;
     const GeoTubs* lArPositiveFcal3Tubs = dynamic_cast<const GeoTubs*>(lArPositiveFcal3Shape);
     lArPositiveFcal3Bounds = (lArPositiveFcal3Tubs) ? Trk::GeoShapeConverter::convert(lArPositiveFcal3Tubs) : nullptr;

     const GeoTubs* lArNegativeFcal1Tubs = dynamic_cast<const GeoTubs*>(lArNegativeFcal1Shape);
     lArNegativeFcal1Bounds = (lArNegativeFcal1Tubs) ? Trk::GeoShapeConverter::convert(lArNegativeFcal1Tubs) : nullptr;
     const GeoTubs* lArNegativeFcal2Tubs = dynamic_cast<const GeoTubs*>(lArNegativeFcal2Shape);
     lArNegativeFcal2Bounds = (lArNegativeFcal2Tubs) ? Trk::GeoShapeConverter::convert(lArNegativeFcal2Tubs) : nullptr;
     const GeoTubs* lArNegativeFcal3Tubs = dynamic_cast<const GeoTubs*>(lArNegativeFcal3Shape);
     lArNegativeFcal3Bounds = (lArNegativeFcal3Tubs) ? Trk::GeoShapeConverter::convert(lArNegativeFcal3Tubs) : nullptr;

     if (lArPositiveFcal1Bounds)
           ATH_MSG_VERBOSE( " -> Positive Fcal1 Bounds: " << *lArPositiveFcal1Bounds );
     if (lArPositiveFcal2Bounds)
           ATH_MSG_VERBOSE( " -> Positive Fcal2 Bounds: " << *lArPositiveFcal2Bounds );
     if (lArPositiveFcal3Bounds)
           ATH_MSG_VERBOSE( " -> Positive Fcal3 Bounds: " << *lArPositiveFcal3Bounds );


     if (lArNegativeFcal1Bounds)
           ATH_MSG_VERBOSE( " -> Negative Fcal1 Bounds: " << *lArNegativeFcal1Bounds );
     if (lArNegativeFcal2Bounds)
           ATH_MSG_VERBOSE( " -> Negative Fcal2 Bounds: " << *lArNegativeFcal2Bounds );
     if (lArNegativeFcal3Bounds)
           ATH_MSG_VERBOSE( " -> Negative Fcal3 Bounds: " << *lArNegativeFcal3Bounds );


     ATH_MSG_VERBOSE( "   Positive parts located at: " << lArPositiveFcal1NomPosition.z()
         << " / " << lArPositiveFcal2NomPosition.z() << " / " << lArPositiveFcal3NomPosition.z() );
 
     ATH_MSG_VERBOSE( "   Negative parts located at: " << lArNegativeFcal1NomPosition.z()
         << " / " <<   lArNegativeFcal2NomPosition.z() << " / " << lArNegativeFcal3NomPosition.z() );

     // construct the halflength 
     // this is actual halflength
     // will change to include the cover, but this is left here for future use
     lArFcalHalflength = lArPositiveFcal3NomPosition.z() + lArPositiveFcal3Bounds->halflengthZ()
                        - lArPositiveFcal1NomPosition.z() + lArNegativeFcal1Bounds->halflengthZ();
                         
     lArFcalHalflength *= 0.5;
     // construct the z-Position
     // this is actual z-Position
     // will change to include the cover, but this is left here for future use
     lArFcalZposition  = lArPositiveFcal3NomPosition.z() + lArPositiveFcal3Bounds->halflengthZ();
     lArFcalZposition += lArPositiveFcal1NomPosition.z() - lArNegativeFcal1Bounds->halflengthZ();
     lArFcalZposition *= 0.5;
   }

   //Building section, we start with the HEC

   // get position and halflength of the Fill-In-Hec 
   // First HEC, if we do not use calo surface builder will go
   // up to sensitive FCAL (too long)
   double lArHecZmax          = lArFcalZposition - lArFcalHalflength;
   double lArHecZmin          = 0;
   if (lArPositiveEndcap && lArEndcapHalfZ != 0)
     lArHecZmin = lArPositiveEndcap->center().z() + lArEndcapHalfZ;
   else
     ATH_MSG_ERROR("lArPositiveEndcap/Bounds is null!");

   //small offset between caloSurfaceBuilder and GeoModel
   double caloSurfZOffset = 0;

   if(m_useCaloSurfBuilder){

     double z, rmin, rmax, hphi, depth;
     Amg::Transform3D pos;
     m_calosurf->get_disk_surface(CaloCell_ID::HEC0, 1, pos, z, rmin, rmax, hphi, depth, &caloDDM);
     caloSurfZOffset = lArHecZmin - z;
     lArHecZmax = z + depth + caloSurfZOffset;
     m_calosurf->get_disk_surface(CaloCell_ID::HEC3, 1, pos, z, rmin, rmax, hphi, depth, &caloDDM);
     hecEnd = z + depth + caloSurfZOffset;
   }

   // that creates a position
   double lArHecZpos          = 0.5*(lArHecZmax + lArHecZmin);
   double lArHecHalflength    = 0.5*(lArHecZmax - lArHecZmin);

   double hecFcalCoverHalflength = 0.5*(hecEnd - lArHecZmax);
   double hecFcalCoverZpos = 0.5*(lArHecZmax + hecEnd);

   lArFcalHalflength = hecFcalCoverHalflength;
   lArFcalZposition = hecFcalCoverZpos;

   // binned material for HEC : layers only
   std::vector<Trk::IdentifiedMaterial> matHEC;
   //Trk::MaterialProperties lArHecFcalCoverMaterial = geoMaterialToMaterialProperties.convert(lArPositiveHec1Material);
   //Trk::MaterialProperties lArHecFcalCoverMaterial = Trk::MaterialProperties(1., 18.6, 0.00345, 27.);
   const Trk::Material* lArHecFcalCoverMaterial=new Trk::Material(18.4, 201.9, 57.2, 26.1, 0.0071);
   const Trk::Material* lArHecMaterial = new Trk::Material(19., 224.4, 56.7, 25.8, 0.007);
   gc.bin.insert(lArHecFcalCoverMaterial);
   gc.bin.insert(lArHecMaterial);

   // layer material can be adjusted here
   baseID = Trk::GeometrySignature(Trk::Calo)*1000 + 8;
   matHEC.emplace_back(lArHecFcalCoverMaterial->scale(0.13*m_scale_HECmaterial),0);
   gc.bin.insert(matHEC.back().first);
   matHEC.emplace_back(lArHecMaterial->scale(m_scale_HECmaterial),baseID);
   gc.bin.insert(matHEC.back().first);
   matHEC.emplace_back(lArHecFcalCoverMaterial->scale(0.93*m_scale_HECmaterial),baseID+1);
   gc.bin.insert(matHEC.back().first);
   matHEC.emplace_back(lArHecFcalCoverMaterial->scale(1.09*m_scale_HECmaterial),baseID+2);
   gc.bin.insert(matHEC.back().first);
   matHEC.emplace_back(lArHecFcalCoverMaterial->scale(1.12*m_scale_HECmaterial),baseID+3);
   gc.bin.insert(matHEC.back().first);

   // divide the HEC into two parts per EC :
   // -  fit one around the FCAL - and adopt to LAr Endcap outer radius
   if (lArPositiveFcal1Bounds && lArNegativeFcal1Bounds){
       // cleanup the HecBounds
       delete lArPositiveHec1Bounds; lArPositiveHec1Bounds = nullptr;
       delete lArPositiveHec2Bounds; lArPositiveHec2Bounds = nullptr;
       delete lArNegativeHec1Bounds; lArNegativeHec1Bounds = nullptr;
       delete lArNegativeHec2Bounds; lArNegativeHec2Bounds = nullptr;

       // adopt the boundaries
       lArPositiveHecFcalCoverBounds = new Trk::CylinderVolumeBounds(lArPositiveFcal1Bounds->outerRadius(),
                                                                     lArEndcapOuterRadius,
                                                                     hecFcalCoverHalflength);

       lArNegativeHecFcalCoverBounds = lArPositiveHecFcalCoverBounds->clone();
       // output
       ATH_MSG_DEBUG( "Smoothed LAr Hec (Fcal covering part) bounds : " << *lArPositiveHecFcalCoverBounds );
       ATH_MSG_DEBUG( "   -> at z-position: +/- " << hecFcalCoverZpos );

       // the new HepTransforms
       Amg::Vector3D lArPositiveHecFcalCoverPos(0.,0.,hecFcalCoverZpos);
       Amg::Vector3D lArPositiveHecFcalCoverNeg(0.,0.,-hecFcalCoverZpos);
       Amg::Transform3D* lArPositiveHecFcalCoverTransform = new Amg::Transform3D(Amg::Translation3D(lArPositiveHecFcalCoverPos));
       Amg::Transform3D* lArNegativeHecFcalCoverTransform = new Amg::Transform3D(Amg::Translation3D(lArPositiveHecFcalCoverNeg));

       // building dense volume here
       Amg::Transform3D* align = nullptr;

       // layer binning in Z
       std::vector<float> spCover;
       spCover.push_back(hecFcalCoverZpos-hecFcalCoverHalflength);
       spCover.push_back(entrySurf[CaloCell_ID::HEC1].first->center().z());
       spCover.push_back(entrySurf[CaloCell_ID::HEC2].first->center().z());
       spCover.push_back(entrySurf[CaloCell_ID::HEC3].first->center().z());
       spCover.push_back(hecFcalCoverZpos+hecFcalCoverHalflength);
       Trk::BinUtility* hfp = new Trk::BinUtility(spCover,Trk::open,Trk::binZ);

       // material index
       std::vector<size_t> hfc{0,2,3,4};
       
       // binned material
       const Trk::BinnedMaterial* lArHecFcalCoverMaterialBinPos = new Trk::BinnedMaterial( lArHecFcalCoverMaterial,hfp,hfc,matHEC);
       
       lArPositiveHecFcalCover = new Trk::AlignableTrackingVolume(lArPositiveHecFcalCoverTransform, align,
								  lArPositiveHecFcalCoverBounds,
								  lArHecFcalCoverMaterialBinPos,
								  9,
								  //hpEntries,
								  "Calo::Detectors::LAr::PositiveHecFcalCover");
       // layer binning in Z
       std::vector<float> snCover;
       snCover.push_back(-hecFcalCoverZpos-hecFcalCoverHalflength);
       snCover.push_back(entrySurf[CaloCell_ID::HEC3].second->center().z());
       snCover.push_back(entrySurf[CaloCell_ID::HEC2].second->center().z());
       snCover.push_back(entrySurf[CaloCell_ID::HEC1].second->center().z());
       snCover.push_back(-hecFcalCoverZpos+hecFcalCoverHalflength);
       Trk::BinUtility* hfn = new Trk::BinUtility(snCover,Trk::open,Trk::binZ);

       // material index
       std::vector<size_t> hfcn{4,3,2,0};

       // binned material
       const Trk::BinnedMaterial* lArHecFcalCoverMaterialBinNeg = new Trk::BinnedMaterial( lArHecFcalCoverMaterial,hfn,hfcn,matHEC);

       lArNegativeHecFcalCover = new Trk::AlignableTrackingVolume(lArNegativeHecFcalCoverTransform, align,
								  lArNegativeHecFcalCoverBounds,
								  lArHecFcalCoverMaterialBinNeg,
								  9,
								  //hnEntries,
								  "Calo::Detectors::LAr::NegativeHecFcalCover");  
   }


   // the second part of the HEC between LAr Endcap and FCAL/HEC cover
   if (lArPositiveFcal1Bounds && lArEndcapOuterRadius != 0){

       // get the inner radius
       // ST Hec lower radius moved up
       double lArHecRmin          = 0.5*(lArPositiveFcal1Bounds->outerRadius()+lArEndcapInnerRadius);
       double lArHecRmax          = lArEndcapOuterRadius;
       Amg::Vector3D lArHecZposition(0.,0.,lArHecZpos);
       // bounds
       lArPositiveHecBounds = new Trk::CylinderVolumeBounds(lArHecRmin, lArHecRmax, lArHecHalflength);
       lArNegativeHecBounds = lArPositiveHecBounds->clone();
       // output
       ATH_MSG_DEBUG( "Smoothed LAr Hec bounds : " << *lArPositiveHecBounds );
       ATH_MSG_DEBUG( "   -> at z-position: +/- " << lArHecZpos );

       // the new HepTransforms
       Amg::Vector3D lArPositiveHecPos(0.,0.,lArHecZpos);
       Amg::Vector3D lArPositiveHecNeg(0.,0.,-lArHecZpos);
       Amg::Transform3D* lArPositiveHecTransform = new Amg::Transform3D(Amg::Translation3D(lArPositiveHecPos));
       Amg::Transform3D* lArNegativeHecTransform = new Amg::Transform3D(Amg::Translation3D(lArPositiveHecNeg));

       // building dense volume here
       Amg::Transform3D* align = nullptr;
       
       // layer binning in Z
       std::vector<float> sphec;
       sphec.push_back(lArHecZpos-lArHecHalflength);
       sphec.push_back(entrySurf[CaloCell_ID::HEC0].first->center().z());
       sphec.push_back(lArHecZpos+lArHecHalflength);
       Trk::BinUtility* hp = new Trk::BinUtility(sphec,Trk::open,Trk::binZ);

       // material index
       std::vector<size_t> hf{0,1};

       // binned material
       const Trk::BinnedMaterial* lArHecMaterialBinPos = new Trk::BinnedMaterial( lArHecMaterial,hp,hf,matHEC);

       lArPositiveHec = new Trk::AlignableTrackingVolume(lArPositiveHecTransform,align,
							 lArPositiveHecBounds,
							 lArHecMaterialBinPos,
							 8,
							 //hpEntries,
							 "Calo::Detectors::LAr::PositiveHec");
       
       // layer binning in Z
       std::vector<float> snhec;
       snhec.push_back(-lArHecZpos-lArHecHalflength);
       snhec.push_back(entrySurf[CaloCell_ID::HEC0].second->center().z());
       snhec.push_back(-lArHecZpos+lArHecHalflength);
       Trk::BinUtility* hn = new Trk::BinUtility(snhec,Trk::open,Trk::binZ);

       // material index
       std::vector<size_t> hfn{1,0};

       // binned material
       const Trk::BinnedMaterial* lArHecMaterialBinNeg = new Trk::BinnedMaterial( lArHecMaterial,hn,hfn,matHEC);

       lArNegativeHec = new Trk::AlignableTrackingVolume(lArNegativeHecTransform,align,
							 lArNegativeHecBounds,
							 lArHecMaterialBinNeg,
							 8,
							 //hnEntries,
							 "Calo::Detectors::LAr::NegativeHec");
   }
   
   // Now the FCAL
   // binned material for FCAL : layers only
   std::vector<Trk::IdentifiedMaterial> matFCAL;
   // convert the Material 
   const Trk::Material* lArFcalMaterial =new Trk::Material(8.4, 175.5, 100.8, 42.1, 0.0097);
   const Trk::Material* lArFcalMaterial0 =new Trk::Material(96., 560., 30.3, 14.3, 0.0025);
   gc.bin.insert(lArFcalMaterial);
   gc.bin.insert(lArFcalMaterial0);

   // layer material can be adjusted here
   baseID = Trk::GeometrySignature(Trk::Calo)*1000 + 20;
   matFCAL.emplace_back(lArFcalMaterial0,0);
   matFCAL.emplace_back(lArFcalMaterial->scale(0.5),baseID+1);
   gc.bin.insert(matFCAL.back().first);
   matFCAL.emplace_back(lArFcalMaterial->scale(1.5),baseID+2);
   gc.bin.insert(matFCAL.back().first);
   matFCAL.emplace_back(lArFcalMaterial->scale(1.4),baseID+3);
   gc.bin.insert(matFCAL.back().first);

   // smooth the FCal to Tube form
   if (lArPositiveFcal1Bounds && lArPositiveFcal2Bounds && lArPositiveFcal3Bounds &&
       lArNegativeFcal1Bounds && lArNegativeFcal2Bounds && lArNegativeFcal3Bounds){
      
       // get the minimum/maximmum of the three radii - it's the one of FCAL1
       double lArFcalRmin = lArPositiveFcal1Bounds->innerRadius();
       double lArFcalRmax = lArPositiveFcal1Bounds->outerRadius();
       // assign the bounds
       lArPositiveFcalBounds = new Trk::CylinderVolumeBounds(lArFcalRmin, lArFcalRmax, lArFcalHalflength);
       lArNegativeFcalBounds = lArPositiveFcalBounds->clone();
       // output
       ATH_MSG_DEBUG( "Smoothed LAr Fcal bounds : " << *lArPositiveFcalBounds );
       ATH_MSG_DEBUG( "   -> at z-position: +/- " << lArFcalZposition );

       // get min and max for the Layer Creation
       lArFcalZmin  = lArFcalZposition - lArFcalHalflength;
       lArFcalZmax  = lArFcalZposition + lArFcalHalflength;

       // cleanup 
       delete lArPositiveFcal1Bounds; lArPositiveFcal1Bounds = nullptr;
       delete lArPositiveFcal2Bounds; lArPositiveFcal2Bounds = nullptr;
       delete lArPositiveFcal3Bounds; lArPositiveFcal3Bounds = nullptr;

       delete lArNegativeFcal1Bounds; lArNegativeFcal1Bounds = nullptr;
       delete lArNegativeFcal2Bounds; lArNegativeFcal2Bounds = nullptr;
       delete lArNegativeFcal3Bounds; lArNegativeFcal3Bounds = nullptr;

       // layer binning in Z
       std::vector<float> spfc;
       spfc.push_back(lArFcalZmin);
       spfc.push_back(entrySurf[CaloCell_ID::FCAL0].first->center().z());
       spfc.push_back(entrySurf[CaloCell_ID::FCAL1].first->center().z());
       spfc.push_back(entrySurf[CaloCell_ID::FCAL2].first->center().z());
       spfc.push_back(lArFcalZmax);
       Trk::BinUtility* fcp = new Trk::BinUtility(spfc,Trk::open,Trk::binZ);

       // material index
       std::vector<size_t> hf{0,1,2,3};

       // binned material
       const Trk::BinnedMaterial* lArFcalMaterialBinPos = new Trk::BinnedMaterial( lArFcalMaterial,fcp,hf,matFCAL);

       // layer binning in Z
       std::vector<float> snfc;
       snfc.push_back(-lArFcalZmax);
       snfc.push_back(entrySurf[CaloCell_ID::FCAL2].second->center().z());
       snfc.push_back(entrySurf[CaloCell_ID::FCAL1].second->center().z());
       snfc.push_back(entrySurf[CaloCell_ID::FCAL0].second->center().z());
       snfc.push_back(-lArFcalZmin);
       Trk::BinUtility* fcn = new Trk::BinUtility(snfc,Trk::open,Trk::binZ);

       // material index
       std::vector<size_t> hfn{3,2,1,0};

       // binned material
       const Trk::BinnedMaterial* lArFcalMaterialBinNeg = new Trk::BinnedMaterial( lArFcalMaterial,fcn,hfn,matFCAL);

       // the new HepTransforms
       Amg::Vector3D lArPositiveFcalPos(0.,0.,lArFcalZposition);
       Amg::Vector3D lArPositiveFcalNeg(0.,0.,-lArFcalZposition);
       Amg::Transform3D* lArPositiveFcalTransform = new Amg::Transform3D(Amg::Translation3D(lArPositiveFcalPos));
       Amg::Transform3D* lArNegativeFcalTransform = new Amg::Transform3D(Amg::Translation3D(lArPositiveFcalNeg));

       // building dense volume here
       Amg::Transform3D* align = nullptr;
       
       lArPositiveFcal = new Trk::AlignableTrackingVolume(lArPositiveFcalTransform, align,
							  lArPositiveFcalBounds,
							  lArFcalMaterialBinPos,
							  21,
							  //fcpEntries,
							  "Calo::Detectors::LAr::PositiveFcal");

       lArNegativeFcal = new Trk::AlignableTrackingVolume(lArNegativeFcalTransform, align,
							  lArNegativeFcalBounds,
							  lArFcalMaterialBinNeg,
							  21,
							  //fcnEntries,
							  "Calo::Detectors::LAr::NegativeFcal");
   }

   // fill in the inner Gap
   // ST this better to be done by CaloTrackingGeometry ( to glue with BeamPipe )
   // pass MBTS info to CaloTG 
  // MBTS
  const PVConstLink topEC = lArMgr->getTreeTop(1U);
  Amg::Transform3D trIn= topEC->getX();   
  Amg::Transform3D tr2(trIn);   
  const PVConstLink mbts= getChild(topEC,"MBTS_mother",trIn);

  float mbtsZ{-1};    // MBTS layer position
  float mbts_rmin{0}; // MBTS layer dimensions
  float mbts_rmax{0}; // MBTS layer dimensions

  if (mbts) {
    //printChildren(mbts,-1,0,Amg::Transform3D(trIn));
    const PVConstLink mbts1= getChild(mbts,"MBTS1",trIn);
    if (mbts1) mbtsZ=fabs(trIn.translation().z());
    if (mbts1) {
      ATH_MSG_VERBOSE("MBTS1 layer found at z "<<mbtsZ);
      // retrieve Rmin
      const GeoLogVol* clv = mbts1->getLogVol();
      const GeoTrd* trd=dynamic_cast<const GeoTrd*> (clv->getShape());
      if (trd) mbts_rmin = trIn.translation().perp()-trd->getZHalfLength();
    }
    // retrieve MBTS2 for Rmax
    const PVConstLink mbts2= getChild(mbts,"MBTS2",tr2);
    if (mbts2) {
      const GeoLogVol* clv = mbts2->getLogVol();
      const GeoTrd* trd=dynamic_cast<const GeoTrd*> (clv->getShape());
      if (trd) mbts_rmax = (tr2.translation().perp()+trd->getZHalfLength())/cos(acos(-1.)/8);
    }
    ATH_MSG_VERBOSE("MBTS layer span in R "<<mbts_rmin<<","<<mbts_rmax);
     
  } else {
    ATH_MSG_VERBOSE("MBTS not found ");    
  }
  
  if (mbtsZ>0. && mbts_rmin>0. && mbts_rmax>0.){
    // create the dummy volume to pass on the MBTS position 
    Trk::CylinderVolumeBounds* lArNegativeMBTSBounds = new Trk::CylinderVolumeBounds(
								       mbts_rmin,
								       mbts_rmax,
								       10. );

    ATH_MSG_DEBUG( "Filled in LAr MBTS bounds : " << *lArNegativeMBTSBounds );
    ATH_MSG_DEBUG( "   -> at z-position: +/- " << mbtsZ );

 
    Amg::Vector3D lArEndcapInnerGapPos(0.,0., mbtsZ);
    Amg::Vector3D lArEndcapInnerGapNeg(0.,0.,-mbtsZ);
    Amg::Transform3D* lArPositiveMBTSTransform = new Amg::Transform3D(Amg::Translation3D(lArEndcapInnerGapPos));
    Amg::Transform3D* lArNegativeMBTSTransform = new Amg::Transform3D(Amg::Translation3D(lArEndcapInnerGapNeg));

    // building dense volume here
    lArPositiveEndcapInnerGap = new Trk::TrackingVolume(lArPositiveMBTSTransform,
							lArNegativeMBTSBounds->clone(),
							dummyMaterial,
							dummyLayers, dummyVolumes,
							"Calo::Detectors::MBTS");
    
    lArNegativeEndcapInnerGap = new Trk::TrackingVolume(lArNegativeMBTSTransform,
							lArNegativeMBTSBounds,
							dummyMaterial,
							dummyLayers, dummyVolumes,
							"Calo::Detectors::MBTS");
  }

  if (msgLvl(MSG::DEBUG)) {
    ATH_MSG_DEBUG( "Checking the existence of all Tracking Volumes:" );
    ATH_MSG_DEBUG( "   -> Calo::Solenoid                                     ");
    printCheckResult(msg(MSG::DEBUG), solenoid);
    ATH_MSG_DEBUG( "   -> Calo::GapVolumes::LAr::SolenoidPresamplerGap       ");
    printCheckResult(msg(MSG::DEBUG), solenoidLArBarrelGap);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::BarrelPresampler             ");
    printCheckResult(msg(MSG::DEBUG), lArBarrelPresampler);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::Barrel                       ");
    printCheckResult(msg(MSG::DEBUG), lArBarrel);
    if (lArPositiveEndcapInnerGap) { 
      ATH_MSG_DEBUG( "   -> Calo::GapVolumes::LAr::PositiveEndcapInnerGap      ");
      printCheckResult(msg(MSG::DEBUG), lArPositiveEndcapInnerGap);
    }
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::PositiveEndcap               ");
    printCheckResult(msg(MSG::DEBUG), lArPositiveEndcap);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::PositiveHec                  ");
    printCheckResult(msg(MSG::DEBUG), lArPositiveHec);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::PositiveFcal                 ");
    printCheckResult(msg(MSG::DEBUG), lArPositiveFcal);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::LArPositiveHecFcalCover      ");
    printCheckResult(msg(MSG::DEBUG), lArPositiveHecFcalCover);
    if (lArNegativeEndcapInnerGap) { 
      ATH_MSG_DEBUG( "   -> Calo::GapVolumes::LAr::NegativeEndcapInnerGap      ");
      printCheckResult(msg(MSG::DEBUG), lArNegativeEndcapInnerGap);
    }
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::NegativeEndcap               ");
    printCheckResult(msg(MSG::DEBUG), lArNegativeEndcap);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::NegativeHec                  ");
    printCheckResult(msg(MSG::DEBUG), lArNegativeHec);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::NegativeFcal                 ");
    printCheckResult(msg(MSG::DEBUG), lArNegativeFcal);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::LArNegativeHecFcalCover      ");
    printCheckResult(msg(MSG::DEBUG), lArNegativeHecFcalCover);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::LArPositiveECPresampler      ");
    printCheckResult(msg(MSG::DEBUG), lArPosECPresampler);
    ATH_MSG_DEBUG( "   -> Calo::Detectors::LAr::LArNegativeECPresampler      ");
    printCheckResult(msg(MSG::DEBUG), lArNegECPresampler);
  } // end of detailed output
  
  // check if everything went fine
  if (solenoid && solenoidLArBarrelGap && lArBarrelPresampler && lArBarrel &&
      lArPositiveEndcap && lArPositiveHec && lArPositiveFcal && lArPositiveHecFcalCover &&
      lArNegativeEndcap && lArNegativeHec && lArNegativeFcal && lArNegativeHecFcalCover){
    
    // + register color code for displaying
    
    // Barrel Part
    lArTrackingVolumes->push_back(solenoid);                        // 0
    solenoid->registerColorCode(6);
    lArTrackingVolumes->push_back(solenoidLArBarrelGap);            // 1
    solenoidLArBarrelGap->registerColorCode(21); 
    lArTrackingVolumes->push_back(lArBarrelPresampler);             // 2
    lArBarrelPresampler->registerColorCode(7);
    lArTrackingVolumes->push_back(lArBarrel);                       // 3
    lArBarrel->registerColorCode(3);
    // Positive Endcap Part
    lArTrackingVolumes->push_back(lArPositiveEndcapInnerGap);       //4
    lArTrackingVolumes->push_back(lArPositiveEndcap);               //5
    lArPositiveEndcap->registerColorCode(3);
    lArTrackingVolumes->push_back(lArPositiveHec);                  //6
    lArPositiveHec->registerColorCode(9);
    lArTrackingVolumes->push_back(lArPositiveFcal);                 //7
    lArPositiveFcal->registerColorCode(8);
    lArTrackingVolumes->push_back(lArPositiveHecFcalCover);         //8
    lArPositiveHecFcalCover->registerColorCode(9);
    // Positive Endcap Part
    lArTrackingVolumes->push_back(lArNegativeEndcapInnerGap);       //9
    lArTrackingVolumes->push_back(lArNegativeEndcap);               //10
    lArNegativeEndcap->registerColorCode(3);
    lArTrackingVolumes->push_back(lArNegativeHec);                  //11
    lArNegativeHec->registerColorCode(9);
    lArTrackingVolumes->push_back(lArNegativeFcal);                 //12
    lArNegativeFcal->registerColorCode(8);
    lArTrackingVolumes->push_back(lArNegativeHecFcalCover);         //13
    lArNegativeHecFcalCover->registerColorCode(9);
    lArTrackingVolumes->push_back(lArPosECPresampler);              //14
    lArPosECPresampler->registerColorCode(7);
    lArTrackingVolumes->push_back(lArNegECPresampler);              //15
    lArNegECPresampler->registerColorCode(7);

   }
   return lArTrackingVolumes;
}

void LAr::LArVolumeBuilder::printCheckResult(MsgStream& log, const Trk::TrackingVolume* vol) 
{
  if (vol) log << "... ok"      << endmsg;
  else     log << "... missing" << endmsg;
}

void LAr::LArVolumeBuilder::printInfo(const PVConstLink& pv, int gen) const
{
  const GeoLogVol* lv = pv->getLogVol();
    ATH_MSG_VERBOSE( "New LAr Object:"<<lv->getName()<<", made of"<<lv->getMaterial()->getName()<<","<<lv->getShape()->type());
    const GeoTrd* trd=dynamic_cast<const GeoTrd*> (lv->getShape());
    if (trd) ATH_MSG_VERBOSE("trddim:"<< trd->getXHalfLength1()<<","<<trd->getXHalfLength2()<<","<<trd->getYHalfLength1()<<","<<trd->getYHalfLength2()<<","<<trd->getZHalfLength());
    const GeoTubs* tub=dynamic_cast<const GeoTubs*> (lv->getShape());
    if (tub) ATH_MSG_VERBOSE("tubdim:"<< tub->getRMin()<<","<<tub->getRMax()<<","<<tub->getZHalfLength());
    const GeoTube* tube=dynamic_cast<const GeoTube*> (lv->getShape());
    if (tube) ATH_MSG_VERBOSE("tubdim:"<< tube->getRMin()<<","<<tube->getRMax()<<","<<tube->getZHalfLength());
    const GeoPcon* con=dynamic_cast<const GeoPcon*> (lv->getShape());
    if (con) {
      const unsigned int nPlanes=con->getNPlanes();
      for (unsigned int i=0; i<nPlanes; i++) {
	      ATH_MSG_VERBOSE("polycone:"<<i<<":"<< con->getRMinPlane(i)<<","<<con->getRMaxPlane(i)<<","<<con->getZPlane(i));
      }
    }
    Amg::Transform3D transf =  pv->getX();
    ATH_MSG_VERBOSE( "position:"<< "R:"<<transf.translation().perp()<<",phi:"<< transf.translation().phi()<<",x:"<<transf.translation().x()<<",y:"<<transf.translation().y()<<",z:"<<transf.translation().z());
    int igen = 0;
    printChildren(pv,gen,igen,transf);
}

void LAr::LArVolumeBuilder::printChildren(const PVConstLink& pv,int gen, int igen, const Amg::Transform3D& trIn) const
{
  // subcomponents
  unsigned int nc = pv->getNChildVols();
  igen++; 
  if (gen>-1 && igen>gen) return; 
  std::string cname; 
  for (unsigned int ic=0; ic<nc; ic++) {
    Amg::Transform3D transf = trIn*pv->getXToChildVol(ic);
    const PVConstLink cv = pv->getChildVol(ic);
    const GeoLogVol* clv = cv->getLogVol();
    ATH_MSG_VERBOSE("  ");
    ATH_MSG_VERBOSE( "subcomponent:"<<igen<<":"<<ic<<":"<<clv->getName()<<", made of"<<clv->getMaterial()->getName()<<","<<clv->getShape()->type());
    ATH_MSG_VERBOSE( "position:"<< "R:"<<transf.translation().perp()<<",phi:"<< transf.translation().phi()<<",x:"<<transf.translation().x()<<",y:"<<transf.translation().y()<<",z:"<<transf.translation().z());
    const GeoTrd* trd=dynamic_cast<const GeoTrd*> (clv->getShape());
    if (trd) ATH_MSG_VERBOSE("trddim:"<< trd->getXHalfLength1()<<","<<trd->getXHalfLength2()<<","<<trd->getYHalfLength1()<<","<<trd->getYHalfLength2()<<","<<trd->getZHalfLength());
    const GeoTubs* tub=dynamic_cast<const GeoTubs*> (clv->getShape());
    if (tub) ATH_MSG_VERBOSE("tubdim:"<< tub->getRMin()<<","<<tub->getRMax()<<","<<tub->getZHalfLength());
    const GeoTube* tube=dynamic_cast<const GeoTube*> (clv->getShape());
    if (tube) ATH_MSG_VERBOSE("tubdim:"<< tube->getRMin()<<","<<tube->getRMax()<<","<<tube->getZHalfLength());
    const GeoPcon* con=dynamic_cast<const GeoPcon*> (clv->getShape());
    if (con) {
      const unsigned int nPlanes=con->getNPlanes();
      for (unsigned int i=0; i<nPlanes; i++) {
	      ATH_MSG_VERBOSE("polycone:"<<i<<":"<< con->getRMinPlane(i)<<","<<con->getRMaxPlane(i)<<","<<con->getZPlane(i));
      }
    }

    if (ic==0 || cname != clv->getName() ) {  
      printChildren(cv,gen,igen,transf);
      cname = clv->getName();
    }
  }  
   
}

GeoPVConstLink LAr::LArVolumeBuilder::getChild(const GeoPVConstLink& mother, const std::string& name, Amg::Transform3D& trIn) const
{
  // subcomponents
  for (const GeoVolumeVec_t::value_type& p : geoGetVolumes (&*mother))
  {
    Amg::Transform3D transf = trIn*p.second;
    GeoPVConstLink cv = p.first;
    const GeoLogVol* clv = cv->getLogVol();
    if (clv->getName().substr(0,name.size())==name) { trIn = transf; return cv; } 
    GeoPVConstLink next=getChild(cv,name,transf);
    if (next) {trIn = transf; return next; }    
  }
  return nullptr;
}
