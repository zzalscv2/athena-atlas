/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelGeoModelXml/ITkPixelDetectorTool.h"
#include "PixelGeoModelXml/PixelDetectorFactory.h"
#include "PixelGeoModelXml/PixelOptions.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "InDetGeoModelUtils/InDetDDAthenaComps.h"
#include "InDetReadoutGeometry/SiCommonItems.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeometryDBSvc/IGeometryDBSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "DetDescrConditions/AlignableTransformContainer.h"
#include "SGTools/DataProxy.h"

using InDetDD::PixelDetectorManager;
using InDetDD::SiDetectorManager;

ITkPixelDetectorTool::ITkPixelDetectorTool(const std::string &type,
                                             const std::string &name,
                                             const IInterface *parent) :
    GeoModelTool(type, name, parent),
    m_detectorName("ITkPixel"),
    m_alignable(false),
    m_gmxFilename(""),
    m_manager(0),
    m_athenaComps(0),
    m_commonItems(0),
    m_geoModelSvc("GeoModelSvc", name),
    m_rdbAccessSvc("RDBAccessSvc", name),
    m_geometryDBSvc("InDetGeometryDBSvc", name),
    m_geoDbTagSvc{"GeoDbTagSvc", name}

    {
//
// Get parameter values from jobOptions file
//
    declareProperty("DetectorName", m_detectorName);
    declareProperty("Alignable", m_alignable);
    declareProperty("GmxFilename", m_gmxFilename);
    declareProperty("RDBAccessSvc", m_rdbAccessSvc);
    declareProperty("GeometryDBSvc", m_geometryDBSvc);
    declareProperty("GeoModelSvc", m_geoModelSvc);
    declareProperty("GeoDbTagSvc", m_geoDbTagSvc);

}

ITkPixelDetectorTool::~ITkPixelDetectorTool() {
  //    delete m_athenaComps;
}

StatusCode ITkPixelDetectorTool::create() {
//
//   Retrieve all services except LorentzAngleSvc, which has to be done later
//

    // Get the detector configuration.
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_rdbAccessSvc.retrieve());
    ATH_CHECK(m_geometryDBSvc.retrieve());
    GeoModelExperiment *theExpt;
    ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
    const PixelID *idHelper;
    ATH_CHECK(detStore()->retrieve(idHelper, "PixelID"));

//
//    Get their interfaces to pass to the DetectorFactory
//
    m_athenaComps = new InDetDD::AthenaComps("PixelGeoModelXml");
    m_athenaComps->setDetStore(&*(detStore()));
    m_athenaComps->setRDBAccessSvc(&*m_rdbAccessSvc);
    m_athenaComps->setGeometryDBSvc(&*m_geometryDBSvc);
    m_athenaComps->setGeoDbTagSvc(&*m_geoDbTagSvc);


    m_commonItems = new InDetDD::SiCommonItems(idHelper);
//
//    Get options from python
//
    InDetDDSLHC::PixelOptions options;
    options.setAlignable(m_alignable);
    options.setGmxFilename(m_gmxFilename);
    options.setDetectorName(m_detectorName);
//
//   Get the version
//
    DecodeVersionKey versionKey(&*m_geoModelSvc, "Pixel");
    if (versionKey.tag() == "AUTO"){
        msg(MSG::ERROR) << "Atlas version tag is AUTO. You must set a version-tag like ATLAS_P2_ITK-00-00-00." << endmsg;
        return StatusCode::FAILURE;
    }
    if (versionKey.custom()) 
        msg(MSG::INFO) << "Building custom ";
    else
        msg(MSG::INFO) << "Building ";
    msg(MSG::INFO) << "Pixel SLHC with Version Tag: "<< versionKey.tag() << " at Node: " << versionKey.node() << endmsg;
//
//    Get the Database Access Service and from there the pixel version tag
//
    std::string pixelVersionTag = m_rdbAccessSvc->getChildTag("Pixel", versionKey.tag(), versionKey.node());
    msg(MSG::INFO) << "Pixel Version: " << pixelVersionTag <<  "  Package Version: " << PACKAGE_VERSION << endmsg;
//
//   Check if pixel version tag is empty. If so, then the pixel cannot be built.
//   This may or may not be intentional. We just issue an INFO message.
//
    if (pixelVersionTag.empty()) {
        msg(MSG::INFO) <<  "No Pixel Version. PixelSLHC will not be built." << endmsg;
        return StatusCode::SUCCESS;
    }
