/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelSpacePoint.h
//   Header file for class PixelSpacePoint
///////////////////////////////////////////////////////////////////
// Class to handle SPs for Pixel
///////////////////////////////////////////////////////////////////
// Version 1.0  12/12/2005 Martin Siebel
///////////////////////////////////////////////////////////////////
#ifndef TRKSPACEPOINT_PIXELSPACEPOINT_H
#define TRKSPACEPOINT_PIXELSPACEPOINT_H

#include "TrkSpacePoint/SpacePoint.h"

class IdentifierHash;

namespace Trk {
class PrepRawData;
}

namespace InDet {

/**
 * @class PixelSpacePoint
 * A PixelSpacePoint is created from a PixelCluster.
 */

class PixelSpacePoint final : public Trk::SpacePoint {

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
 public:
  /** Default constructor */
  PixelSpacePoint() = default;

  /** Parametrised constructor */
  PixelSpacePoint(IdentifierHash elementId, const Trk::PrepRawData* clus);

  /** add Covariance Matrix and global position directly */
  PixelSpacePoint(IdentifierHash elementId,
                  const Trk::PrepRawData* clus,
                  const Amg::Vector3D& globpos,
                  const AmgSymMatrix(3)& globcov);

  PixelSpacePoint(const PixelSpacePoint& PSP) = default;
  PixelSpacePoint(PixelSpacePoint&& PSP) noexcept = default;
  PixelSpacePoint& operator=(const PixelSpacePoint&) = default;
  PixelSpacePoint& operator=(PixelSpacePoint&&) noexcept = default;

  /** Destructor */
  ~PixelSpacePoint() = default;

  /** Clones */
  virtual SpacePoint* clone() const override final;

  /** Interface method for output, to be overloaded by child classes */
  virtual MsgStream& dump(MsgStream& out) const override final;

  /** Interface method for output, to be overloaded by child classes */
  virtual std::ostream& dump(std::ostream& out) const override final;
};

///////////////////////////////////////////////////////////////////
// Inline methods:
///////////////////////////////////////////////////////////////////

inline Trk::SpacePoint* PixelSpacePoint::clone() const {
  return new PixelSpacePoint(*this);
}

}  // namespace InDet

#endif  // TRKSPACEPOINT_PIXELSPACEPOINT_H
