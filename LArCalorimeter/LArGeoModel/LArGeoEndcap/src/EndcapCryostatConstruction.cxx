/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EndcapCryostatConstruction

// 04-Jan-2002 WGS: Revised to create cryostat mother volume, and to
// place all endcap components (EMEC, HEC, FCAL) within this volume.

#include "LArGeoEndcap/EndcapCryostatConstruction.h"
#include "LArGeoEndcap/EndcapPresamplerConstruction.h"
#include "LArGeoEndcap/MbtsReadoutBuilder.h"
#include "EndcapDMConstruction.h"

#include "LArGeoHec/HECWheelConstruction.h"

#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoXF.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelUtilities/StoredPhysVol.h"
#include "GeoModelUtilities/GeoDBUtils.h"
#include "GeoModelUtilities/StoredAlignX.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoShapeShift.h"

#include "GeoGenericFunctions/AbsFunction.h"
#include "GeoGenericFunctions/Variable.h"

// For the database:
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "GeoModelInterfaces/IGeoModelSvc.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <cmath>
#include <iomanip>
#include <map>
#include <string>
#include <utility>

using namespace GeoGenfun;
using namespace GeoXF;



// The objects for mapping plane indexes in Pcon to the record index
// in RDBRecordset
using planeIndMap = std::map<int, unsigned int, std::less<int>>;


LArGeo::EndcapCryostatConstruction::EndcapCryostatConstruction(
    bool fullGeo, std::string emecVariantInner,
    std::string emecVariantOuter, bool activateFT, bool enableMBTS
) :
  //  cryoEnvelopePhysical(NULL),
  m_fcalVisLimit(-1),
  m_pAccessSvc(nullptr),
  m_geoModelSvc(nullptr),
  m_fullGeo(fullGeo),
  m_EMECVariantInner(std::move(emecVariantInner)),
  m_EMECVariantOuter(std::move(emecVariantOuter)),
  m_activateFT(activateFT),
  m_enableMBTS(enableMBTS)
{

  m_fcal = new FCALConstruction();

  ISvcLocator *svcLocator = Gaudi::svcLocator();
  StatusCode sc;
  sc=svcLocator->service("RDBAccessSvc",m_pAccessSvc);
  if (sc != StatusCode::SUCCESS) {
    throw std::runtime_error ("Cannot locate RDBAccessSvc!!");
  }

  sc = svcLocator->service ("GeoModelSvc",m_geoModelSvc);
  if (sc != StatusCode::SUCCESS) {
    throw std::runtime_error ("Cannot locate GeoModelSvc!!");
  }
}

LArGeo::EndcapCryostatConstruction::~EndcapCryostatConstruction()
{
  delete m_fcal;
}


