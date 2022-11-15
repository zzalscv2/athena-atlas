/* -*- C++ -*- */


/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FASTSIDIGITZATION_SCT_FASTDIGITZATIONTOOL_H
#define FASTSIDIGITZATION_SCT_FASTDIGITZATIONTOOL_H
/** @file SCT_FastDigitizationTool.h
 * @brief Digitize the SCT using an implementation of IPileUpTool
 * $Id: SCT_DigitizationTool.h,v 1.0 2009-09-22 18:34:42 jchapman Exp $
 * @author John Chapman - ATLAS Collaboration
 */

#include "PileUpTools/PileUpToolBase.h"

#include "HitManagement/TimedHitCollection.h"
#include "InDetSimEvent/SiHit.h"
#include "InDetSimEvent/SiHitCollection.h"

#include "InDetPrepRawData/SCT_ClusterContainer.h"  // typedef
#include "InDetPrepRawData/SiClusterContainer.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "TrkTruthData/PRD_MultiTruthCollection.h"

#include "EventPrimitives/EventPrimitives.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "AthenaKernel/IAthRNGSvc.h"

// Gaudi
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/AlgTool.h"

#include <vector>
#include <utility> /* pair */
#include <map>
#include <string>

//FIXME - not used anywhere?
// #ifndef MAXSTEPS
// #define MAXSTEPS 15
// #endif

// #ifndef MAXDRIFTSTEPS
// #define MAXDRIFTSTEPS 15
// #endif


class InDetSimDataCollection;

class SCT_ID;

class SiChargedDiodeCollection;
class StoreGateService;

namespace InDet {
  class ClusterMakerTool;
  class SCT_Cluster;
  class SiCluster;
}

namespace CLHEP
{
  class HepRandomEngine;
}

namespace Trk {
  class Surface;
}

namespace CLHEP {
  class     HepSymMatrix ;   /// CLHEP
}

class SCT_FastDigitizationTool :
  virtual public PileUpToolBase
{

public:
  SCT_FastDigitizationTool(const std::string& type,
                           const std::string& name,
                           const IInterface* parent);
  /**
     @brief Called before processing physics events
  */
  virtual StatusCode initialize();
  StatusCode prepareEvent(const EventContext& ctx, unsigned int );
  StatusCode processBunchXing( int bunchXing,
                               SubEventIterator bSubEvents,
                               SubEventIterator eSubEvents );
  StatusCode mergeEvent(const EventContext& ctx);
  StatusCode processAllSubEvents(const EventContext& ctx);
  StatusCode createAndStoreRIOs(const EventContext& ctx);

private:

  StatusCode digitize(const EventContext& ctx,
                      TimedHitCollection<SiHit>& thpcsi);
  bool NeighbouringClusters(const std::vector<Identifier>& potentialClusterRDOList,  const InDet::SCT_Cluster *existingCluster) const;
  void Diffuse(HepGeom::Point3D<double>& localEntry, HepGeom::Point3D<double>& localExit, double shiftX, double shiftY ) const;

  StringProperty m_inputObjectName{this, "InputObjectName", "SCT_Hits", "Input Object name"};     //! name of the sub event  hit collections.

  std::vector<SiHitCollection*> m_siHitCollList;

  const SCT_ID* m_sct_ID{};                              //!< Handle to the ID helper
  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "MergeSvc", "PileUpMergeSvc", "Merge service"};            //!< PileUp Merge service
  IntegerProperty      m_HardScatterSplittingMode{this, "HardScatterSplittingMode", 0, "Control pileup & signal splitting"}; /**< Process all SiHit or just those from signal or background events */
  bool                      m_HardScatterSplittingSkipper{false};

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  //!< Random number service
  StringProperty      m_randomEngineName{this, "RndmEngine", "FastSCT_Digitization"};    //!< Name of the random number stream

  TimedHitCollection<SiHit>* m_thpcsi{};

  PublicToolHandle<InDet::ClusterMakerTool>  m_clusterMaker{this, "ClusterMaker", "InDet::ClusterMakerTool"};
  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "SiLorentzAngleTool/SCTLorentzAngleTool", "Tool to retreive Lorentz angle"};

  typedef std::multimap<IdentifierHash, InDet::SCT_Cluster*> SCT_detElement_RIO_map;
  SCT_detElement_RIO_map* m_sctClusterMap{};

  SG::WriteHandleKey<InDet::SCT_ClusterContainer>  m_sctClusterContainerKey{this, "SCT_ClusterContainerName", "SCT_Clusters"}; //!< the SCT_ClusterContainer
  SG::WriteHandleKey<PRD_MultiTruthCollection>     m_sctPrdTruthKey{this, "TruthNameSCT", "PRD_MultiTruthSCT"};         //!< the PRD truth map for SCT measurements
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  DoubleProperty m_sctSmearPathLength{this, "SCT_SmearPathSigma", 0.01};       //!< the 2. model parameter: smear the path
  BooleanProperty m_sctSmearLandau{this, "SCT_SmearLandau", true};           //!< if true : landau else: gauss
  BooleanProperty m_sctEmulateSurfaceCharge{this, "EmulateSurfaceCharge", true};  //!< emulate the surface charge
  DoubleProperty m_sctTanLorentzAngleScalor{this, "SCT_ScaleTanLorentzAngle", 1.}; //!< scale the lorentz angle effect
  BooleanProperty m_sctAnalogStripClustering{this, "SCT_AnalogClustering", false}; //!< not being done in ATLAS: analog strip clustering
  IntegerProperty m_sctErrorStrategy{this, "SCT_ErrorStrategy", 2};         //!< error strategy for the  ClusterMaker
  BooleanProperty m_sctRotateEC{this, "SCT_RotateEndcapClusters", true};

  bool m_mergeCluster{true}; //!< enable the merging of neighbour SCT clusters >
  DoubleProperty m_DiffusionShiftX_barrel{this, "DiffusionShiftX_barrel",4 };
  DoubleProperty m_DiffusionShiftY_barrel{this, "DiffusionShiftY_barrel", 4};
  DoubleProperty m_DiffusionShiftX_endcap{this, "DiffusionShiftX_endcap", 15};
  DoubleProperty m_DiffusionShiftY_endcap{this, "DiffusionShiftY_endcap", 15};
  DoubleProperty m_sctMinimalPathCut{this, "SCT_MinimalPathLength", 90.};        //!< the 1. model parameter: minimal 3D path in strip

  Amg::Vector3D stepToStripBorder(const InDetDD::SiDetectorElement& sidetel,
                                  //const Trk::Surface& surface,
                                  double localStartX, double localStartY,
                                  double localEndX, double localEndY,
                                  double slopeYX,
                                  double slopeZX,
                                  const Amg::Vector2D& stripCenter,
                                  int direction) const;




};
#endif // FASTSIDIGITZATION_SCT_FASTDIGITZATIONTOOL_H
