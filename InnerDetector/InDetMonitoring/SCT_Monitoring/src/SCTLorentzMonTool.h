/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**    @file SCTLorentzMonTool.h
 *   Class declaration for SCTLorentzMonTool
 *
 *
 *
 *    @author Luca Fiorini, based on code from Shaun Roe, Manuel Diaz Gomez
 *    and Maria Jose Casta.
 *
 *
 *
 *
 */

#ifndef SCTLORENTZMONTOOL_H
#define SCTLORENTZMONTOOL_H

#include "AthenaMonitoring/ManagedMonitorToolBase.h"

#include "SCT_MonitoringNumbers.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkTrack/TrackCollection.h"

#include "GaudiKernel/ToolHandle.h"

#include <string>

// Forward declarations
class IInterface;
class TProfile;
class StatusCode;
class SCT_ID;

class SCTLorentzMonTool : public ManagedMonitorToolBase {
 public:
  SCTLorentzMonTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~SCTLorentzMonTool() = default;
  //initialize
  virtual StatusCode initialize() final;
  /**    @name Book, fill & check (reimplemented from baseclass) */
  //@{
  ///Book histograms in initialization
  virtual StatusCode bookHistograms();
  virtual StatusCode bookHistogramsRecurrent();
  ///Fill histograms in each loop
  virtual StatusCode fillHistograms() ;
  ///process histograms at the end (we only use 'isEndOfRun')
  virtual StatusCode procHistograms();
  ///helper function used in procHistograms
  StatusCode checkHists(bool fromFinalize);
  //@}

 private:
  //@name typedefs centralised to enable easy changing of types
  //@{
  typedef TProfile* Prof_t;
  //@}

  enum SiliconSurface { surface100, surface111, allSurfaces, nSurfaces };
  enum Sides { side0, side1, bothSides, nSidesInclBoth };

  int m_numberOfEvents;
  //@name Histograms related members
  //@{

  /// Vector of pointers to profile histogram of local inc angle (phi) vs nStrips
  Prof_t m_phiVsNstrips[SCT_Monitoring::N_BARRELS][nSidesInclBoth][nSurfaces];

  std::string m_path;
  //@}
  /// Name of the Track collection to use
  SG::ReadHandleKey<TrackCollection> m_tracksName{this, "tracksName", "CombinedInDetTracks"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  //@name Service members
  //@{
  ///SCT Helper class
  const SCT_ID* m_pSCTHelper;
  //@}
  //@name  Histograms related methods
  //@{
  // Book Histograms
  StatusCode bookLorentzHistos();
  //@}

  //@name Service methods
  //@{
  // Calculate the local angle of incidence
  int findAnglesToWaferSurface ( const float (&vec)[3], const float& sinAlpha, const Identifier& id, const InDetDD::SiDetectorElementCollection* elements, float& theta, float& phi );

  ///Factory + register for the 2D profiles, returns whether successfully registered
  Prof_t pFactory(const std::string& name, const std::string& title, int nbinsx, float xlow, float xhigh, MonGroup& registry, int& iflag);
  //@}
};

#endif
