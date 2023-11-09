/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "FrontBeamConstructionH62002.h"
#include "LArGeoH6Cryostats/MWPCConstruction.h"

#include "GeoModelKernel/GeoElement.h"  
#include "GeoModelKernel/GeoMaterial.h"  
#include "GeoModelKernel/GeoFullPhysVol.h"  
#include "GeoModelKernel/GeoVFullPhysVol.h"  
#include "GeoModelKernel/GeoPhysVol.h"  
#include "GeoModelKernel/GeoVPhysVol.h"  
#include "GeoModelKernel/GeoLogVol.h"  
#include "GeoModelKernel/GeoBox.h"  
#include "GeoModelKernel/GeoTubs.h"  
#include "GeoModelKernel/GeoTube.h"  
#include "GeoModelKernel/GeoNameTag.h"  
#include "GeoModelKernel/GeoTransform.h"  
#include "GeoModelKernel/GeoSerialDenominator.h"  
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoAlignableTransform.h"  
#include "GeoModelKernel/GeoIdentifierTag.h"  
#include "GeoModelKernel/GeoDefinitions.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoShapeShift.h" 
#include "GeoGenericFunctions/Variable.h"

// For the database:
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "GeoModelInterfaces/IGeoDbTagSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <string>
#include <cmath>
#include <iostream>

LArGeo::FrontBeamConstructionH62002::FrontBeamConstructionH62002()
 : m_H62002FrontBeamPhysical(nullptr)
{
}


LArGeo::FrontBeamConstructionH62002::~FrontBeamConstructionH62002()
= default;



