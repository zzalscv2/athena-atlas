/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1TGC_TGCNumbering_h
#define TrigT1TGC_TGCNumbering_h

#include <string>

namespace LVL1TGC {

/** The sides of TGC (A- or C-side) */
enum TGCSide {
  ASIDE = 0,
  CSIDE,
  kNSide
};

/** The number of octants per side */
static constexpr unsigned int kNOctant = 8;

/** The number of endcap trigger sectors per side */
static constexpr unsigned int kNEndcapTrigSector = 48;

/** The maximim number of SubSector-Clusters (SSC) (i.e. The number of Endcap SSCs) */
static constexpr unsigned int kNMaxSSC = 19;

/** The default number of ROIs in SSC */
static constexpr unsigned int kNRoiInSSC = 8;

/** The number of ROIs in a endcap trigger sector */
static constexpr unsigned int kNumberOfEndcapRoI = (kNMaxSSC-1) * kNRoiInSSC + 4;

/** The number of pT thresholds in Run-2 */
static constexpr unsigned int kNThresholdsR2 = 6;
/** The number of pT thresholds in Run-3 */
static constexpr unsigned int kNThresholdsR3 = 15;

}   // namespace LVL1TGC

namespace LVL1TGCTrigger {

enum TGCZDirection {
  kZ_FORWARD = 0,         // 0
  kZ_BACKWARD,            // 1
  kTotalNumTGCZDirection  // 2
};

enum class TGCRegionType { FORWARD, ENDCAP };

enum TGCSignalType {
  WIRE=1, STRIP
};

const int NumberOfRegionType = 2;

enum TGCSlaveBoardType { WTSB=0, WDSB, STSB, SDSB, WISB, SISB, NumberOfSlaveBoardType };

enum TGCForwardBackwardType { ForwardSector=0, BackwardSector=1, TotalNumForwardBackwardType };

} //end of namespace bracket

#endif // TGCNumbering_hh