//
// Create the PixelDetectorFactory
//
    // The * converts a ConstPVLink to a ref to a GeoVPhysVol
    // The & takes the address of the GeoVPhysVol
    GeoPhysVol *world = &*theExpt->getPhysVol();
    InDetDDSLHC::PixelDetectorFactory thePixel(m_athenaComps, m_commonItems, options);
    thePixel.create(world);
//
// Get the manager from the factory and store it in the detector store.
//
    m_manager = thePixel.getDetectorManager();

    if (!m_manager) {
        msg(MSG::ERROR) << "PixelDetectorManager not found; not created in PixelDetectorFactory?" << endmsg;
        return(StatusCode::FAILURE);
    }

    StatusCode sc;
    sc = detStore()->record(m_manager, m_manager->getName());
    if (sc.isFailure() ) {
        msg(MSG::ERROR) << "Could not register PixelDetectorManager" << endmsg;
        return StatusCode::FAILURE;
    }
    theExpt->addManager(m_manager);

    // Create a symLink to the SiDetectorManager base class so it can be accessed as either SiDetectorManager or 
    // PixelDetectorManager
    const SiDetectorManager *siDetManager = m_manager;
    sc = detStore()->symLink(m_manager, siDetManager);
    if(sc.isFailure()){
        msg(MSG::ERROR) << "Could not make link between PixelDetectorManager and SiDetectorManager" << endmsg;
        return StatusCode::FAILURE;
    }
//
//    And retrieve the LorentzAngleService. Has to be after the symLink just made,
//    which has to be after the manager is made by the DetectorFactory.
//
//    if (m_lorentzAngleSvc.empty()) {
//        msg(MSG::INFO) << "Lorentz angle service not requested." << endmsg;
//    }
//    else {
//        sc = m_lorentzAngleSvc.retrieve();
//        if (sc.isFailure()) {
//            msg(MSG::INFO) << "Could not retrieve Lorentz angle service:" <<  m_lorentzAngleSvc << endmsg;
//        }
//        else {
//            msg(MSG::INFO) << "Lorentz angle service retrieved: " << m_lorentzAngleSvc << endmsg;
//        }
//    }

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDetectorTool::clear() {
    SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::PixelDetectorManager>::ID(),m_manager->getName());
    if(proxy) {
        proxy->reset();
        m_manager = 0;
    }
    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDetectorTool::registerCallback() {
// 
//    Register call-back for software alignment
//
    StatusCode sc = StatusCode::FAILURE;
    if (m_alignable) {
        std::string folderName = "/Indet/Align";
        if (detStore()->contains<AlignableTransformContainer>(folderName)) {
            msg(MSG::DEBUG) << "Registering callback on AlignableTransformContainer with folder " << folderName << endmsg;
            const DataHandle<AlignableTransformContainer> atc;
            sc =  detStore()->regFcn(&IGeoModelTool::align, dynamic_cast<IGeoModelTool *>(this), atc, folderName);
            if(sc.isFailure()) {
                msg(MSG::ERROR) << "Could not register callback on AlignableTransformContainer with folder " << 
                                    folderName << endmsg;
            } 
        } 
        else {
            msg(MSG::WARNING) << "Unable to register callback on AlignableTransformContainer with folder " <<
                                 folderName << ", Alignment disabled (only if no Run2 scheme is loaded)!" << endmsg;
        }
    } 
    else {
        msg(MSG::INFO) << "Alignment disabled. No callback registered" << endmsg;
        // We return failure otherwise it will try and register a GeoModelSvc callback associated with this callback.
    }
    return sc;
}

StatusCode ITkPixelDetectorTool::align(IOVSVC_CALLBACK_ARGS_P(I, keys)) {
//
//    The call-back routine, which just calls the real call-back routine from the manager.
//
    if (!m_manager) {
        msg(MSG::WARNING) << "Manager does not exist" << endmsg;
        return StatusCode::FAILURE;
    }
    if (m_alignable) {
        return m_manager->align(I, keys);
    } 
    else {
        msg(MSG::DEBUG) << "Alignment disabled. No alignments applied" << endmsg;
        return StatusCode::SUCCESS;
    }
}
