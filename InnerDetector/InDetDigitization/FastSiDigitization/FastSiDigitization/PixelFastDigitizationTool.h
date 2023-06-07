/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelFastDigitizationTool.h
//   Header file for class PixelFastDigitizationTool
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// Top algorithm class for Pixel fast digitization
///////////////////////////////////////////////////////////////////

#ifndef FASTSIDIGITIZATION_PIXELFASTDIGITIZATIONTOOL_H
#define FASTSIDIGITIZATION_PIXELFASTDIGITIZATIONTOOL_H

#include "PileUpTools/PileUpToolBase.h"
#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/SiHit.h"
#include "InDetSimEvent/SiHitCollection.h" // cannot fwd declare
#include "InDetPrepRawData/PixelClusterContainer.h" //typedef, cannot fwd declare
#include "SiClusterizationTool/PixelGangedAmbiguitiesFinder.h"
#include "InDetPrepRawData/PixelGangedClusterAmbiguities.h" //typedef, cannot fwd declare
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiClusterizationTool/ClusterMakerTool.h"
#include "PileUpTools/PileUpMergeSvc.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelConditionsData/PixelOfflineCalibData.h"
#include "PixelConditionsData/PixelDistortionData.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaKernel/IAthRNGSvc.h"

//New digi
#include "TrkDigEvent/DigitizationModule.h"
#include "TrkDigInterfaces/IModuleStepper.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "InDetCondTools/ISiLorentzAngleTool.h"

#include <string>
#include <vector>
#include <map>


class PixelID;
class PRD_MultiTruthCollection;

namespace InDetDD{
  class SiDetectorElement;
}
namespace CLHEP {class HepRandomEngine;}

namespace InDet {
  class PixelCluster;
  class PixelGangedAmbiguitiesFinder;
}

