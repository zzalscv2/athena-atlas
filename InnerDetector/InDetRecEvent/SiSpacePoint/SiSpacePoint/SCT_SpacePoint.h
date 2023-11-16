/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_SpacePoint.h
//   Header file for class SCT_SpacePoint
///////////////////////////////////////////////////////////////////
// Class to handle SPs for SCT
///////////////////////////////////////////////////////////////////
// Version 1.0   12/12/2005 Martin Siebel
///////////////////////////////////////////////////////////////////
#ifndef TRKSPACEPOINT_SCTSPACEPOINT_H
#define TRKSPACEPOINT_SCTSPACEPOINT_H

#include "Identifier/IdentifierHash.h"
#include "TrkSpacePoint/SpacePoint.h"

namespace Trk {
class PrepRawData;
}

namespace InDet {

/**
 * @class SCT_SpacePoint
 * An SCT_SpacePoint is created from two SCT_Cluster's from two different
 * wafers.
 */

class SCT_SpacePoint final : public Trk::SpacePoint {

  /////////////////////////////`//////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

 public:
  /** Default constructor */
  SCT_SpacePoint() = default;

  /**
   * @name Parametrised constructors
   * In order to ensure initialisation, the global Position has to be
   * on the surface associated to the FIRST member of the PRD-pair clusList.
   */
  //@{
  SCT_SpacePoint(const std::pair<IdentifierHash, IdentifierHash>& elementIdList,
                 const Amg::Vector3D& position,
                 const std::pair<const Trk::PrepRawData*,
                                 const Trk::PrepRawData*>& clusList);
  //@}

  /** default move,copy,dtor*/
  SCT_SpacePoint(const SCT_SpacePoint&) = default;
  SCT_SpacePoint& operator=(const SCT_SpacePoint&) = default;
  SCT_SpacePoint(SCT_SpacePoint&&) noexcept = default;
  SCT_SpacePoint& operator=(SCT_SpacePoint&&) noexcept = default;
  virtual ~SCT_SpacePoint() = default;

  /** Clones */
  virtual Trk::SpacePoint* clone() const override final;

  /**Interface method for output, to be overloaded by child classes* */
  virtual MsgStream& dump(MsgStream& out) const override final;

  /**Interface method for output, to be overloaded by child classes* */
  virtual std::ostream& dump(std::ostream& out) const override final;

 private:
  /** method to set up the local Covariance Matrix. */
  void setupLocalCovarianceSCT();

  /** common method used in constructors. */
  void setup(const std::pair<IdentifierHash, IdentifierHash>& elementIdList,
             const Amg::Vector3D& position,
             const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*>&
                 clusList);
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

inline Trk::SpacePoint* SCT_SpacePoint::clone() const {
  return new SCT_SpacePoint(*this);
}

}  // namespace InDet

#endif  // TRKSPACEPOINT_PIXELSPACEPOINT_H