GeoFullPhysVol* LArGeo::EndcapCryostatConstruction::createEnvelope(bool bPos)
{
  // Get access to the material manager:

  ISvcLocator *svcLocator = Gaudi::svcLocator();
  IMessageSvc * msgSvc;
  if (svcLocator->service("MessageSvc", msgSvc, true )==StatusCode::FAILURE) {
    throw std::runtime_error("Error in EndcapCryostatConstruction, cannot access MessageSvc");
  }

  MsgStream log(msgSvc, "LArGeo::EndcapCryostatConstruction");
  log << MSG::DEBUG << "started" << endmsg;


  StoreGateSvc *detStore;
  if (svcLocator->service("DetectorStore", detStore, false )==StatusCode::FAILURE) {
    throw std::runtime_error("Error in EndcapCryostatConstruction, cannot access DetectorStore");
  }

  // Get the materials from the material manager:-----------------------------------------------------//
  //                                                                                                  //
  StoredMaterialManager* materialManager = nullptr;
  if (StatusCode::SUCCESS != detStore->retrieve(materialManager, std::string("MATERIALS"))) return nullptr;

  const GeoMaterial *Lead = materialManager->getMaterial("std::Lead");
  if (!Lead) {
    throw std::runtime_error("Error in EndcapCryostatConstruction, std::Lead is not found.");
  }

  const GeoMaterial *Air  = materialManager->getMaterial("std::Air");
  if (!Air) {
    throw std::runtime_error("Error in EndcapCryostatConstruction, std::Air is not found.");
  }

  const GeoMaterial *Al  = materialManager->getMaterial("std::Aluminium");
  if (!Al) {
    throw std::runtime_error("Error in EndcapCryostatConstruction, std::Aluminium is not found.");
  }

  const GeoMaterial *LAr  = materialManager->getMaterial("std::LiquidArgon");
  if (!LAr) {
    throw std::runtime_error("Error in EndcapCryostatConstruction, std::LiquidArgon is not found.");
  }

  const GeoMaterial *G10  = materialManager->getMaterial("LAr::G10");
  if (!G10) throw std::runtime_error("Error in EndcapCryostatConstruction, LAr::G10 is not found.");

  const GeoMaterial *Copper  = materialManager->getMaterial("std::Copper");
  if (!Copper) throw std::runtime_error("Error in EndcapCryostatConstruction, std::Copper is not found.");

  const GeoMaterial *Iron  = materialManager->getMaterial("std::Iron");
  if (!Iron) throw std::runtime_error("Error in EndcapCryostatConstruction, std::Iron is not found.");

  const GeoMaterial *Polystyrene  = materialManager->getMaterial("std::Polystyrene");
  if (!Polystyrene) throw std::runtime_error("Error in EndcapCryostatConstruction, std::Polystyrene is not found.");

  //                                                                                                 //
  //-------------------------------------------------------------------------------------------------//

  std::string AtlasVersion = m_geoModelSvc->atlasVersion();
  std::string LArVersion = m_geoModelSvc->LAr_VersionOverride();

  std::string detectorKey  = LArVersion.empty() ? AtlasVersion : LArVersion;
  std::string detectorNode = LArVersion.empty() ? "ATLAS" : "LAr";

  IRDBRecordset_ptr cryoCylinders =  m_pAccessSvc->getRecordsetPtr("CryoCylinders",detectorKey, detectorNode);
  IRDBRecordset_ptr larPosition  =  m_pAccessSvc->getRecordsetPtr("LArPosition",detectorKey, detectorNode);
  if (larPosition->size()==0 ) {
    larPosition = m_pAccessSvc->getRecordsetPtr("LArPosition", "LArPosition-00");
    if (larPosition->size()==0 ) {
      throw std::runtime_error("Error, no lar position table in database!");
    }
  }




  if(cryoCylinders->size()==0) cryoCylinders = m_pAccessSvc->getRecordsetPtr("CryoCylinders","CryoCylinders-00");

  // Deal with Pcons
  IRDBRecordset_ptr cryoPcons = m_pAccessSvc->getRecordsetPtr("CryoPcons",detectorKey, detectorNode);
  if(cryoPcons->size()==0) cryoPcons = m_pAccessSvc->getRecordsetPtr("CryoPcons","CryoPcons-00");

  planeIndMap cryoMotherPlanes, emhPlanes, fcalNosePlanes;
  std::vector<planeIndMap> brassPlugPlanesVect;
  brassPlugPlanesVect.emplace_back();
  brassPlugPlanesVect.emplace_back();
  planeIndMap::const_iterator iter;

  for (unsigned int ind=0; ind<cryoPcons->size(); ind++)
  {
    int key = (*cryoPcons)[ind]->getInt("PLANE_ID");
    const std::string& pconName = (*cryoPcons)[ind]->getString("PCON");
    if(pconName=="Endcap::CryoMother") {
      cryoMotherPlanes[key] = ind;
    }
    else if(pconName=="Endcap::EMH") {
      emhPlanes[key] = ind;
    }
    else if(pconName=="Endcap::FcalNose") {
      fcalNosePlanes[key] = ind;
    }
    else if(pconName=="Endcap::BrassPlug1") {
      brassPlugPlanesVect[0][key] = ind;
    }
    else if(pconName=="Endcap::BrassPlug2") {
      brassPlugPlanesVect[1][key] = ind;
    }
  }


  //////////////////////////////////////////////////////////////////
  // Define geometry
  //////////////////////////////////////////////////////////////////

  // Set up strings for volume names.
  std::string baseName = "LAr::Endcap::Cryostat";

  // Define the mother volume for the endcap cryostat.  Everything
  // else in the endcap (cryostat walls, detectors, etc.) should be
  // placed inside here.

  // The position of this volume may change if the thickness of the
  // cabling in front of the endcaps changes.  Therefore, we must get
  // the z-shift from the database and adjust the volume geometry
  // accordingly.

  std::string cryoMotherName = baseName + "::MotherVolume";
  GeoPcon* cryoMotherShape = new GeoPcon(0.,2.*M_PI);

  double zStartCryoMother = 0.; // This variable is needed to calculate local transform of the MBTS mother

  for(unsigned int ind = 0; ind < cryoMotherPlanes.size(); ++ ind){
    iter = cryoMotherPlanes.find(ind);
    if(iter == cryoMotherPlanes.end()){
      throw std::runtime_error(
        "Error in EndcapCryostatConstruction, missing plane in Endcap Cryo Mother"
      );
    } else {
      const IRDBRecord *currentRecord = (*cryoPcons)[(*iter).second];
      double zplane = currentRecord->getDouble("ZPLANE");
      double rmin = currentRecord->getDouble("RMIN");
      double rmax =  currentRecord->getDouble("RMAX");
      /* This sould be corrected in the DB, but we have no time */
      if(m_activateFT){
        if(zplane == 0.){
          zplane = 12.;
          log << MSG::DEBUG << "Put cryoMother zplane " << ind
		          << " at " << zplane << " to accomodate FEC" << endmsg;
        }
        if(rmax == 2476.){
          rmax = 2506.;
          log << MSG::DEBUG << "Put cryoMother rmax " << ind
		          << " at " << rmax << " to accomodate FT Chimney" << endmsg;
        }
      }
      cryoMotherShape->addPlane(zplane, rmin, rmax);
      if(ind == 0) zStartCryoMother = zplane;
    }
  }

  const GeoLogVol* cryoMotherLogical = new GeoLogVol(cryoMotherName, cryoMotherShape, Air);
  GeoFullPhysVol* cryoMotherPhysical = new GeoFullPhysVol(cryoMotherLogical);

  //JT. 04.2013
  // insert extra material in form of Tubes
  // between warm and cold wall in front of Emec Presampler
  //       ( walls : warm(cylNumber=3), cold(cylNumber=14)
  //        in case of negative value of the thickness, no volume will be created
  //

  //  Extra cylinders

  IRDBRecordset_ptr cryoExtraCyl = m_pAccessSvc->getRecordsetPtr("LArCones",detectorKey, detectorNode);

  if(m_fullGeo && cryoCylinders->size()>0){
    unsigned int nextra=cryoExtraCyl->size();
    if(nextra>0){
      log << MSG::DEBUG << "activate extra material in front of PS" << endmsg;
      bool finloop=false;
      for(unsigned int i=0;i<nextra;i++){
        const std::string& name=(*cryoExtraCyl)[i]->getString("CONE");
          if(name.find("EmecCylBeforePS") != std::string::npos){
            double rmin=(*cryoExtraCyl)[i]->getDouble("RMIN1"); //PS rmin
            double rmax=(*cryoExtraCyl)[i]->getDouble("RMAX1"); //PS rmax
            double dz = (*cryoExtraCyl)[i]->getDouble("DZ");    //leadthickness
            if(dz>0.){

              double rmin_warm=0.,rmax_warm=0.,dz_warm=0.,zInCryostat_warm=0.;
              double rmin_cold=0.,rmax_cold=0.,dz_cold=0.,zInCryostat_cold=0.;
              int wallfind=0;

              for (unsigned int layer = 0; layer < cryoCylinders->size(); layer++) {
                const IRDBRecord *currentRecord = (*cryoCylinders)[layer];
                int cylNumber = currentRecord->getInt("CYL_NUMBER");
                if(currentRecord->getString("CYL_LOCATION")=="Endcap"){
                  if(cylNumber  == 3 )
                    {
                      rmin_warm=currentRecord->getDouble("RMIN")*Gaudi::Units::cm;
                      rmax_warm=currentRecord->getDouble("RMIN")*Gaudi::Units::cm + currentRecord->getDouble("DR")*Gaudi::Units::cm;
                      dz_warm=currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.;
                      zInCryostat_warm = currentRecord->getDouble("ZMIN")*Gaudi::Units::cm + currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.;
                      wallfind=wallfind+1;
                    }
                  if(cylNumber  == 14 )
                    {
                      rmin_cold=currentRecord->getDouble("RMIN")*Gaudi::Units::cm;
                      rmax_cold=currentRecord->getDouble("RMIN")*Gaudi::Units::cm + currentRecord->getDouble("DR")*Gaudi::Units::cm;
                      dz_cold=currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.;
                      zInCryostat_cold = currentRecord->getDouble("ZMIN")*Gaudi::Units::cm + currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.;
                      wallfind=wallfind+1;
                    }
                }
              }
              if(wallfind==2){
                double maxdz=(zInCryostat_cold-dz_cold)-(zInCryostat_warm+dz_warm);
                if(dz>maxdz) dz=maxdz;
                double zpos=((zInCryostat_cold-dz_cold)+(zInCryostat_warm+dz_warm))/2.;

                std::ostringstream cylStream;
                cylStream << baseName << "::ExtraCyl";
                std::string cylName = cylStream.str();
                cylName = cylName + "_beforePS";

                double phi0=(*cryoExtraCyl)[i]->getDouble("PHI0");
                double dphi=(*cryoExtraCyl)[i]->getDouble("DPHI");
                if(dphi>6.28) dphi=2.*M_PI;
                const std::string& material=(*cryoExtraCyl)[i]->getString("MATERIAL"); //lead
                const GeoMaterial *mat = materialManager->getMaterial(material);
                if (!mat) {
                  throw std::runtime_error("Error in EndcapCryostatConstruction,material for CylBeforePS is not found.");
                }

                GeoTubs         *solidCyl = new GeoTubs(rmin,rmax,dz/2.,phi0,dphi);
                const GeoLogVol *logicCyl = new GeoLogVol(cylName,solidCyl,mat);
                GeoPhysVol      *physCyl  = new GeoPhysVol(logicCyl);

                cryoMotherPhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(zpos)));
                cryoMotherPhysical->add(physCyl);

                std::cout<<"**************************************************"<<std::endl;
                std::cout<<"EndcapCryostatConstruction insert extra material between warm and cold wall of cryo at front of PS::"<<std::endl;
                std::cout<<"  ExtraCyl params: name,mat= "<<cylName<<" "<<mat->getName()
                         <<" rmin,rmax,dzthick,zpos="<<rmin<<" "<<rmax<<" "<<dz<<" "<<zpos
                         <<" PhiStart,PhiSize="<<phi0<<" "<<dphi
                         <<std::endl;

                std::cout<<"  warm cyl params: rmin,rmax,dzthick,zpos="<<rmin_warm<<" "<<rmax_warm<<" "<<2.*dz_warm<<" "
                         <<zInCryostat_warm<<std::endl;
                std::cout<<"  cold cyl params: rmin,rmax,dzthick,zpos="<<rmin_cold<<" "<<rmax_cold<<" "<<2.*dz_cold<<" "
                         <<zInCryostat_cold<<std::endl;
                std::cout<<"**************************************************"<<std::endl;

                finloop=true;
              } // warm cold walls found
            }  // dz>0. , extra cyl. thickness is positive
          }   //CylBeforePS ; extracyl is defined in the db record
          if(finloop) break;
      }   // loop for extra cyls in the db record
    }  // number of items in the db record >0
    else {
      log << MSG::DEBUG << "no extra material in front of PS" << endmsg;
    }
  }  // fullgeo is required and cryocyl reciord is not empty

    // end of inserting extra lead plate before PS


  IRDBRecordset_ptr LArEndcapCratePhiPos = m_pAccessSvc->getRecordsetPtr("LArEndcapCratePhiPos",detectorKey, detectorNode);


  for(unsigned int layer = 0; layer < cryoCylinders->size(); layer++){
    const IRDBRecord *currentRecord = (*cryoCylinders)[layer];
    int cylNumber = currentRecord->getInt("CYL_NUMBER");
      if(m_fullGeo || cylNumber == 14 || cylNumber == 100) {
      // 100 - is the piece of Shielding. We need to build it for minimal geo too

      if(currentRecord->getString("CYL_LOCATION")=="Endcap") {
        std::ostringstream cylStream;
        cylStream << baseName << "::Cylinder";
        std::string cylName = (cylNumber == 100?"JDSH_AddShield_Inner":cylStream.str());

        if(!currentRecord->isFieldNull("QUALIFIER")){
          const std::string& qualifier = currentRecord->getString("QUALIFIER");
          if (!qualifier.empty()) cylName = cylName + "::" + qualifier;
        }

        const GeoShape* solidCyl = new GeoTubs(
          currentRecord->getDouble("RMIN")*Gaudi::Units::cm,
          currentRecord->getDouble("RMIN")*Gaudi::Units::cm + currentRecord->getDouble("DR")*Gaudi::Units::cm,
          currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.,
          (double) 0.,
          (double) 2.*M_PI*Gaudi::Units::rad
        );
        const GeoMaterial *material = materialManager->getMaterial(currentRecord->getString("MATERIAL"));

        if(!material){
          std::ostringstream errorMessage;
          errorMessage << "Error in EndcapCrysostat Construction" << std::endl;
          errorMessage << "Material " << currentRecord->getString("MATERIAL") << " is not found" << std::endl;
          throw std::runtime_error(errorMessage.str().c_str());
        }

        if(m_activateFT){ // need to cut holes in cryostat walls for FT
          if(cylNumber == 13){ // warm wall
            log << MSG::DEBUG << "Cut holes for feedthroughs in warm wall "
                << cylName
                << endmsg;
            const double rmin = currentRecord->getDouble("RMIN")*Gaudi::Units::cm;
            const double rmax = currentRecord->getDouble("RMIN")*Gaudi::Units::cm + currentRecord->getDouble("DR")*Gaudi::Units::cm;
            const double dz = currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.;
            const double warmhole_radius = 0.5*340.*Gaudi::Units::mm;
            const double warmhole_pos = dz - 247.*Gaudi::Units::mm;
            GeoTube *warmhole = new GeoTube(0., warmhole_radius, (rmax - rmin) * 4);
            const GeoShapeShift &h1 = (*warmhole) << GeoTrf::RotateX3D(90*Gaudi::Units::deg);
            const double r = (rmin + rmax) * 0.5;
            const GeoShape* warmwall = solidCyl;
            const double dphi = 5.*Gaudi::Units::deg;
            auto put = [&warmwall, &warmhole_pos, &r, &h1](double pos){
              const double x = r*cos(pos), y = r*sin(pos);
              warmwall = &(warmwall->subtract(
                h1 << GeoTrf::Translate3D(x, y, warmhole_pos)
                     *GeoTrf::RotateZ3D(pos + 90*Gaudi::Units::deg)
              ));
            };
            for(unsigned int i{0}; i < LArEndcapCratePhiPos->size(); ++ i){
              const int num = (*LArEndcapCratePhiPos)[i]->getInt("CRATENUM");
              const double phi = (*LArEndcapCratePhiPos)[i]->getDouble("PHIPOS")*Gaudi::Units::deg;
              if(num == 10){ // topmost crate has one FT
                put(phi + dphi); // asymmetric, see DMConstruction
              } else {
                put(phi - dphi);
                put(phi + dphi);
              }
            }
            solidCyl = warmwall;
          } else if(cylNumber == 20){ // cold wall
            log << MSG::DEBUG << "Cut holes for feedthroughs in cold wall "
                << cylName
                << endmsg;
            const double rmin = currentRecord->getDouble("RMIN")*Gaudi::Units::cm;
            const double rmax = currentRecord->getDouble("RMIN")*Gaudi::Units::cm + currentRecord->getDouble("DR")*Gaudi::Units::cm;
            const double coldhole_radius = 0.5*150.*Gaudi::Units::mm;
            const double coldhole_pos = 21.5*Gaudi::Units::mm;
            GeoTube *coldhole = new GeoTube(0., coldhole_radius, (rmax - rmin) * 4);
            const GeoShapeShift &h1 = (*coldhole) << GeoTrf::RotateX3D(90*Gaudi::Units::deg);
            const double r = (rmin + rmax) * 0.5;
            const GeoShape *coldwall = solidCyl;
            const double dphi = 5.*Gaudi::Units::deg;
            auto put = [&coldwall, &coldhole_pos, &r, &h1](double pos){
              const double x = r*cos(pos), y = r*sin(pos);
              coldwall = &(coldwall->subtract(
                h1 << GeoTrf::Translate3D(x, y, coldhole_pos)
                     *GeoTrf::RotateZ3D(pos + 90*Gaudi::Units::deg)
              ));
            };
            for(unsigned int i{0}; i < LArEndcapCratePhiPos->size(); ++ i){
              const int num = (*LArEndcapCratePhiPos)[i]->getInt("CRATENUM");
              const double phi = (*LArEndcapCratePhiPos)[i]->getDouble("PHIPOS")*Gaudi::Units::deg;
              if(num == 10){ // topmost crate has one FT
                put(phi + dphi); // asymmetric, see DMConstruction
              } else {
                put(phi - dphi);
                put(phi + dphi);
              }
            }
            solidCyl = coldwall;
          }
        }

        const GeoLogVol* logicCyl = new GeoLogVol(cylName,solidCyl,material);
        GeoPhysVol* physCyl = new GeoPhysVol(logicCyl);

        double zInCryostat = currentRecord->getDouble("ZMIN")*Gaudi::Units::cm
                           + currentRecord->getDouble("DZ")*Gaudi::Units::cm / 2.;
        // Don't move the pump even if the rest of the cryostat moves.

        //if ( cylNumber == 33 ) zInCryostat -= zEmec;

        // Place each cylinder.

        cryoMotherPhysical->add(new GeoIdentifierTag(cylNumber));
        cryoMotherPhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(zInCryostat)));

        // Front cold wall of Cryostat is a mother for Endcap Presampler
        if ( cylNumber == 14 ) {
          // its PhysicalVolume has a special name
          cryoMotherPhysical->add( new GeoNameTag(cylName + "::PresamplerMother") );

          EndcapPresamplerConstruction endcapPresamplerConstruction;

          GeoFullPhysVol* emecPSEnvelope = endcapPresamplerConstruction.Envelope();
          if ( emecPSEnvelope != nullptr ) {
            // Get the position of the presampler from the geometry helper.
            double Zpos = 30.5*Gaudi::Units::mm;

            // It is highly debateable whether the endcap presampler is
            // alignable, but in any case we shall not align it here because
            // we need to completely redo it, anyway, since it does  not
            // even live  "in" this volume, not in real life anyway.
            GeoTransform *xfPs = new GeoTransform(GeoTrf::TranslateZ3D(Zpos));

            physCyl->add(xfPs);
            physCyl->add( emecPSEnvelope );

            std::string tag = bPos? std::string("PRESAMPLER_EC_POS") : std::string("PRESAMPLER_EC_NEG");
            StatusCode status;

            StoredPhysVol *sPhysVol = new StoredPhysVol(emecPSEnvelope);
            status=detStore->record(sPhysVol,tag);
            if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag).c_str());
          }
        }

        // After we've added any additional sub-volumes, add the cylinder.
        cryoMotherPhysical->add(physCyl);
      }
    }
  }

  // g.p., 3-Apr-2006
  // placing Pcones for FCAL nose, instead of cylinders 34,28,29
  if(m_fullGeo)
    if (!fcalNosePlanes.empty()) {
      GeoPcon *fcalNosePcon = new GeoPcon(0,2*M_PI);
      for(unsigned int ind=0; ind<fcalNosePlanes.size(); ind++) {
	iter = fcalNosePlanes.find(ind);

	if(iter==fcalNosePlanes.end()) {
	  throw std::runtime_error("Error in BarrelCryostatConstruction, missing plane in InnerWall");
	} else {
	  const IRDBRecord *currentRecord = (*cryoPcons)[(*iter).second];
	  fcalNosePcon->addPlane(currentRecord->getDouble("ZPLANE"),
				 currentRecord->getDouble("RMIN"),
				 currentRecord->getDouble("RMAX"));
	}
      }
      int coneNumber = 1;
      const GeoLogVol *fcalNoseLog = new GeoLogVol("LAr::Endcap::Cryostat::Cone::Mixed", fcalNosePcon, Al);
      cryoMotherPhysical->add(new GeoIdentifierTag(coneNumber));
      GeoPhysVol *fcalNosePhys = new GeoPhysVol(fcalNoseLog);
      cryoMotherPhysical->add(fcalNosePhys);
    }


  // There are two LAr regions within the endcap cryostat.  The first
  // is the region that contains the EMEC and HEC (the electromagnetic
  // and hadronic endcap calorimeters, respectively).  The second is
  // the region that contains the forward calorimeter.

  // This volumes will be sub-divided into sensitive-detector regions
  // in the detector.

  // EMEC + HEC:
  std::string totalEMHLArName = baseName + "::EmecHecLAr";
  GeoPcon* totalEMHLArShape = new GeoPcon(0.,2.*M_PI);

  for(unsigned int ind=0; ind<emhPlanes.size(); ind++)
  {
    iter = emhPlanes.find(ind);

    if(iter==emhPlanes.end())
      throw std::runtime_error("Error in EndcapCryostatConstruction, missing plane in EMH");
    else
    {
      const IRDBRecord *currentRecord = (*cryoPcons)[(*iter).second];
      totalEMHLArShape->addPlane(currentRecord->getDouble("ZPLANE"),
			        currentRecord->getDouble("RMIN"),
			        currentRecord->getDouble("RMAX"));
    }
  }

  const GeoLogVol* totalEMHLArLogical =
    new GeoLogVol(totalEMHLArName, totalEMHLArShape, LAr);

  GeoFullPhysVol* totalEMHLArPhysical = new GeoFullPhysVol(totalEMHLArLogical);

  // Add brass plugs
  const GeoMaterial *PlugBrass(nullptr);
  for(size_t i(0);i<2;++i) {
    const planeIndMap& brassPlugPlanes = brassPlugPlanesVect[i];
    if (!brassPlugPlanes.empty()) {
      if(!PlugBrass) {
	PlugBrass  = materialManager->getMaterial("LAr::PlugBrass");
	if (!PlugBrass) throw std::runtime_error("Error in EndcapCryostatConstruction, LAr::PlugBrass is not found.");
      }
      GeoPcon *brassPlugPcon = new GeoPcon(0,2*M_PI);
      for(unsigned int ind=0; ind<brassPlugPlanes.size(); ind++) {
	iter = brassPlugPlanes.find(ind);

	if(iter==brassPlugPlanes.end()) {
	  throw std::runtime_error("Error in EndcapCryostatConstruction, missing plane in BrassPlug");
	} else {
	  const IRDBRecord *currentRecord = (*cryoPcons)[(*iter).second];
	  brassPlugPcon->addPlane(currentRecord->getDouble("ZPLANE"),
				  currentRecord->getDouble("RMIN"),
				  currentRecord->getDouble("RMAX"));
	}
      }
      const GeoLogVol *brassPlugLog = new GeoLogVol("LAr::Endcap::Cryostat::BrassPlug", brassPlugPcon, PlugBrass);
      GeoPhysVol *brassPlugPhys = new GeoPhysVol(brassPlugLog);
      totalEMHLArPhysical->add(new GeoIdentifierTag(i+1));
      totalEMHLArPhysical->add(brassPlugPhys);
    }
  }

  if(m_activateFT){
  // this ring emulates signal cables concentration area
  // nearby the theedtrougs inside the cryostat
    const double rcoldwall = 2155.*Gaudi::Units::mm;
    const double coldhole_radius = 0.5*150.*Gaudi::Units::mm; // copied from above
    const double icable_dz = coldhole_radius;
    const double icable_dr =
      (1920./LArEndcapCratePhiPos->size()) *
      M_PI * 1.1*1.1/4. * Gaudi::Units::mm2
      / (icable_dz*2);
    log << MSG::DEBUG << "adding " << icable_dr/Gaudi::Units::mm << " mm"
        << " of cables inside EC cryostat in front of FT" << endmsg;
    const double z_pos = -249.*Gaudi::Units::mm - icable_dz;
    const GeoMaterial* icable_mat = materialManager->getMaterial("LAr::FT::Cable");
    GeoShape* icable = new GeoTube(rcoldwall - icable_dr, rcoldwall, icable_dz);
    GeoLogVol* icableLV = new GeoLogVol("LAr::Endcap::InnerFTCables", icable, icable_mat);
    GeoPhysVol* icablePV = new GeoPhysVol(icableLV);
    totalEMHLArPhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(z_pos)));
    totalEMHLArPhysical->add(icablePV);
  }

  cryoMotherPhysical->add( totalEMHLArPhysical );

  {
    m_emec.setFullGeo(m_fullGeo);
    m_emec.setInnerVariant(m_EMECVariantInner);
    m_emec.setOuterVariant(m_EMECVariantOuter);
    GeoFullPhysVol *envelope = m_emec.GetEnvelope(bPos);

    //=>
    const IRDBRecord *posRec = GeoDBUtils::getTransformRecord(larPosition, bPos ? "EMEC_POS":"EMEC_NEG");
    if (!posRec) throw std::runtime_error("Error, no lar position record in the database") ;
    GeoTrf::Transform3D xfPos = GeoDBUtils::getTransform(posRec);
    GeoAlignableTransform *xfEmec = new GeoAlignableTransform(xfPos);

    std::string tag = bPos? std::string("EMEC_POS") : std::string("EMEC_NEG");
    StatusCode status;

    StoredPhysVol *sPhysVol = new StoredPhysVol(envelope);
    status=detStore->record(sPhysVol,tag);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag).c_str());

    StoredAlignX *sAlignX = new StoredAlignX(xfEmec);
    status=detStore->record(sAlignX,tag);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag).c_str());


    //=>
    totalEMHLArPhysical->add(xfEmec);
    totalEMHLArPhysical->add( envelope );
  }

  {

    std::string wheelType="front";
    bool threeBoards= false;
    HECWheelConstruction frontHEC(m_fullGeo,wheelType,threeBoards,bPos) ;
    GeoFullPhysVol* EnvelopeF = frontHEC.GetEnvelope();

    StatusCode status;


    //--- Make the Front Wheel alignable:

    const IRDBRecord *posHec1 = GeoDBUtils::getTransformRecord(larPosition, bPos ? "HEC1_POS":"HEC1_NEG");
    GeoTrf::Transform3D xfPosHec1 = posHec1 ? GeoDBUtils::getTransform(posHec1) : GeoTrf::Translate3D(0.,0.,-2423.0);
    GeoAlignableTransform *xfHec1 = new GeoAlignableTransform(xfPosHec1);

    std::string tag1 = bPos? std::string("HEC1_POS") : std::string("HEC1_NEG");

    StoredPhysVol *sPhysVol1 = new StoredPhysVol(EnvelopeF);
    status=detStore->record(sPhysVol1,tag1);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag1).c_str());

    StoredAlignX *sAlignX1 = new StoredAlignX(xfHec1);
    status=detStore->record(sAlignX1,tag1);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag1).c_str());

    totalEMHLArPhysical->add( xfHec1);
    totalEMHLArPhysical->add(new GeoIdentifierTag(0));
    totalEMHLArPhysical->add( EnvelopeF );



    wheelType="rear";
    threeBoards= false;
    HECWheelConstruction rearHEC(m_fullGeo,wheelType,threeBoards,bPos) ;
    GeoFullPhysVol* EnvelopeR = rearHEC.GetEnvelope();

    const IRDBRecord *posHec2 = GeoDBUtils::getTransformRecord(larPosition, bPos ? "HEC2_POS":"HEC2_NEG");
    GeoTrf::Transform3D xfPosHec2 = posHec2 ? GeoDBUtils::getTransform(posHec2) : GeoTrf::Translate3D(0.,0.,-1566.0);
    GeoAlignableTransform *xfHec2 = new GeoAlignableTransform(xfPosHec2);

    std::string tag2 = bPos? std::string("HEC2_POS") : std::string("HEC2_NEG");

    StoredPhysVol *sPhysVol2 = new StoredPhysVol(EnvelopeR);
    status=detStore->record(sPhysVol2,tag2);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag2).c_str());

    StoredAlignX *sAlignX2 = new StoredAlignX(xfHec2);
    status=detStore->record(sAlignX2,tag2);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag2).c_str());

    totalEMHLArPhysical->add( xfHec2);
    totalEMHLArPhysical->add(new GeoIdentifierTag(1));
    totalEMHLArPhysical->add( EnvelopeR );


  }



  // 13-Mar-2002 WGS: Place the FCAL detector inside the cryostat.
  m_fcal->setFCALVisLimit(m_fcalVisLimit);
  m_fcal->setFullGeo(m_fullGeo);
  {

    // The "envelope" determined by the EMB should be a GeoFullPhysVol.
    GeoVFullPhysVol* fcalEnvelope = m_fcal->GetEnvelope(bPos);

    /* For now, comment out the FCAL placement, for two reasons:
       1) The FCAL geometry helper class has been written yet;
       2) Reconstruction needs each FCAL module to have a
       separate alignable envelope.  We'll work out t hese
       issues later. */


    // From FCALConstruction.cxx: */
   //

    std::string tag = bPos ? "FCAL_POS" : "FCAL_NEG";
    // Get default values for alignable transform deltas from SubdetPosHelper
    const IRDBRecord *posRec = GeoDBUtils::getTransformRecord(larPosition, tag);
    if (!posRec) throw std::runtime_error("Error, no lar position record in the database") ;
    GeoTrf::Transform3D xfPos = GeoDBUtils::getTransform(posRec);
    GeoAlignableTransform *fcalXF = new GeoAlignableTransform(xfPos);

    StatusCode status;
    StoredAlignX *sAlignX = new StoredAlignX(fcalXF);
    status=detStore->record(sAlignX,tag);
    if(!status.isSuccess()) throw std::runtime_error ((std::string("Cannot store")+tag).c_str());


    const GeoLogVol *envVol = fcalEnvelope->getLogVol();
    const GeoShape  *envShape = envVol->getShape();
    if (envShape->typeID()!=GeoTubs::getClassTypeID()) {
      throw std::runtime_error("Cannot recognize FCAL container shape");
    }

    const GeoTubs * tubs = (const GeoTubs *) envShape;

    // Place the FCAL modules.
    cryoMotherPhysical->add(fcalXF);
    cryoMotherPhysical->add( new GeoTransform( GeoTrf::TranslateZ3D(tubs->getZHalfLength()) ) );
    cryoMotherPhysical->add( fcalEnvelope );


  }

  //__________________________ MBTS+moderator+JM tube _____________________________________
  if(m_enableMBTS && !m_pAccessSvc->getChildTag("MBTS",detectorKey, detectorNode).empty()) {
    // DB related stuff first
    IRDBRecordset_ptr mbtsTubs   = m_pAccessSvc->getRecordsetPtr("MBTSTubs", detectorKey, detectorNode);
    IRDBRecordset_ptr mbtsScin   = m_pAccessSvc->getRecordsetPtr("MBTSScin", detectorKey, detectorNode);
    IRDBRecordset_ptr mbtsPcons  = m_pAccessSvc->getRecordsetPtr("MBTSPcons",detectorKey, detectorNode);
    IRDBRecordset_ptr mbtsGen    = m_pAccessSvc->getRecordsetPtr("MBTSGen",  detectorKey, detectorNode);
    IRDBRecordset_ptr mbtsTrds   = m_pAccessSvc->getRecordsetPtr("MBTSTrds", detectorKey, detectorNode);

    double zposMM = 0.;
    std::map<std::string,unsigned> trdMap;  // Used in the new description only
    for(unsigned indTrd(0);indTrd<mbtsTrds->size();++indTrd) {
      const std::string& keyTrd = (*mbtsTrds)[indTrd]->getString("TRD");
      trdMap[keyTrd]=indTrd;
    }

    // Build material geometry only if the FullGeo flag has been set
    if(m_fullGeo) {
      // Define iterators
      IRDBRecordset::const_iterator itMother = mbtsTubs->end();
      IRDBRecordset::const_iterator itModerator = mbtsTubs->end();
      IRDBRecordset::const_iterator itTube=mbtsTubs->end();

      IRDBRecordset::const_iterator first = mbtsTubs->begin();
      IRDBRecordset::const_iterator last = mbtsTubs->end();

      // Mother volume
      GeoPhysVol* pvMM = nullptr;

      if(mbtsPcons->size()==0) {
	// ****
	// In this description the Moderator and the JM tube are constructed as separate volumes (both of them are tubes)
	// ****

	for(; first!=last; ++first) {
	  const std::string& strTubeName = (*first)->getString("TUBE");
	  if(strTubeName == "MBTS_mother") {
	    itMother = first;
	  }
	  else if(strTubeName == "Moderator") {
	    itModerator = first;
	  }
	  else if(strTubeName == "JMTUBE") {
	    itTube = first;
	  }
	}

	// Build mother volume
	double rminMM = (*itMother)->getDouble("RMIN")*Gaudi::Units::mm;
	double rmaxMM = (*itMother)->getDouble("RMAX")*Gaudi::Units::mm;
	double dzMM = (*itMother)->getDouble("DZ")*Gaudi::Units::mm;
	zposMM = (*itMother)->getDouble("ZPOS")*Gaudi::Units::mm;

	const GeoMaterial *matMM  = materialManager->getMaterial((*itMother)->getString("MATERIAL"));

	GeoTube  *tubeMM = new GeoTube(rminMM,rmaxMM,dzMM);

	GeoTube *tubeJM=nullptr;
	const GeoShape *solidMM=nullptr;
	if (itTube!=mbtsTubs->end()) {
	  double dzMod   = (*itTube)->getDouble("DZ")*Gaudi::Units::mm;
	  double rMaxMod = (*itTube)->getDouble("RMAX")*Gaudi::Units::mm;

	  GeoPcon *pcon = new GeoPcon(0,2*M_PI);
	  pcon->addPlane(-dzMM,rminMM,rmaxMM);
	  pcon->addPlane( dzMM,rminMM,rmaxMM);
	  pcon->addPlane( dzMM,rminMM,rMaxMod);
	  pcon->addPlane( dzMM+2*dzMod, rminMM,rMaxMod);
	  tubeJM = new GeoTube(rminMM,rMaxMod,dzMod);
	  solidMM=pcon;
	}

	if (!solidMM)  solidMM = new GeoTube(rminMM,rmaxMM,dzMM);

	GeoLogVol* lvMM = new GeoLogVol("MBTS_mother",solidMM,matMM);
	pvMM = new GeoPhysVol(lvMM);

	cryoMotherPhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(zposMM)));
	cryoMotherPhysical->add(pvMM);

	// Moderator cylinder
	//double rminMod  = (*itModerator)->getDouble("RMIN")*Gaudi::Units::mm;
	//double rmaxMod = (*itModerator)->getDouble("RMAX")*Gaudi::Units::mm;
	double dzMod = (*itModerator)->getDouble("DZ")*Gaudi::Units::mm;
	double zposMod = (*itModerator)->getDouble("ZPOS")*Gaudi::Units::mm;

	const GeoMaterial *matMod  = materialManager->getMaterial((*itModerator)->getString("MATERIAL"));

	GeoTube* solidMod = new GeoTube(rminMM,rmaxMM,dzMod);
	GeoLogVol* lvMod = new GeoLogVol("Moderator",solidMod, matMod);
	GeoPhysVol* pvMod = new GeoPhysVol(lvMod);

	pvMM->add(new GeoTransform(GeoTrf::TranslateZ3D(zposMod)));
	pvMM->add(pvMod);

	if (tubeJM) {
	  GeoLogVol* lvMod  = new GeoLogVol("ModeratorTube",tubeJM, matMod);
	  GeoPhysVol* pvMod = new GeoPhysVol(lvMod);

	  pvMM->add(new GeoTransform(GeoTrf::TranslateZ3D(tubeMM->getZHalfLength()+tubeJM->getZHalfLength())));
	  pvMM->add(pvMod);
	}
	tubeMM->ref();  tubeMM->unref();
      } else {
	// ****
	// In this description the Moderator and the JM tube are constructed one polycone + one extra part for the moderator
	// ****

	planeIndMap mbtsMotherPlanes, jmTubePlanes;
	planeIndMap::const_iterator iter;

	for (unsigned int ind=0; ind<mbtsPcons->size(); ind++) {
	  int key = (*mbtsPcons)[ind]->getInt("PLANE_ID");
	  const std::string& pconName = (*mbtsPcons)[ind]->getString("PCON");
	  if(pconName=="MBTS::Mother") {
	    mbtsMotherPlanes[key] = ind;
	  }
	  else if(pconName=="MBTS::JM") {
	    jmTubePlanes[key] = ind;
	  }
	}

	double zStartMM = 0.;

	// construct shapes for the MBTS mother and Moderator+JM
	GeoPcon* solidMM = new GeoPcon(0.,2.*M_PI);
	for(unsigned int ind=0; ind<mbtsMotherPlanes.size(); ind++) {
	  iter = mbtsMotherPlanes.find(ind);
	  if(iter==mbtsMotherPlanes.end())
	    throw std::runtime_error("Error in EndcapCryostatConstruction, missing plane in MBTS Mother");
	  else {
	    const IRDBRecord *currentRecord = (*mbtsPcons)[(*iter).second];
	    solidMM->addPlane(currentRecord->getDouble("ZPLANE"),
			      currentRecord->getDouble("RMIN"),
			      currentRecord->getDouble("RMAX"));
	    if(ind==0)
	      zStartMM = currentRecord->getDouble("ZPLANE");
	  }
	}


	GeoPcon* solidMod = new GeoPcon(0.,2.*M_PI);
	for(unsigned int ind=0; ind<jmTubePlanes.size(); ind++) {
	  iter = jmTubePlanes.find(ind);
	  if(iter==jmTubePlanes.end())
	    throw std::runtime_error("Error in EndcapCryostatConstruction, missing plane in Moderator+JM tube volume");
	  else {
	    const IRDBRecord *currentRecord = (*mbtsPcons)[(*iter).second];
	    solidMod->addPlane(currentRecord->getDouble("ZPLANE"),
			      currentRecord->getDouble("RMIN"),
			      currentRecord->getDouble("RMAX"));
	  }
	}

	// Construct volumes:

	// Mother
	GeoLogVol* lvMM = new GeoLogVol("MBTS_mother",solidMM,Air);
	pvMM = new GeoPhysVol(lvMM);

	zposMM = zStartCryoMother - zStartMM;
	cryoMotherPhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(zposMM)));
	cryoMotherPhysical->add(pvMM);

	// Extra tube for the moderator:
	if((mbtsTubs->size()!=1) || ((*mbtsTubs)[0]->getString("TUBE")!="MBTS::JM"))
	  throw std::runtime_error("Error in EndcapCryostatConstruction, unexpected number of tubes or wrong name for the JM tube");
	GeoTube* tubeJM = new GeoTube((*mbtsTubs)[0]->getDouble("RMIN"),
				      (*mbtsTubs)[0]->getDouble("RMAX"),
				      (*mbtsTubs)[0]->getDouble("DZ"));
	const GeoMaterial* matJM  = materialManager->getMaterial((*mbtsTubs)[0]->getString("MATERIAL"));
	GeoLogVol* lvJM = new GeoLogVol("ModeratorJMTube",tubeJM, matJM);
	GeoPhysVol* pvJM = new GeoPhysVol(lvJM);

	pvMM->add(new GeoTransform(GeoTrf::TranslateZ3D((*mbtsTubs)[0]->getDouble("ZPOS"))));
	pvMM->add(pvJM);

	// Moderator+JM polycone
	GeoLogVol* lvMod = new GeoLogVol("ModeratorJMPcon",solidMod, matJM);
	GeoPhysVol* pvMod = new GeoPhysVol(lvMod);

	pvMM->add(pvMod);
      }

      // Scintillators
      if(mbtsGen->size()==0) {
	// The "old" description: just scintillators, no aluminum envelopes
	for(unsigned int scinId=0; scinId<mbtsScin->size(); scinId++) {
	  const IRDBRecord* curScin = (*mbtsScin)[scinId];

	  int nScin = curScin->getInt("SCINNUM");
	  double dx1Scin = curScin->getDouble("DX1")*Gaudi::Units::mm;
	  double dx2Scin = curScin->getDouble("DX2")*Gaudi::Units::mm;
	  double dy1Scin = curScin->getDouble("DY1")*Gaudi::Units::mm;
	  double dy2Scin = curScin->getDouble("DY2")*Gaudi::Units::mm;
	  double dzScin  = curScin->getDouble("DZ")*Gaudi::Units::mm;
	  double zposScin = curScin->getDouble("ZPOS")*Gaudi::Units::mm;
	  double rposScin = curScin->getDouble("RPOS")*Gaudi::Units::mm;

	  double startPhi = 0.;
	  try {
	    if(!curScin->isFieldNull("STARTPHI"))
	      startPhi = curScin->getDouble("STARTPHI");
	  }
	  catch(std::runtime_error&) {}

	  const GeoMaterial *matScin  = materialManager->getMaterial(curScin->getString("MATERIAL"));

	  std::ostringstream ostr;
	  ostr << curScin->getInt("SCIN_ID");
	  std::string scinName = std::string("MBTS")+ostr.str();

	  GeoTrd* solidScin = new GeoTrd(dx1Scin,dx2Scin,dy1Scin,dy2Scin,dzScin);
	  GeoLogVol* lvScin = new GeoLogVol(scinName,solidScin,matScin);
	  GeoPhysVol* pvScin = new GeoPhysVol(lvScin);

	  // parameterizations
	  double deltaPhi = 360./nScin;
	  Variable varInd;
	  GeoSerialTransformer* stScin = nullptr;

	  if(bPos) {
	    GENFUNCTION phiInd = deltaPhi*(varInd + startPhi)*Gaudi::Units::deg;
	    TRANSFUNCTION xfScin = Pow(GeoTrf::RotateZ3D(1.0),phiInd)*GeoTrf::TranslateZ3D(zposScin)*GeoTrf::TranslateX3D(rposScin)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
	    stScin = new GeoSerialTransformer(pvScin,&xfScin,nScin);
	  } else {
	    GENFUNCTION phiInd = (180 - deltaPhi*(varInd + startPhi))*Gaudi::Units::deg;
	    TRANSFUNCTION xfScin = Pow(GeoTrf::RotateZ3D(1.0),phiInd)*GeoTrf::TranslateZ3D(zposScin)*GeoTrf::TranslateX3D(rposScin)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
	    stScin = new GeoSerialTransformer(pvScin,&xfScin,nScin);
	  }

	  pvMM->add(new GeoSerialIdentifier(0));
	  pvMM->add(stScin);
	}
      }
      else {
	// The "new" description: scintillators + aluminum envelopes + plastic plugs + aluminum press bars (only for RUN1)

	// General parameters
	int nAirEnv = (*mbtsGen)[0]->getInt("NSCIN");
	double startPhi = (*mbtsGen)[0]->getDouble("STARTPHI");
	double zposAirEnv = (*mbtsGen)[0]->getDouble("ZPOSENV");
	double rposAirEnv = (*mbtsGen)[0]->getDouble("RPOSENV");

	GeoPhysVol *pvAirEnv(nullptr),*pvAluEnv(nullptr),*pvAirInAlu(nullptr);

	// Build the air envelope first
	std::map<std::string,unsigned>::const_iterator itTrdMap = trdMap.find("MBTSAirEnv");
	if(itTrdMap==trdMap.end())
	  throw std::runtime_error("Error in EndcapCryostatConstruction, unable to get MBTS air envelope parameters from the database!");
	const IRDBRecord* rec = (*mbtsTrds)[itTrdMap->second];
	pvAirEnv = buildMbtsTrd(rec,materialManager,nullptr);

	// Build direct children of the air envelope
	for(itTrdMap=trdMap.begin();itTrdMap!=trdMap.end();++itTrdMap) {
	  rec = (*mbtsTrds)[itTrdMap->second];
	  const std::string& trd = rec->getString("TRD");
	  if(rec->getString("PARENT")=="MBTSAirEnv") {
	    GeoPhysVol* nevVol = buildMbtsTrd(rec,materialManager,pvAirEnv);
	    if(trd.compare("MBTSAluEnv")==0)
	      pvAluEnv = nevVol;
	  }
	}

	// Build direct children of the aluminum envelope
	for(itTrdMap=trdMap.begin();itTrdMap!=trdMap.end();++itTrdMap) {
	  rec = (*mbtsTrds)[itTrdMap->second];
	  const std::string& trd = rec->getString("TRD");
	  if(rec->getString("PARENT")=="MBTSAluEnv") {
	    GeoPhysVol* nevVol = buildMbtsTrd(rec,materialManager,pvAluEnv);
	    if(trd.compare("MBTSAirInAlu")==0)
	      pvAirInAlu = nevVol;
	  }
	}

	// Build direct children of the 'air-in-aluminum'
	for(itTrdMap=trdMap.begin();itTrdMap!=trdMap.end();++itTrdMap) {
	  rec = (*mbtsTrds)[itTrdMap->second];
	  if(rec->getString("PARENT")=="MBTSAirInAlu")
	     buildMbtsTrd(rec,materialManager,pvAirInAlu);
	}

	// parameterizations
	double deltaPhi = 360./nAirEnv;
	Variable varInd;
	GeoSerialTransformer* stAirEnv = nullptr;
	if(bPos) {
	  GENFUNCTION phiInd = deltaPhi*(varInd + startPhi)*Gaudi::Units::deg;
	  TRANSFUNCTION xfAirEnv = Pow(GeoTrf::RotateZ3D(1.0),phiInd)*GeoTrf::TranslateZ3D(zposAirEnv)*GeoTrf::TranslateX3D(rposAirEnv)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
	  stAirEnv = new GeoSerialTransformer(pvAirEnv,&xfAirEnv,nAirEnv);
	} else {
	  GENFUNCTION phiInd = (180 - deltaPhi*(varInd + startPhi))*Gaudi::Units::deg;
	  TRANSFUNCTION xfAirEnv = Pow(GeoTrf::RotateZ3D(1.0),phiInd)*GeoTrf::TranslateZ3D(zposAirEnv)*GeoTrf::TranslateX3D(rposAirEnv)*GeoTrf::RotateY3D(90*Gaudi::Units::deg);
	  stAirEnv = new GeoSerialTransformer(pvAirEnv,&xfAirEnv,nAirEnv);
	}

	pvMM->add(new GeoSerialIdentifier(0));
	pvMM->add(stAirEnv);

      }
    } // if(m_fullGeo)

    // Build readout for MBTS
    // Do it only once for both A and C sides
    if(bPos) {
      if(LArGeo::buildMbtsReadout(detStore
				  , m_pAccessSvc
				  , msgSvc
				  , zposMM
				  , trdMap
				  , detectorKey
				  , detectorNode).isFailure()) {
	throw std::runtime_error("Failed to build MBTS readout geometry");
      }
    }

  }

  // Build endcap dead matter around the cryostat
  EndcapDMConstruction crateBuilder(m_activateFT);
  crateBuilder.create(cryoMotherPhysical);

  return cryoMotherPhysical;
}

GeoPhysVol* LArGeo::EndcapCryostatConstruction::buildMbtsTrd(const IRDBRecord* rec
							     , StoredMaterialManager* matmanager
							     , GeoPhysVol* parent)
{
  // Construct the Trd
  double dx1 = rec->getDouble("DX1");
  double dx2 = rec->getDouble("DX2");
  double dy1 = rec->getDouble("DY1");
  double dy2 = rec->getDouble("DY2");
  double dz = rec->getDouble("DZ");
  GeoTrd* solid = new GeoTrd(dx1,dx2,dy1,dy2,dz);
  GeoLogVol* lv = new GeoLogVol(rec->getString("TRD")
				,solid
				,matmanager->getMaterial(rec->getString("MATERIAL")));
  GeoPhysVol* pv = new GeoPhysVol(lv);
  if(parent) {
    double xpos = rec->getDouble("XPOS");
    double ypos = rec->getDouble("YPOS");
    double zpos = rec->getDouble("ZPOS");
    parent->add(new GeoTransform(GeoTrf::TranslateZ3D(zpos)*GeoTrf::TranslateY3D(ypos)*GeoTrf::TranslateX3D(xpos)));
    parent->add(pv);
  }
  return pv;
}