class PixelFastDigitizationTool :
  virtual public PileUpToolBase
{

public:

  /** Constructor with parameters */
  PixelFastDigitizationTool(
                            const std::string& type,
                            const std::string& name,
                            const IInterface* parent
                            );
  /** Destructor */
  ~PixelFastDigitizationTool();

  StatusCode initialize();
  StatusCode prepareEvent(const EventContext& ctx, unsigned int);
  StatusCode processBunchXing( int bunchXing,
                               SubEventIterator bSubEvents,
                               SubEventIterator eSubEvents );
  StatusCode processAllSubEvents(const EventContext& ctx);
  StatusCode mergeEvent(const EventContext& ctx);
  StatusCode digitize(const EventContext& ctx,
                      TimedHitCollection<SiHit>& thpcsi);
  StatusCode createAndStoreRIOs(const EventContext& ctx);




private:


  TimedHitCollection<SiHit>* m_thpcsi{};

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  //!< Random number service

  StringProperty                m_randomEngineName{this, "RndmEngine", "FastPixelDigitization"};         //!< Name of the random number stream

  const PixelID* m_pixel_ID{};                             //!< Handle to the ID helper

  PublicToolHandle<InDet::ClusterMakerTool>  m_clusterMaker{this, "ClusterMaker", "InDet::ClusterMakerTool/FatrasClusterMaker"};   //!< ToolHandle to ClusterMaker
  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "PixelLorentzAngleTool", "Tool to retreive Lorentz angle"};
  BooleanProperty                                  m_pixUseClusterMaker{this, "PixelUseClusterMaker", true}; //!< use the pixel cluster maker or not

  InDet::PixelClusterContainer*         m_pixelClusterContainer{};               //!< the PixelClusterContainer
  StringProperty                           m_pixel_SiClustersName{this, "PixelClusterContainerName", "PixelClusters"};

  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "MergeSvc", "PileUpMergeSvc"};      /**< PileUp Merge service */
  IntegerProperty    m_HardScatterSplittingMode{this, "HardScatterSplittingMode", 0, "Control pileup & signal splitting"}; /**< Process all SiHit or just those from signal or background events */
  bool                      m_HardScatterSplittingSkipper{false};

  typedef std::multimap<IdentifierHash, InDet::PixelCluster*> Pixel_detElement_RIO_map;
  Pixel_detElement_RIO_map* m_pixelClusterMap{};

  StringProperty                           m_prdTruthNamePixel{this, "TruthNamePixel", "PRD_MultiTruthPixel"};
  PRD_MultiTruthCollection*             m_pixPrdTruth{};              //!< the PRD truth map for Pixel measurements

  PublicToolHandle< InDet::PixelGangedAmbiguitiesFinder > m_gangedAmbiguitiesFinder{this, "gangedAmbiguitiesFinder", "InDet::PixelGangedAmbiguitiesFinder"};

  StringProperty m_inputObjectName{this, "InputObjectName", "PixelHits"};     //! name of the sub event  hit collections.

  std::vector<SiHitCollection*> m_siHitCollList{};

  double                                m_pixTanLorentzAngleScalor{1.}; //!< scale the lorentz angle effect
  BooleanProperty                m_pixEmulateSurfaceCharge{this, "PixelEmulateSurfaceCharge", true};  //!< emulate the surface charge
  DoubleProperty                  m_pixSmearPathLength{this, "PixelSmearPathSigma", 0.01};       //!< the 2. model parameter: smear the path
  BooleanProperty                m_pixSmearLandau{this, "PixelSmearLandau", true};           //!< if true : landau else: gauss
  DoubleProperty                  m_pixMinimalPathCut{this, "PixelMinimalPathLength", 0.06};        //!< the 1. model parameter: minimal 3D path in pixel
  DoubleProperty                  m_pixPathLengthTotConv{this, "PixelPathLengthTotConversion", 125.};     //!< from path length to tot
  BooleanProperty                m_pixModuleDistortion{this, "PixelEmulateModuleDistortion", true};       //!< simulationn of module bowing
  DoubleArrayProperty         m_pixPhiError{this, "PixelErrorPhi", {} };              //!< phi error when not using the ClusterMaker
  DoubleArrayProperty         m_pixEtaError{this, "PixelErrorEta", {} };              //!< eta error when not using the ClusterMaker
  IntegerProperty                  m_pixErrorStrategy{this, "PixelErrorStrategy", 2};         //!< error strategy for the  ClusterMaker
  DoubleProperty                  m_pixDiffShiftBarrX{this, "PixDiffShiftBarrX", 0.005}; //Shift of the track to improve cluster size description
  DoubleProperty                  m_pixDiffShiftBarrY{this, "PixDiffShiftBarrY", 0.005}; //Shift of the track to improve cluster size description
  DoubleProperty                  m_pixDiffShiftEndCX{this, "PixDiffShiftEndCX", 0.008}; //Shift of the track to improve cluster size description
  DoubleProperty                  m_pixDiffShiftEndCY{this, "PixDiffShiftEndCY", 0.008}; //Shift of the track to improve cluster size description
  DoubleProperty                  m_ThrConverted{this, "ThrConverted", 50000};

  bool m_mergeCluster{true}; //!< enable the merging of neighbour Pixel clusters >
  short m_splitClusters{0}; //!< merging parameter used to define two clusters as neighbour >
  bool m_acceptDiagonalClusters{true}; //!< merging parameter used to define two clusters as neighbour >
  StringProperty                    m_pixelClusterAmbiguitiesMapName{this, "PixelClusterAmbiguitiesMapName", "PixelClusterAmbiguitiesMap"};
  InDet::PixelGangedClusterAmbiguities* m_ambiguitiesMap{};
  ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout
  {this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager" };

  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey
  {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Pixel charge calibration data"};

  SG::ReadCondHandleKey<PixelCalib::PixelOfflineCalibData> m_offlineCalibDataKey{this, "PixelOfflineCalibData", "PixelOfflineCalibData", "Pixel offline calibration data"};

  SG::ReadCondHandleKey<PixelDistortionData> m_distortionKey
  {this, "PixelDistortionData", "PixelDistortionData", "Output readout distortion data"};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey
  {this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};

  static bool areNeighbours(const std::vector<Identifier>& group,  const Identifier& rdoID, const InDetDD::SiDetectorElement* /*element*/, const PixelID& pixelID) ;

  PixelFastDigitizationTool();
  PixelFastDigitizationTool(const PixelFastDigitizationTool&);

  PixelFastDigitizationTool& operator=(const PixelFastDigitizationTool&);

  PublicToolHandle<Trk::IModuleStepper>       m_digitizationStepper{this, "DigitizationStepper", "Trk::PlanarModuleStepper"};

  Trk::DigitizationModule * buildDetectorModule(const InDetDD::SiDetectorElement* ) const;

 static Amg::Vector3D CalculateIntersection(const Amg::Vector3D & Point, const Amg::Vector3D & Direction, Amg::Vector2D PlaneBorder, double halfthickness) ;
 static void Diffuse(HepGeom::Point3D<double>& localEntry, HepGeom::Point3D<double>& localExit, double shiftX, double shiftY ) ;
  //   void addSDO( const DiodeCollectionPtr& collection );



};

#endif // FASTSIDIGITIZATION_PIXELDIGITIZATION_H
