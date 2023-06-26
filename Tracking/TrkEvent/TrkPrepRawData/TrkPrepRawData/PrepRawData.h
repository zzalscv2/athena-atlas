/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PrepRawData.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKPREPRAWDATA_PREPRAWDATA_H
#define TRKPREPRAWDATA_PREPRAWDATA_H

#include "Identifier/Identifier.h"
//
#include "AthLinks/tools/IdentContIndex.h"

// Eigen ATLAS helpers
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
//
#include <atomic>
#include <iostream>
#include <memory>
#include <vector>

class MsgStream;
class PrepRawDataCnv_p1;

namespace Muon {
class RpcPrepDataContainerCnv_p1;
}

namespace Trk {

/* The various kind of PrepRawData
 * to avoid dynamic_cast via using the type method
 */
enum class PrepRawDataType
{
  SiCluster,
  PixelCluster,
  SCT_Cluster,
  TRT_DriftCircle,
  HGTD_Cluster,
  PlanarCluster,
  MdtPrepData,
  CscStripPrepData,
  CscPrepData,
  MMPrepData,
  RpcPrepData,
  TgcPrepData,
  sTgcPrepData
};

class TrkDetElementBase;

/** class thrown in the event of an variable not being defined.*/
class PrepRawDataUndefinedVariable
{};

class PrepRawData : public Trk::ObjectCounter<Trk::PrepRawData>
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:
  /** public because of DataPool*/
  PrepRawData();
  PrepRawData(const PrepRawData&) = default;
  PrepRawData(PrepRawData&&) noexcept = default;
  PrepRawData& operator=(const PrepRawData&) = default;
  PrepRawData& operator=(PrepRawData&&) noexcept = default;
  virtual ~PrepRawData(); //default in .cxx

  /** Full Constructor using lvalue references
      @param clusId Identifier of the tube, strip etc which has produced this
     Trk::PrepRawData.
      @param locpos LocalPosition of the measurement (see Trk::LocalPosition for
     more discussion about what this means).
      @param rdoList Vector the Identifiers  tubes, strips etc which produced
     this Trk::PrepRawData (cluster).
      @param locerr Errors of the above measurement.
      */
  PrepRawData(const Identifier& clusId,
              const Amg::Vector2D& locpos,
              const std::vector<Identifier>& rdoList,
              const Amg::MatrixX& locerr);

  /** Full Constructor with r-value references
   * */
  PrepRawData(const Identifier& clusId,
              const Amg::Vector2D& locpos,
              std::vector<Identifier>&& rdoList,
              Amg::MatrixX&& locerr);

  /** Constructor - same as above, but no need to pass a vector of Identifiers
     (i.e. for DriftCircles)
      @param clusId Identifier of the tube, strip etc which has produced this
     Trk::PrepRawData.
      @param locpos LocalPosition of the measurement (see Trk::LocalPosition for
     more discussion about what this means).
      @param locerr Errors of the above measurement. It is not required to pass
     this (e.g. for Trigger clients) in which case a null pointer is passed
  */
  PrepRawData(const Identifier& clusId,
              const Amg::Vector2D& locpos,
              const Amg::MatrixX& locerr);

  PrepRawData(const Identifier& clusId,
              const Amg::Vector2D& locpos,
              Amg::MatrixX&& locerr);

  /** return the identifier*/
  Identifier identify() const;

  /** return the local position reference*/
  const Amg::Vector2D& localPosition() const;

  /** return the List of rdo identifiers (pointers) */
  const std::vector<Identifier>& rdoList() const;

  /** return const ref to the  error matrix */
  const Amg::MatrixX& localCovariance() const;
  /** returns localCovariance().size()!=0 */
  bool hasLocalCovariance() const;

  /** return the detector element corresponding to this PRD
      The pointer will be zero if the det el is not defined (i.e. it was not
     passed in by the ctor)*/
  virtual const TrkDetElementBase* detectorElement() const = 0;

  /** Interface method checking the type*/
  virtual bool type(PrepRawDataType type) const = 0;

  /** dump information about the PRD object. */
  virtual MsgStream& dump(MsgStream& stream) const;
  /** dump information about the PRD object. I would like to make this pure
   * virtual, but I don't want to risk screwing up the muon classes for the
   * moment*/
  virtual std::ostream& dump(std::ostream& stream) const;

  /** TEMP for testing: might make some classes friends later ... */
  void setHashAndIndex(unsigned short collHash, unsigned short objIndex);
  const IdentContIndex& getHashAndIndex() const;

private:
  friend class ::PrepRawDataCnv_p1;
  friend class Muon::RpcPrepDataContainerCnv_p1;

  /**PrepRawData ID, not const because of DataPool*/
  Identifier m_clusId{ 0 };
  /**see derived classes for definition of meaning of LocalPosition*/
  // Need to force proper alignment; otherwise cling gets it wrong.
  alignas(16) Amg::Vector2D m_localPos{};
  /**Stores the identifiers of the RDOs.*/
  std::vector<Identifier> m_rdoList{};
  /**See derived classes for definition of ErrorMatrix */
  Amg::MatrixX m_localCovariance{};

  /**Stores its own position (index) in collection plus the hash id for the
     collection (needed for the EL to IDC) */
  IdentContIndex m_indexAndHash;
};

MsgStream&
operator<<(MsgStream& stream, const PrepRawData& prd);
std::ostream&
operator<<(std::ostream& stream, const PrepRawData& prd);
}

#include "TrkPrepRawData/PrepRawData.icc"
#endif // TRKPREPRAWDATA_PREPRAWDATA_H