GeoVPhysVol* LArGeo::FrontBeamConstructionH62002::GetEnvelope()
{

  if (m_H62002FrontBeamPhysical) return m_H62002FrontBeamPhysical;

  // Get access to the material manager:
  
  ISvcLocator *svcLocator = Gaudi::svcLocator();
  IMessageSvc * msgSvc;
  if (svcLocator->service("MessageSvc", msgSvc, true )==StatusCode::FAILURE) {
    throw std::runtime_error("Error in FrontBeamConstructionH62002, cannot access MessageSvc");
  }

  MsgStream log(msgSvc, "LArGeo::FrontBeamConstructionH62002"); 
  log << MSG::INFO;
  log << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  log << "+                                                     +" << std::endl;
  log << "+    HELLO from LArGeo::FrontBeamConstructionH62002   +" << std::endl;
  log << "+                                                     +" << std::endl;
  log << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;


  StoreGateSvc *detStore;
  if (svcLocator->service("DetectorStore", detStore, false )==StatusCode::FAILURE) {
    throw std::runtime_error("Error in FrontBeamConstructionH62002, cannot access DetectorStore");
  }

  ServiceHandle<IGeoDbTagSvc> geoDbTagSvc ("GeoDbTagSvc", "WallsConstruction");
  if (geoDbTagSvc.retrieve().isFailure()) {
    throw std::runtime_error ("Cannot locate GeoDbTagSvc!!");
  }

  // Get the materials from the material manager:-----------------------------------------------------//
  //                                                                                                  //

  StoredMaterialManager* materialManager = nullptr;
  if (StatusCode::SUCCESS != detStore->retrieve(materialManager, std::string("MATERIALS"))) return nullptr;


  const GeoMaterial *Air  = materialManager->getMaterial("std::Air");
  if (!Air) throw std::runtime_error("Error in FrontBeamConstructionH62002, std::Air is not found.");
   
  const GeoMaterial *Aluminium  = materialManager->getMaterial("std::Aluminium");
  if (!Aluminium) throw std::runtime_error("Error in FrontBeamConstructionH62002, std::Aluminium is not found.");

  // Is this ok for the Scintillator?
  // I don't really know for sure what kind of a scintillator we have.
  // Lots of Scintillators are PMMA (Plexiglas), which has a composition of C5 H8 O2 and density 1.18 g/cm3
  // The Tile uses a composition of C H (density 1.032)    
  // The old FrontBeam testbeam code uses a composition of C9 H10 (density 1.032)
  // ... because it's easiest at the moment and not all that different from the fractional
  // composition of the old tb code, take the Tile material (polysterene)...     
  const GeoMaterial *Scint  = materialManager->getMaterial("std::Polystyrene");
  if (!Scint) throw std::runtime_error("Error in FrontBeamConstructionH62002, std::Polystyrene is not found.");
  
  const GeoMaterial *Mylar  = materialManager->getMaterial("std::Mylar");
  if (!Mylar) throw std::runtime_error("Error in FrontBeamConstructionH62002, std::Mylar is not found.");
  
  
  //                                                                                                 //
  //-------------------------------------------------------------------------------------------------//

  std::string AtlasVersion = geoDbTagSvc->atlasVersion();
  std::string LArVersion = geoDbTagSvc->LAr_VersionOverride();
   
  std::string detectorKey  = LArVersion.empty() ? AtlasVersion : LArVersion;
  std::string detectorNode = LArVersion.empty() ? "ATLAS" : "LAr";  

  //////////////////////////////////////////////////////////////////
  // Define geometry
  //////////////////////////////////////////////////////////////////

 
  //  Here we creat the envelope for the Moveable FrontBeam Instrumentation.  This code is repeated
  //  createEnvelope() method below.  There should be a way to avoid this repitition.

  //------ The FrontBeam 

  std::string baseName = "LAr::TBH62002";
  std::string H62002FrontBeamName = baseName + "::FrontBeam";

  const double H62002FrontBeamXY  = 2000.*Gaudi::Units::mm;
  const double H62002FrontBeamZ   =  350.*Gaudi::Units::mm;


  GeoBox* H62002FrontBeamShape = new GeoBox( H62002FrontBeamXY, H62002FrontBeamXY, H62002FrontBeamZ );   
  const GeoLogVol* H62002FrontBeamLogical = new GeoLogVol( H62002FrontBeamName, H62002FrontBeamShape, Air );

  m_H62002FrontBeamPhysical = new GeoPhysVol(H62002FrontBeamLogical);
  //m_H62002FrontBeamPhysical->add( new GeoNameTag("LArTBFrontBeamPos") );



 

  //------ W1,W2,B1 Scintillators
  //   In the old stand-alone code, all three were round with a radius of 5cm 
  //   and 7.5mm thickness.
  //   Logbooks in the control-room say that their xyz sizes are:
  //   B1   : 30 x 30 x 10 Gaudi::Units::mm
  //   W1,2 : 150 x 150 x 10 Gaudi::Units::mm
  // They are certainly not round, so stick with the logbook values 
  // The beam sees the instrumentation in the following order:
  // W1, W2, B1, MWPC5

  log << "Create Front Scintillators ..." << std::endl;
  
  const double Wxy=  75.0*Gaudi::Units::mm;
  const double Wz =   5.0*Gaudi::Units::mm;
  const double Bxy=  15.0*Gaudi::Units::mm;
  const double Bz =   5.0*Gaudi::Units::mm;

  std::vector<double> v_ScintXY;
  std::vector<double> v_ScintZ;
  v_ScintXY.push_back(Wxy);
  v_ScintXY.push_back(Wxy); 
  v_ScintXY.push_back(Bxy); 
  v_ScintZ.push_back(170.*Gaudi::Units::mm); 
  v_ScintZ.push_back(200.*Gaudi::Units::mm); 
  v_ScintZ.push_back(340.*Gaudi::Units::mm);

   // Create one Scintillator and place it twice along z:
 
  GeoBox* ScintShapeW = new GeoBox(Wxy, Wxy, Wz);  
  GeoBox* ScintShapeB = new GeoBox(Bxy, Bxy, Bz);  
  std::string ScintName = baseName + "::Scintillator";
  GeoLogVol* WScintLogical = new GeoLogVol( ScintName, ScintShapeW, Scint );
  GeoLogVol* BScintLogical = new GeoLogVol( ScintName, ScintShapeB, Scint );
  GeoPhysVol* WScintPhysical = new GeoPhysVol( WScintLogical );    
  GeoPhysVol* BScintPhysical = new GeoPhysVol( BScintLogical );    
  //WScintPhysical->add( new GeoNameTag(ScintName) );
  //BScintPhysical->add( new GeoNameTag(ScintName) );
  for ( unsigned int i = 0; i < v_ScintZ.size(); i++ ) {
    m_H62002FrontBeamPhysical->add( new GeoIdentifierTag(i) );
    m_H62002FrontBeamPhysical->add( new GeoTransform( GeoTrf::Translate3D( 0.*Gaudi::Units::cm, 0.*Gaudi::Units::cm, (v_ScintZ[ i ]-H62002FrontBeamZ) ) ) ) ;     m_H62002FrontBeamPhysical->add( new GeoNameTag(ScintName) );

    switch(i) {
    case 0: case 1: { m_H62002FrontBeamPhysical->add( WScintPhysical ); break; }
    case 2:         { m_H62002FrontBeamPhysical->add( BScintPhysical ); break; }
    default: { throw std::runtime_error("H62002FrontBeam wants too many Scintillators!!");   break; }
    }
  }

  //----- Done with Scintillators




  //------ Get MWPC number 5 from LArGeoH6Cryostats
  const double MwpcPos = 605.*Gaudi::Units::mm;
  double WireStep = 2.*Gaudi::Units::mm;
  MWPCConstruction  mwpcXConstruction (WireStep);
  GeoVPhysVol* mwpcEnvelope = mwpcXConstruction.GetEnvelope();
  m_H62002FrontBeamPhysical->add(new GeoIdentifierTag(5));
  m_H62002FrontBeamPhysical->add( new GeoTransform(GeoTrf::Translate3D( 0.*Gaudi::Units::cm, 0.*Gaudi::Units::cm, (MwpcPos-H62002FrontBeamZ) ) ) );
  m_H62002FrontBeamPhysical->add(mwpcEnvelope);    
  //------ Done with creating an MWPC from LArGeoH6Cryostats




  // End Moveable FrontBeam detectors


  return m_H62002FrontBeamPhysical;
}



