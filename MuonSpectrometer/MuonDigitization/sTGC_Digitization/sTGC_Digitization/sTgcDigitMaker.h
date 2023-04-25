/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class sTgcDigitMaker
   @section Class, Methods and Properties

   All functionality of sTGC digitization is implemented to this
   class.
*/

#ifndef STGCDIGITMAKER_H
#define STGCDIGITMAKER_H

#include <memory>
#include <vector>
#include <string>

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/StatusCode.h"
#include "Identifier/Identifier.h"
#include "MuonSimEvent/sTGCSimHit.h"

namespace CLHEP {
  class HepRandomEngine;
  class HepRandom;
}

namespace MuonGM {
  class MuonDetectorManager;
}

class sTgcDigitCollection;
class sTgcHitIdHelper;
class sTgcIdHelper;

//--- class description
class sTgcDigitMaker : public AthMessaging {
  //------ for public
 public:

  sTgcDigitMaker(const sTgcHitIdHelper* hitIdHelper, const MuonGM::MuonDetectorManager * mdManager, bool doEfficiencyCorrection, double meanGasGain, bool doPadChargeSharing);

  virtual ~sTgcDigitMaker();

  /**
     Initializes sTgcHitIdHelper and sTgcIdHelper,
     and call the functions to read files containing digitization parameters.
  */
  StatusCode initialize(const int channelTypes);

  /**
     Digitize a given hit, determining the time and charge spread on wires, pads and strips.
  */
  std::unique_ptr<sTgcDigitCollection> executeDigi(const sTGCSimHit* hit, const float globalHitTime, CLHEP::HepRandomEngine* rndmEngine) const;

  //====== for private
 private:

  /** Parameters of a gamma probability distribution function, required for
   *  estimating wire digit's time of arrival.
   *  More detail in the dat file.
   */
  struct GammaParameter {
    double lowEdge{0.}; // low side of the interval in ns
    double kParameter{0.};
    double thetaParameter{0.};
  };

  /** Ionization object with distance, position on the hit segment and
   *  position on the wire.
   */
  struct Ionization {
    double distance{-9.99}; //smallest distance bet the wire and particle trajectory
    Amg::Vector3D posOnSegment{0.,0.,0.}; // Point of closest approach
    Amg::Vector3D posOnWire{0.,0.,0.}; // Position on the wire
  };

  //uint16_t bcTagging(const float digittime, const int channelType) const;
  void addDigit(sTgcDigitCollection* digits, const Identifier id, const uint16_t bctag, const float digittime, int channelType) const;
  void addDigit(sTgcDigitCollection* digits, const Identifier id, const uint16_t bctag, const float digittime, float charge, int channelType) const;

  /** Read share/sTGC_Digitization_EffChamber.dat file */
  StatusCode readFileOfEffChamber();
  /** Read share/sTGC_Digitization_timeArrival.dat */
  StatusCode readFileOfTimeArrival();
  /** Read share/sTGC_Digitization_timeOffsetStrip.dat */
  StatusCode readFileOfTimeOffsetStrip();

  float getChamberEfficiency(const int stationName, const int stationEta, const int stationPhi, const int multiPlet, const int gasGap) const;
  /** Get stationName integer from stationName string */
  int getIStationName(const std::string& staionName) const;

  /** Compute the distance between a track segment and a wire.
   *  Expected distance is between zero and half of wire pitch (i.e. 0.9 mm),
   *  but can be greater if particle passes through the edge of a chamber.
   *  Assumig the hit is near wire k, the sign of the distance returned is:
   *   - negative if particle crosses the wire surface between wire k and wire k-1
   *   + positive if particle crosses the wire surface between wire k and wire k+1
   *  In case of error, the function returns -9.99.
   */
  double distanceToWire(Amg::Vector3D& position, Amg::Vector3D& direction, Identifier id, int wire_number) const;

  /** Determine the points where the distance between two segments is smallest.
   *  Given two segments, e.g. a particle trajectory and a sTGC wire, solve for the
   *  two points, the point on the trajectory and the point on the wire, where the
   *  distance between the two segments is the smallest.
   *
   *  Positions returned are in the local coordinate frame of the wire plane.
   *  Returns an object with distance of -9.99 in case of error.
   */
  Ionization pointClosestApproach(const Identifier& id, int wireNumber, Amg::Vector3D& preStepPos,
                                  Amg::Vector3D& postStepPos) const;

  /** Get digit time offset of a strip depending on its relative position to
   *  the strip at the centre of the cluster.
   *  It returns 0 ns by default, as well as when it fails or container is empty.
   */
  double getTimeOffsetStrip(int neighbor_index) const;

  double getPadChargeFraction(double distance) const;

  /** Find the gamma pdf parameters of a given distance */
  GammaParameter getGammaParameter(double distance) const;
  /** Get the most probable time of arrival */
  double getMostProbableArrivalTime(double distance) const;

  // sTGC chamber efficiency from HV tests
  float m_ChamberEfficiency[2][4][8][2][4]{};

  // Parameters of the gamma pdf required for determining digit time
  std::vector<GammaParameter> m_gammaParameter;
  // 4th-order polymonial describing the most probable time as function of the distance of closest approach
  std::vector<double> m_mostProbableArrivalTime;

  // Time offset to add to Strip timing
  std::vector<double> m_timeOffsetStrip;

  const sTgcHitIdHelper* m_hitIdHelper{}; // not owned here
  const MuonGM::MuonDetectorManager* m_mdManager{}; // not owned here
  const sTgcIdHelper* m_idHelper{}; // not owned here
  bool m_doEfficiencyCorrection{false};

  /**
     define offsets and widths of time windows for signals from
     wiregangs and strips. The offsets are defined as relative time
     diffference with respect to the time after TOF and cable
     length corrections. Bunch crossing time is specified.
  */

  int m_channelTypes{3}; // 1 -> strips, 2 -> strips+wires, 3 -> strips/wires/pads
  double m_theta{0.8}; // theta=0.8 value best matches the PDF
  double m_meanGasGain{5.e4};  // mean gain estimated from ATLAS note "ATL-MUON-PUB-2014-001"
  bool m_doPadSharing{false};


  // Flag to enable strip time offset
  bool m_doTimeOffsetStrip{false};
  double m_GausMean{2.27};  //mm; VMM response from Oct/Nov 2013 test beam
  double m_GausSigma{0.1885}; //mm; VMM response from Oct/Nov 2013 test beam
  double m_StripResolution{0.0949}; // Angular strip resolution parameter
  double m_posResIncident{1.};
  double m_posResAngular{0.305/m_StripResolution};
};

#endif
