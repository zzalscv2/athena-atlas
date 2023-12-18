/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TRT_DriftCircle.h
//   Header file for class TRT_DriftCircle
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to implement DriftCircle for TRT
///////////////////////////////////////////////////////////////////
// Version 7.0 15/07/2007 Peter Hansen
///////////////////////////////////////////////////////////////////

#ifndef TRKPREPRAWDATA_TRT_DRIFTCIRCLE_H
#define TRKPREPRAWDATA_TRT_DRIFTCIRCLE_H

// Base class
#include "InDetRawData/TRT_LoLumRawData.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "TrkPrepRawData/PrepRawData.h"
#include <iosfwd>

class TRT_DriftCircleContainerCnv;
class TRT_DriftCircleContainerCnv_p0;
class MsgStream;

namespace InDet {

class TRT_DriftCircle final : public Trk::PrepRawData
{
  friend class ::TRT_DriftCircleContainerCnv;
  friend class TRT_DriftCircleContainerCnv_p1;
  friend class ::TRT_DriftCircleContainerCnv_p0;
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:
  using Island = TRT_LoLumRawData::Island;
  TRT_DriftCircle();
  TRT_DriftCircle(const TRT_DriftCircle&) = default;
  TRT_DriftCircle& operator=(const TRT_DriftCircle&) = default;
  TRT_DriftCircle& operator=(TRT_DriftCircle&&) = default;
  virtual ~TRT_DriftCircle() = default;
  /** Constructor with parameters:
   *  compact id of the DriftCircle,
   *  the driftRadius and its error
   *  the RDO dataword with additional validity bit
   *  The TRT_BaseElement ptr is not owned
   */
  TRT_DriftCircle(const Identifier& Id,
                  const Amg::Vector2D& driftRadius,
                  std::vector<Identifier>&& rdoList,
                  Amg::MatrixX&& errDriftRadius,
                  const InDetDD::TRT_BaseElement* detEl,
                  const unsigned int word = 0);

  TRT_DriftCircle(const Identifier& clusId,
                  const Amg::Vector2D& driftRadius,
                  Amg::MatrixX&& errDriftRadius,
                  const InDetDD::TRT_BaseElement* detEl,
                  const unsigned int word = 0);

  /** returns the TRT dataword */
  unsigned int getWord() const;

  /** returns the leading edge bin
   *  defined as in TRT_LoLumRawData to be the first 0-1 transition */
  int driftTimeBin() const;

  /** returns the trailing edge bin */
  int trailingEdge() const;

  /** returns true if the high level threshold was passed */
  bool highLevel() const;

  /** returns true if the first bin is high */
  bool firstBinHigh() const;

  /** returns true if the last bin is high */
  bool lastBinHigh() const;

  /** returns  Time over threshold in ns  */
  double timeOverThreshold() const;

  /** returns number of high bins between LE and TE (these included) */
  int numberOfHighsBetweenEdges() const;

  /** returns number of low bins between LE and TE (these included) */
  int numberOfLowsBetweenEdges() const;
  /** returns the raw driftTime */
  double rawDriftTime() const;

  /** returns the raw driftTime,
   * the passed boolean indicates if the drift time is valid or not.
   * depreciated for 13.0.20 and later */
  double driftTime(bool& valid) const;

  /** return true if the corrected drift time is OK */
  bool driftTimeValid() const;

  /** return the detector element corresponding to this PRD */
  virtual const InDetDD::TRT_BaseElement* detectorElement()
    const override final;

  /** Interface method checking the type*/
  virtual bool type(Trk::PrepRawDataType type) const override final;

  // modifiers

  /** set driftTimeValid flag */
  void setDriftTimeValid(bool valid);

  // analysers

  /** returns true if the hit is caused by noise with a high
    probability. This is a temporary feature. To be replaced
    by a tool that can be configured for different gas
    speeds etc */
  bool isNoise() const;

  // debug printers

  /** dump information about the PRD object. */
  virtual MsgStream& dump(MsgStream& stream) const override final;

  /** dump information about the PRD object. */
  virtual std::ostream& dump(std::ostream& stream) const override final;

private:
  // not owning plain ptr
  const InDetDD::TRT_BaseElement* m_detEl;
  unsigned int m_word;
  CxxUtils::CachedValue<Island> m_island{};
};

MsgStream&
operator<<(MsgStream& stream, const TRT_DriftCircle& prd);
std::ostream&
operator<<(std::ostream& stream, const TRT_DriftCircle& prd);
}

#include "InDetPrepRawData/TRT_DriftCircle.icc"
#endif // TRKPREPRAWDATA_TRT_DRIFTCIRCLE_H

